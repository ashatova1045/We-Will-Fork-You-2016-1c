//------------------------------------------------------------------------------------------------------
//Includes
//------------------------------------------------------------------------------------------------------
#include "Conexiones_Umc.h"
#include "../../sockets/Sockets.h"
#include "../../general/Operaciones_umc.h"
#include "../../general/operaciones_swap.h"
//------------------------------------------------------------------------------------------------------
//Sockets
//------------------------------------------------------------------------------------------------------
//Variables globales

//mutex para la comunicación con el swap
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//mutex para la tlb
pthread_mutex_t mutex_tlb = PTHREAD_MUTEX_INITIALIZER;

//Función para atender las conexiones de las cpus y el núcleo
void atender_conexion(int* socket_conexion){
	
	int32_t proceso_activo;
	bool se_cerro = false;

	while(!se_cerro){
		log_info(logUMC,"\n\n\n\n");
		//Me llega un pedido de una cpu o del nucleo
		t_paquete* pedido = recibir_paquete(*socket_conexion);
		log_info(logUMC,"Se recibio un pedido del socket %d",*socket_conexion);

		pthread_mutex_lock(&nuevos_pedidos);
		*cant_pedidos_corriendo = (*cant_pedidos_corriendo)+1;
		pthread_mutex_unlock(&nuevos_pedidos);

		switch(pedido->cod_op){
			case NUEVO_PROGRAMA:

				log_info(logUMC,"Llego el aviso de un nuevo programa");

				//Simulo el tiempo de acceso a memoria con el tiempo de retardo ingresado en la configuracion


				//Multiplico por mil para que sean milisegundos, usleep reconoce microsegundos
				usleep((config_umc->retardo)*1000);

				//Deserializo el programa
				t_pedido_inicializar *pedido_inicializar=deserializar_pedido_inicializar(pedido->datos);

				//Creo y cargo una estructura de lo que el swap tiene que recibir
				t_pedido_inicializar_swap nuevo_programa_swap;
				log_info(logUMC,"Cantidad de paginas pedidas: %d",pedido_inicializar->pagRequeridas);
				log_info(logUMC,"Datos:\n%s",pedido_inicializar->codigo);

				nuevo_programa_swap.idPrograma=pedido_inicializar->idPrograma;
				nuevo_programa_swap.pagRequeridas=pedido_inicializar->pagRequeridas;
				nuevo_programa_swap.codigo=strdup(pedido_inicializar->codigo);

				//Serializo el pedido para mandarle al swap
				t_pedido_inicializar_serializado_swap* pedido_inicializar_swap_serializado;
				pedido_inicializar_swap_serializado = serializar_pedido_inicializar_swap(&nuevo_programa_swap);

				//Bloqueo la conexión con el swap
				pthread_mutex_lock(&mutex);

				//Le envío la estructura al swap para saber si tiene espacio para guardar el programa
				enviar(NUEVO_PROGRAMA,pedido_inicializar_swap_serializado->tamano,pedido_inicializar_swap_serializado->pedido_serializado,socketswap);
				log_info(logUMC,"Se le envio la cantidad de paginas al swap");

				//Recibo una respuesta del swap
				t_paquete *paquete=recibir_paquete(socketswap);
				log_info(logUMC,"Recibi respuesta de cantidad de páginas del swap");

				//Desbloqueo la conexión con el swap
				pthread_mutex_unlock(&mutex);


				//Si la respuesta es que no hay espacio
				if(paquete->cod_op==NO_OK){
					log_info(logUMC,"No hay espacio sufuciente para el programa %d",pedido_inicializar->idPrograma);

					//Le aviso al núcleo de que no hay espacio
					enviar(NO_OK,sizeof(int32_t),&pedido_inicializar->idPrograma,*socket_conexion);
					log_debug(logUMC,"Se informó al nucleo de que no hay espacio para el programa %d",pedido_inicializar->idPrograma);

				//Si la respuesta es que hay espacio para el programa
				}else if(paquete->cod_op==OK){
					log_info(logUMC,"Hay espacio sufuciente para el programa %d",pedido_inicializar->idPrograma);

					//Creo la entrada a la tabla de paginas del proceso
					nuevaTablaDePaginas(pedido_inicializar->idPrograma,pedido_inicializar->pagRequeridas);
					log_info(logUMC,"Se creo la tabla de paginas del programa %d ",pedido_inicializar->idPrograma);

					//Le aviso al nucleo que hay espacio para el nuevo programa
					enviar(OK,sizeof(int32_t),&pedido_inicializar->idPrograma,*socket_conexion);
					log_debug(logUMC,"Se informo al kernel que hay paginas para el programa %d",pedido_inicializar->idPrograma);

				//Si la respuesta es que se desconecto el socket
				}else if(paquete->cod_op==ERROR_COD_OP){

					log_warning(logUMC,"Se desconecto el socket %d",*socket_conexion);
					se_cerro = true;
				}

				break;

			case LECTURA_PAGINA:
				//Simulo el tiempo de acceso a memoria con el retardo ingresado en la configuracion

				//Multiplico por mil para que sean milisegundos, usleep reconoce microsegundos
				usleep((config_umc->retardo)*1000);

				t_pedido_solicitarBytes solicitud=*((t_pedido_solicitarBytes*)pedido->datos);

				log_info(logUMC,"Llego un pedido de lectura de página %d del proceso %d",solicitud.nroPagina,proceso_activo);

				//Casteo el pedido como una solicitud de lectura

				t_entrada_tabla_paginas* entrada_pag_pedida = NULL;

				if(config_umc->entradas_tlb){
					//Poner semáforo mutex para acceder a la TLB
					log_info(logUMC,"Se busca en la TLB");
					pthread_mutex_lock(&mutex_tlb);
					//Buscar la página en la TLB, si no esta la busco en la tabla de marcos
					entrada_pag_pedida = buscar_pagina_en_TLB(proceso_activo,solicitud.nroPagina);
					pthread_mutex_unlock(&mutex_tlb);
					if(entrada_pag_pedida == NULL){
						log_warning(logUMC,"Página no encontrada en la TLB");
					}
				}

				if(entrada_pag_pedida == NULL){

					//Busco la pagina en la tabla de paginas
					log_info(logUMC,"Se busca en la tabla de páginas");
					entrada_pag_pedida= buscar_pagina_en_tabla(proceso_activo,solicitud.nroPagina);

					if(entrada_pag_pedida==NULL){

						log_info(logUMC,"La página %d del proceso %d no se pudo leer por falta de espacio",solicitud.nroPagina,proceso_activo);
						enviar(NO_OK,1,&*socket_conexion,*socket_conexion);
						log_info(logUMC,"Se le informó a la cpu %d que no se pudo leer la página %d",*socket_conexion,solicitud.nroPagina);

					}else{

						if(config_umc->entradas_tlb){
							log_info(logUMC,"Se va a agregar la página %d a la TLB", solicitud.nroPagina);
							log_debug(logUMC,"Tamaño viejo TLB: %d",list_size(tlb));
							//Cargar pagina en la TLB
							pthread_mutex_lock(&mutex_tlb);
							if(list_size(tlb) == config_umc->entradas_tlb){
								//Implementación de LRU
								eliminar_menos_usado_en_TLB();
							}

							cargar_en_TLB(proceso_activo,solicitud.nroPagina,entrada_pag_pedida);

							log_debug(logUMC,"Tamaño nuevo TLB: %d",list_size(tlb));

							pthread_mutex_unlock(&mutex_tlb);
						}

						//Busco los datos de la página y se los envío a la cpu
						log_info(logUMC,"Nro de marco: %d",entrada_pag_pedida->nro_marco);
						char* datosDePagina=datos_pagina_en_memoria(entrada_pag_pedida->nro_marco);

						enviar(BUFFER_LEIDO,solicitud.tamanioDatos,datosDePagina+solicitud.offset,*socket_conexion);
						log_info(logUMC,"Se le envio el contenido de la pagina %d a la cpu %d",solicitud.nroPagina,*socket_conexion);
					}
				}else{
					log_info(logUMC,"Página encontrada en la TLB - frame %d",entrada_pag_pedida->nro_marco);

					//Busco los datos de la página y se los envío a la cpu
					char* datosDePagina=datos_pagina_en_memoria(entrada_pag_pedida->nro_marco);

					enviar(BUFFER_LEIDO,solicitud.tamanioDatos,datosDePagina+solicitud.offset,*socket_conexion);
					log_info(logUMC,"Se le envio el contenido de la pagina %d a la cpu %d",solicitud.nroPagina,*socket_conexion);
				}

				break;
			case ESCRITURA_PAGINA:

				log_info(logUMC,"Llego un pedido de escritura de página");

				//Simulo el tiempo de acceso a memoria con el retardo ingresado en la configuración

				//Multiplico por mil para que sean milisegundos, usleep reconoce microsegundos
				usleep((config_umc->retardo)*1000);

				//Deserializo el paquete que me llego
				t_pedido_almacenarBytes *pedido_almacenar;
				pedido_almacenar=deserializar_pedido_almacenar(pedido->datos);

				log_info(logUMC,"Se pidio escribir en la pagina %d con el offset %d la cantidad de bytes %d",pedido_almacenar->nroPagina,pedido_almacenar->offset,pedido_almacenar->tamanioDatos);

				t_entrada_tabla_paginas* entrada_pag_escritura = NULL;

				if(config_umc->entradas_tlb){
					//Poner semáforo mutex para acceder a la TLB
					pthread_mutex_lock(&mutex_tlb);

					//Buscar página en la TLB, si no está la busco en memoria
					entrada_pag_escritura = buscar_pagina_en_TLB(proceso_activo,pedido_almacenar->nroPagina);

					pthread_mutex_unlock(&mutex_tlb);
					if(entrada_pag_pedida == NULL){
						log_warning(logUMC,"Página no encontrada en la TLB");
					}
				}

				if(entrada_pag_escritura == NULL){
					//Busco la página en la tabla de páginas
					entrada_pag_escritura=buscar_pagina_en_tabla(proceso_activo,pedido_almacenar->nroPagina);

					if(entrada_pag_escritura==NULL){

						log_info(logUMC,"La página %d del proceso %d no se pudo escribir por falta de espacio",solicitud.nroPagina,proceso_activo);
						enviar(NO_OK,1,&*socket_conexion,*socket_conexion);
						log_info(logUMC,"Se le informó a la cpu %d que no se pudo escribir la página %d",*socket_conexion,solicitud.nroPagina);

					}else{

						if(config_umc->entradas_tlb){
							//Cargar pagina en la TLB
							pthread_mutex_lock(&mutex_tlb);
							if(list_size(tlb) == config_umc->entradas_tlb){
								//Implementación de LRU
								eliminar_menos_usado_en_TLB();
							}

							//cargar_en_TLB(proceso_activo,solicitud.nroPagina,entrada_pag_escritura);
							cargar_en_TLB(proceso_activo,pedido_almacenar->nroPagina,entrada_pag_escritura);

							pthread_mutex_unlock(&mutex_tlb);
						}

						//Busco los datos de la página como están ahora en memoria
						char* datosDePaginaEscritura = datos_pagina_en_memoria(entrada_pag_escritura->nro_marco);

						//Modifico los datos de la página
						log_info(logUMC,"Se pidieron almacenar los datos de la pagina %d del proceso %d",pedido_almacenar->nroPagina,proceso_activo);
						memcpy(datosDePaginaEscritura+pedido_almacenar->offset,pedido_almacenar->buffer,pedido_almacenar->tamanioDatos);
						log_info(logUMC,"Se almacenaron los datos de la pagina %d del proceso %d",pedido_almacenar->nroPagina,proceso_activo);

						//Pongo el bit de modificado de la página en true
						entrada_pag_escritura->modificado=true;
						log_info(logUMC,"El bit de modificado de la pagina %d del proceso %d es %d",pedido_almacenar->nroPagina,proceso_activo,entrada_pag_escritura->modificado);

						//Envío el aviso de que se escribio en la página
						log_info(logUMC,"Se escribio en la página correctamente");
						enviar(OK,1,&socket_conexion,*socket_conexion);
						log_info(logUMC,"Se informo que se escribio en la página");
					}
				}else{
					log_info(logUMC,"Página encontrada en la TLB");

					//Busco los datos de la página como están ahora en memoria
					char* datosDePaginaEscritura = datos_pagina_en_memoria(entrada_pag_escritura->nro_marco);

					//Modifico los datos de la página
					log_info(logUMC,"Se pidieron almacenar los datos de la pagina %d del proceso %d",pedido_almacenar->nroPagina,proceso_activo);
					memcpy(datosDePaginaEscritura+pedido_almacenar->offset,pedido_almacenar->buffer,pedido_almacenar->tamanioDatos);
					log_info(logUMC,"Se almacenaron los datos de la pagina %d del proceso %d",pedido_almacenar->nroPagina,proceso_activo);

					//Pongo el bit de modificado de la página en true
					entrada_pag_escritura->modificado=true;
					log_info(logUMC,"El bit de modificado de la pagina %d del proceso %d es %d",solicitud.nroPagina,proceso_activo,entrada_pag_escritura->modificado);


					//Envío el aviso de que se escribio en la página
					log_info(logUMC,"Se escribio en la página correctamente");
					enviar(OK,1,&socket_conexion,*socket_conexion);
					log_info(logUMC,"Se informo que se escribio en la página");
				}

				break;

			case CAMBIO_PROCESO_ACTIVO:

				if(config_umc->entradas_tlb){
					pthread_mutex_lock(&mutex_tlb);
					eliminarPaginasEnTLB(proceso_activo);
					pthread_mutex_unlock(&mutex_tlb);
				}

				proceso_activo=*((int32_t*)pedido->datos);
				log_info(logUMC,"Se informo que el pid del programa es: %d",proceso_activo);

				break;

			case FINALIZA_PROGRAMA:
				log_info(logUMC,"Llego un pedido para finalizar un programa");

				//Recibo el id del programa a finalizar
				int32_t *programaAFinalizar=pedido->datos;

				log_info(logUMC,"Se pidio finalizar el proceso %d",*programaAFinalizar);

				//Bloqueo la conexión con el swap
				pthread_mutex_lock(&mutex);

				//Le informo al swap que finalizo un programa y le paso el PID para que elimine las estructuas
				enviar(FINALIZA_PROGRAMA,sizeof(int32_t),programaAFinalizar,socketswap);
				log_info(logUMC,"Se le informo al swap que se elimino el proceso: %d",*programaAFinalizar);

				//Recibo la respuesta del swap
				t_paquete *pedidoFinalizar=recibir_paquete(socketswap);

				//Desbloqueo la conexión con el swap
				pthread_mutex_unlock(&mutex);

				//Si el swap me informa que el programa se elimino correctamente le aviso al nucleo
				if(pedidoFinalizar->cod_op==OK){

					//Protejo el acceso a la tabla de páginas
					pthread_mutex_lock(&mutex_pags);

					//Se marcan los frames asignados como libres en el bitmap
					t_entrada_diccionario *entrada_diccionario = dictionary_remove(tablasDePagina,i_to_s(*programaAFinalizar));

					//Libero el acceso a la tabla de páginas
					pthread_mutex_unlock(&mutex_pags);

					//Se elimina de la TLB
					if(config_umc->entradas_tlb){
						eliminarPaginasEnTLB(*programaAFinalizar);
					}

					t_list* tablaDePaginas = entrada_diccionario->tablaDePaginas;
					list_iterate(tablaDePaginas,eliminarPaginas);
					list_destroy(tablaDePaginas);
					free(entrada_diccionario);

					log_info(logUMC,"Se eliminaron las estructuras del proceso %d",*programaAFinalizar);

					log_info(logUMC,"Se elimino el programa correctamente");
					//enviar(OK,1,&socket_conexion,*socket_conexion);
					log_info(logUMC,"Se informo al nucleo que se elimino el programa %d correctamente",*programaAFinalizar);

				//Si el swap me informa que el programa no se pudo eliminar le aviso al nucleo
				}else if(pedidoFinalizar->cod_op==NO_OK){

					log_info(logUMC,"No  se pudo eliminar el programa %d",*programaAFinalizar);
					//enviar(NO_OK,1,&socket_conexion,*socket_conexion);
					log_info(logUMC,"Se le informo al nucleo que no se pudo eliminar el programa %d",*programaAFinalizar);

				//Si el swap me informa que se desconecto el socket
				}else if(pedidoFinalizar->cod_op==ERROR_COD_OP){
					log_warning(logUMC,"Se desconecto el socket %d",*socket_conexion);
					se_cerro = true;
				}

				destruir_paquete(pedidoFinalizar);

				break;

			//Si me llega un codigo de error porque se desconecto el socket
			case ERROR_COD_OP:

				//Si el socket que se desconectó es del núcleo se cierra
				if(*socket_conexion==socket_nucleo){

					log_warning(logUMC,"Se desconectó el núcleo");
					exit(EXIT_FAILURE);

				//Si el socket que se cerró es de una cpu
				}else{

				log_warning(logUMC,"Se desconecto la cpu con el socket %d",*socket_conexion);
				se_cerro = true;
				}

				break;
		}
		destruir_paquete(pedido);

		*cant_pedidos_corriendo = (*cant_pedidos_corriendo)-1;
	}
	free(socket_conexion);
}

int crear_hilo_conexion(int socket){
	int* socket_actual = malloc(sizeof(int));
	*socket_actual=socket;

	pthread_t threadAux;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	int hilo_create = pthread_create(&threadAux,&attr,(void*)atender_conexion,socket_actual);
	if(hilo_create==0){
		log_trace(logUMC,"Se creo el hilo para las conexion %d",socket);
	}else{
		log_error(logUMC,"No se pudo crear el hilo para las conexiones");
	}

	pthread_attr_destroy(&attr);
	return hilo_create;
}


//Función para manejar los mensajes
void manejar_paquete(int socket,t_paquete paq){

	log_debug(logUMC,"Llego un pedido de %d\n",socket);

	//Comprueba el codigo de operacion
	switch (paq.cod_op) {
		//Handshake con cpu
		case HS_CPU_UMC:

			//Borro el descriptor de archivos para usar los hilos en lugar del select
			FD_CLR(socket,&set_de_fds);

			//Creo el hilo para la conexion con una cpu
			if(crear_hilo_conexion(socket)){

				log_error(logUMC,"Error al crear el hilo para la conexion de CPU con socket %d",socket);
			}

			//Respondo al handshake de la cpu
			enviar(OK_HS_CPU,1,&socket,socket);

			//Le mando a la cpu el tamaño de las paginas
			enviar(TAMANIO_PAGINA,sizeof(int),&config_umc->marco_size,socket);

			log_debug(logUMC,"Handshake con CPU exitoso");
			log_info(logUMC,"El socket %d es de una cpu",socket);

			break;

		//Handshake con nucleo
		case HS_NUCLEO_UMC:
			log_info(logUMC,"El nucleo pidio handshake");

			//Guardo el socket del núcleo
			socket_nucleo=socket;

			//Borro el descriptor de archivos para usar los hilos en lugar de los select
			FD_CLR(socket,&set_de_fds);

			//Creo el hilo para la conexion con el nucleo
			if(crear_hilo_conexion(socket)){

				log_error(logUMC,"Error al crear el hilo para la conexion de nucleo con socket %d",socket);
				close(socket);
				break;

			}else{
				log_info(logUMC,"Se creo el hilo para la conexion con el nucleo");
			}

			//Respondo al handshake del nucleo
			enviar(OK_HS_NUCLEO,1,&socket,socket);

			//Le mando al nucleo el tamaño de las paginas
			enviar(TAMANIO_PAGINA,sizeof(int),&config_umc->marco_size,socket);


			log_debug(logUMC,"Handshake con nucleo exitoso");
			log_info(logUMC,"El socket %d es del nucleo",socket);

			break;

		//Llega código de error
		case ERROR_COD_OP:

			log_error(logUMC,"Llego el codigo de error");
			break;
	}
}

//Cerrar puerto de socket conectado
void cerrar_conexion(int socket){
	log_debug(logUMC,"Se cerro %d\n",socket);
}

//Recibe nuevas conexiones
void nueva_conexion(int socket){
	log_debug(logUMC,"Se conecto %d\n",socket);
}

//------------------------------------------------------------------------------------------------------
//Hilos sockets
//------------------------------------------------------------------------------------------------------

//Creo la funcion del socket servidor para CPU y Nucleo
void servidor_pedidos(){

	log_trace(logUMC,"Entro a la funcion servidor");

	//Creo el server multiconexión
	int fdmax;

	int puerto=config_umc->puerto;
	socketServerPedido = crear_server_multiconexion(&set_de_fds,puerto,&fdmax);
	if(socketServerPedido ==-1){
		log_error(logUMC,"Error al crear el server");
		exit(EXIT_FAILURE);
	}

	//Mensajes de conexión exitosa
	log_info(logUMC,"Se creo un socket multiconexion. Su fd es: %d \n",socketServerPedido);
	log_info(logUMC,"Escuchando conexiones y corriendo!");

	//correr_server_multiconexion(fdmax,&set_de_fds,socketServer,manejar_paquete,cerrar_conexion,nueva_conexion);
	correr_server_multiconexion(&fdmax,&set_de_fds,socketServerPedido,manejar_paquete,cerrar_conexion,nueva_conexion);


	//Cierro el puerto y libero la memoria del socket
	close(socketServerPedido);
}

//------------------------------------------------------------------------------------------------------
//Operaciones con swap (Leer y Escribir)
//------------------------------------------------------------------------------------------------------

void escribirEnSwap(int pagina,char* datos_pagina,int pid){

	//Armo la estructura para pasarle la pagina al swap
	t_pedido_almacenar_swap pedido_almacenar_swap;
	pedido_almacenar_swap.buffer = datos_pagina;

	pedido_almacenar_swap.pid=pid;
	pedido_almacenar_swap.nroPagina=pagina;

	log_info(logUMC,"Se le pidio al swap almacenar la página %d del proceso %d",pagina,pid);

	//Serializo estructura para mandarsela al swap
	t_pedido_almacenar_swap_serializado *pedido_almacenar_swap_serializado;
	pedido_almacenar_swap_serializado=serializar_pedido_almacenar_swap(&pedido_almacenar_swap,config_umc->marco_size);

	//Bloqueo la conexión con el swap
	pthread_mutex_lock(&mutex);

	//Envio el pedido de escritura serializado al swap
	enviar(ESCRITURA_PAGINA,pedido_almacenar_swap_serializado->tamano,pedido_almacenar_swap_serializado->pedido_serializado,socketswap);
	log_info(logUMC,"Se le mando el pedido de escritura al swap");
	log_debug(logUMC,"Pagina a escribir\n%.*s",config_umc->marco_size,pedido_almacenar_swap.buffer);

	//Recibo la respuesta del swap
	t_paquete *paqueteEscritura=recibir_paquete(socketswap);
	log_info(logUMC,"Recibi una respuesta del swap");

	//Desbloqueo la conexión con el swap
	pthread_mutex_unlock(&mutex);

	//Si la respuesta del swap es que se pudo escribir la página
	if(paqueteEscritura->cod_op==OK){

		log_info(logUMC,"Se pudo escribir la pagina %d del proceso %d en swap",pagina,pid);

	//Si la respuesta del swap es que no pudo escribir la página
	}else if(paqueteEscritura->cod_op==NO_OK){

		log_info(logUMC,"No se pudo escribir la pagina %d del proceso %d",pagina,pid);
		exit(EXIT_FAILURE);

	//Si la respuesta del swap es que se desconecto el socket
	}else if(paqueteEscritura->cod_op==ERROR_COD_OP){

		log_warning(logUMC,"Hubo un error al escribir la página");
		log_warning(logUMC,"Se cerro la conexión con el swap");
		exit(EXIT_FAILURE);

	}

	destruir_paquete(paqueteEscritura);
	free(pedido_almacenar_swap_serializado);
}


char* leerDeSwap(int pid,int pagina){

	char* datos_pagina;

	//Le pido la página al swap
	t_pedido_leer_swap pedido_leer;
	pedido_leer.nroPagina = pagina;
	pedido_leer.pid = pid;
	log_info(logUMC,"Se pidio al swap leer la pagina %d del proceso %d",pagina,pid);

	//Bloqueo la conexión con el swap
	pthread_mutex_lock(&mutex);

	//Le pido la página requerida al swap
	enviar(LECTURA_PAGINA,sizeof(pedido_leer),&pedido_leer,socketswap);
	t_paquete *paquete_lectura = recibir_paquete(socketswap);
	log_info(logUMC,"Recibi el contenido de la pagina %d del proceso %d del swap",pagina,pid);

	//Desbloqueo la conexión con el swap
	pthread_mutex_unlock(&mutex);

	//Si el swap tiene la pagina me la pasa
	if(paquete_lectura->cod_op==BUFFER_LEIDO){

		datos_pagina=malloc(paquete_lectura->tamano_datos);
		memcpy(datos_pagina,paquete_lectura->datos,paquete_lectura->tamano_datos);
		log_debug(logUMC,"Pagina a leer\n%.*s",config_umc->marco_size,datos_pagina);

	//Si el swap no tiene la pagina
	}else if(paquete_lectura->cod_op==NO_OK){

		log_info(logUMC,"No se pudo leer la página %d del proceso %d",pagina,pid);
		exit(EXIT_FAILURE);

	//Si se desconecto el socket
	}else if(paquete_lectura->cod_op==ERROR_COD_OP){

		log_warning(logUMC,"Se desconecto el swap");
		exit(EXIT_FAILURE);
	}

	destruir_paquete(paquete_lectura);

	return datos_pagina;
}

void eliminar_pagina_TLB(int proceso, int pagina){
	if(config_umc->entradas_tlb){

		pthread_mutex_lock(&mutex_tlb);
		int i;
		for(i=0;i<list_size(tlb);i++){
			t_entrada_tlb* entrada_eliminar = list_get(tlb,i);

			if(entrada_eliminar->pid==proceso && entrada_eliminar->nroPagina==pagina){
				free(list_remove(tlb,i));
			}
		}
		pthread_mutex_unlock(&mutex_tlb);
	}
}

//------------------------------------------------------------------------------------------------------
