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

//mutex para el acceso a la tabla de paginas
pthread_mutex_t mutex_pags=PTHREAD_MUTEX_INITIALIZER;

//Función para atender las conexiones de las cpus y el núcleo


void atender_conexion(int* socket_conexion){
	
	int32_t proceso_activo;
	bool se_cerro = false;

	while(!se_cerro){

		//Me llega un pedido de una cpu o del nucleo
		t_paquete* pedido = recibir_paquete(*socket_conexion);
		log_info(logUMC,"Se recibio un pedido del socket %d",*socket_conexion);

		switch(pedido->cod_op){
			case NUEVO_PROGRAMA:

				log_info(logUMC,"Llego el aviso de un nuevo programa");

				//Simulo el tiempo de acceso a memoria con el tiempo de retardo ingresado en la configuracion
				//Multiplico por mil para que sean milisegundos, usleep reconoce microsegundos
				usleep((config_umc->retardo)*1000);

				//Descerializo el programa
				t_pedido_inicializar *pedido_inicializar=deserializar_pedido_inicializar(pedido->datos);

				//Creo y cargo una estructura de lo que el swap tiene que recibir
				t_pedido_inicializar_swap nuevo_programa_swap;
				log_info(logUMC,"Cantidad de paginas pedidas: %d",pedido_inicializar->pagRequeridas);


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
					log_info(logUMC,"No hay espacio sufuciente para el nuevo programa");

					//Le aviso al núcleo de que no hay espacio
					enviar(NO_OK,1,socket_conexion,*socket_conexion);
					log_debug(logUMC,"Se informó a nucleo de que no hay espacio para el nuevo programa");

				//Si la respuesta es que hay espacio para el programa
				}else if(paquete->cod_op==OK){
					log_info(logUMC,"Hay espacio sufuciente para el nuevo programa");

					//Creo la entrada a la tabla de paginas del proceso
					nuevaTablaDePaginas(pedido_inicializar->idPrograma,pedido_inicializar->pagRequeridas);
					log_info(logUMC,"Se creo la tabla de paginas del programa %d ",pedido_inicializar->idPrograma);

					//Le aviso al nucleo que hay espacio para el nuevo programa
					enviar(OK,1,socket_conexion,*socket_conexion);
					log_debug(logUMC,"Se informo al kernel que hay paginas para el programa");

				//Si la respuesta es que se desconecto el socket
				}else if(paquete->cod_op==ERROR_COD_OP){

					log_warning(logUMC,"Se desconecto el socket %d",*socket_conexion);
					se_cerro = true;
				}

				break;

			case LECTURA_PAGINA:

				log_info(logUMC,"Llego un pedido de lectura de página");

				//Simulo el tiempo de acceso a memoria con el retardo ingresado en la configuracion
				//Multiplico por mil para que sean milisegundos, usleep reconoce microsegundos
				usleep((config_umc->retardo)*1000);

				//TODO Poner semáforo mutex para acceder a la TLB
				//TODO Buscar la página en la TLB, si no esta la busco en la tabla de marcos

				//Casteo el pedido como una solicitud de lectura
				t_pedido_solicitarBytes solicitud=*((t_pedido_solicitarBytes*)pedido->datos);

				//Protejo con un semáforo el acceso a la tabla de paginas
				pthread_mutex_lock(&mutex_pags);

				//Busco la pagina en la tabla de paginas
				t_entrada_tabla_paginas *tablaDePaginasLectura= buscar_pagina_en_tabla(proceso_activo,solicitud.nroPagina);

				//Libero el acceso a la tabla de páginas
				pthread_mutex_unlock(&mutex_pags);

				//Si la pagina no esta en la tabla es un error
				if(tablaDePaginasLectura==NULL){

					log_warning(logUMC,"No se encontro la pagina %d",solicitud.nroPagina);
					exit(EXIT_FAILURE);


				//Si la página está en memoria
				}else if(tablaDePaginasLectura->presencia==true){

					//Busco los datos de la página y se los envío a la cpu
					char* datosDePagina=datos_pagina_en_memoria(tablaDePaginasLectura->nro_marco);

					log_info(logUMC,"Leido: %.*s",solicitud.tamanioDatos,datosDePagina+solicitud.offset);
					enviar(BUFFER_LEIDO,solicitud.tamanioDatos,datosDePagina+solicitud.offset,*socket_conexion);
					log_info(logUMC,"Se le envio el contenido de la pagina a la cpu %d",*socket_conexion);

				//Si la página no está en memoria le pido los datos al swap
				}else if(tablaDePaginasLectura->presencia==false){

					//TODO Ver si el programa ya uso la cantidad maxima permitida de frames en bitmap
					//TODO Si ya los uso todos se elije una de sus páginas como víctima
					//TODO Si la pagina fue modificada se manda al swap y sino se elimina
					//TODO Cargar la pagina requerida en el frame que se libero en memoria

					//TODO Si todavia no uso todos sus frames ver si hay frames libres en memoria
					//TODO Si hay frames libres en memoria se carga la pagina en un frame libre y se setea la misma como usada en bitmap
					//TODO Si no hay frames libres en memoria se elije una víctima de entre los frames que tiene el programa
					//TODO Si la víctima fue modificada se la manda al swap y sino se elimina
					//TODO Cargar la página requerida en el frame que se liberó

					//TODO Ver que pasa si no hay espacio y el programa no tiene frames en memoria

					//Creo y cargo la estructura para mandarle el pedido de lectura al swap
					t_pedido_leer_swap pedidoASwap;
					pedidoASwap.pid=proceso_activo;
					pedidoASwap.nroPagina=solicitud.nroPagina;

					log_info(logUMC,"Se pidio leer la pagina %d del proceso %d la cantidad de bytes %d con el offset %d",pedidoASwap.nroPagina,pedidoASwap.pid,solicitud.tamanioDatos,solicitud.offset);

					//Bloqueo la conexión con el swap
					pthread_mutex_lock(&mutex);

					//Le envío al swap el pedido de lectura
					enviar(LECTURA_PAGINA,sizeof(pedidoASwap),&pedidoASwap,socketswap);
					log_info(logUMC,"Se pidio pagina al swap");

					//Recibo una respuesta de la lectura de parte del swap
					t_paquete *paqueteLectura=recibir_paquete(socketswap);
					log_info(logUMC,"Recibi respuesta del swap");

					//Desbloqueo la conexión con el swap
					pthread_mutex_unlock(&mutex);

					//Si el swap tiene la pagina me la pasa y le paso los bytes pedidos a la cpu
					if(paqueteLectura->cod_op==BUFFER_LEIDO){

						log_info(logUMC,"Leido: %.*s",solicitud.tamanioDatos,paqueteLectura->datos+solicitud.offset);
						enviar(BUFFER_LEIDO,solicitud.tamanioDatos,paqueteLectura->datos+solicitud.offset,*socket_conexion);
						log_info(logUMC,"Se le envio el contenido de la pagina a la cpu %d",*socket_conexion);

					//Si el swap no tiene la pagina le aviso a la cpu que hubo un error
					}else if(paqueteLectura->cod_op==NO_OK){

						log_info(logUMC,"No se pudo leer la página");
						enviar(NO_OK,1,&socket_conexion,*socket_conexion);
						log_info(logUMC,"Se le informo a la cpu %d que no se pudo leer la pagina pedida",*socket_conexion);

					//Si se desconecto el socket
					}else if(paqueteLectura->cod_op==ERROR_COD_OP){

						log_warning(logUMC,"Se desconecto el socket %d",*socket_conexion);
						se_cerro = true;
					}
					destruir_paquete(paqueteLectura);
				}
				break;
			case ESCRITURA_PAGINA:

				log_info(logUMC,"Llego un pedido de escritura de página");

				//Simulo el tiempo de acceso a memoria con el retardo ingresado en la configuración
				//Multiplico por mil para que sean milisegundos, usleep reconoce microsegundos
				usleep((config_umc->retardo)*1000);

				//TODO Poner semáforo mutex para acceder a la TLB
				//TODO Buscar página en la TLB, si no está la busco en memoria

				//Descerializo el paquete que me llego
				t_pedido_almacenarBytes *pedido_almacenar;
				pedido_almacenar=deserializar_pedido_almacenar(pedido->datos);
				log_info(logUMC,"Se pidio escribir en la pagina %d con el offset %d la cantidad de bytes %d. Se pidio escribir buffer: %.*s",pedido_almacenar->nroPagina,pedido_almacenar->offset,pedido_almacenar->tamanioDatos,pedido_almacenar->tamanioDatos,pedido_almacenar->buffer);

				//Protejo el acceso a la tabla de páginas
				pthread_mutex_lock(&mutex_pags);

				//Busco la página en la tabla de páginas
				t_entrada_tabla_paginas *tablaDePaginasEscritura=buscar_pagina_en_tabla(proceso_activo,pedido_almacenar->nroPagina);

				//Libero el acceso a la tabla de páginas
				pthread_mutex_unlock(&mutex_pags);

				//Si no se encontró la página en la tabla es un error
				if(tablaDePaginasEscritura==NULL){

					log_warning(logUMC,"No se encontro la pagina %d",pedido_almacenar->nroPagina);
					exit(EXIT_FAILURE);

				//Si la página está en memoria
				}else if(tablaDePaginasEscritura->presencia==true){

					//Busco los datos de la página como están ahora
					char* datosDePagina=datos_pagina_en_memoria(tablaDePaginasEscritura->nro_marco);

					//Modifico los datos de la página
					log_info(logUMC,"Se pidieron almacenar los datos %s", pedido_almacenar->buffer);
					memcpy(datosDePagina+pedido_almacenar->offset,pedido_almacenar->buffer,pedido_almacenar->tamanioDatos);

					//Pongo el bit de modificado de la página en true
					tablaDePaginasEscritura->modificado=true;

					//Envío el aviso de que se escribio en la página
					log_info(logUMC,"Se escribio en la página correctamente");
					enviar(OK,1,&socket_conexion,*socket_conexion);
					log_info(logUMC,"Se informo que se escribio en la página");

				//Si la página no está en memoria se la pido al swap
				}else if(tablaDePaginasEscritura->presencia==false){

					//TODO Ver si el programa ya uso la cantidad maxima permitida de frames en bitmap
					//TODO Si ya los uso todos se elije una de sus páginas como víctima
					//TODO Si la pagina fue modificada se manda al swap y sino se elimina
					//TODO Cargar la pagina requerida en el frame que se libero en memoria

					//TODO Si todavia no uso todos sus frames ver si hay frames libres en memoria
					//TODO Si hay frames libres en memoria se carga la pagina en un frame libre y se setea la misma como usada en bitmap
					//TODO Si no hay frames libres en memoria se elije una víctima de entre los frames que tiene el programa
					//TODO Si la víctima fue modificada se la manda al swap y sino se elimina
					//TODO Cargar la página requerida en el frame que se liberó

					//TODO Ver que pasa si no hay espacio y el programa no tiene frames en memoria

					//Pido la pagina como estaba antes. si no lo hago, la nueva escritura sobreescribiria la vieja
					t_pedido_leer_swap pedido_leer;
					pedido_leer.nroPagina = pedido_almacenar->nroPagina;
					pedido_leer.pid = proceso_activo;

					//Bloqueo la conexión con el swap
					pthread_mutex_lock(&mutex);

					//Le pido la página requerida al swap
					enviar(LECTURA_PAGINA,sizeof(pedido_leer),&pedido_leer,socketswap);
					t_paquete *lectura_intermedia = recibir_paquete(socketswap);

					//Desbloqueo la conexión con el swap
					pthread_mutex_unlock(&mutex);

					//Armo la estructura para pedirle la pagina al swap
					t_pedido_almacenar_swap pedido_almacenar_swap;
					pedido_almacenar_swap.buffer = malloc(config_umc->marco_size);
					memcpy(pedido_almacenar_swap.buffer,lectura_intermedia->datos,config_umc->marco_size);
					destruir_paquete(lectura_intermedia);
					log_debug(logUMC,"LECTURA INTERMEDIA PARA ESCRIBIR PAGINA %s",pedido_almacenar_swap.buffer);

					pedido_almacenar_swap.pid=proceso_activo;
					pedido_almacenar_swap.nroPagina=pedido_almacenar->nroPagina;
					memcpy((pedido_almacenar_swap.buffer)+(pedido_almacenar->offset),pedido_almacenar->buffer,pedido_almacenar->tamanioDatos);

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

						log_info(logUMC,"Se pudo escribir la pagina");
						enviar(OK,1,&socket_conexion,*socket_conexion);
						log_info(logUMC,"Se informo de que se escribio la pagina");

					//Si la respuesta del swap es que no pudo escribir la página
					}else if(paqueteEscritura->cod_op==NO_OK){

						log_info(logUMC,"No se pudo escribir en la pagina");
						enviar(NO_OK,1,&socket_conexion,*socket_conexion);
						log_info(logUMC,"Se informo de que se no se pudo escribir la pagina");

					//Si la respuesta del swap es que se desconecto el socket
					}else if(paqueteEscritura->cod_op==ERROR_COD_OP){destruir_paquete(paqueteEscritura);

						log_warning(logUMC,"Se desconecto el socket %d",*socket_conexion);
						se_cerro = true;

					}

					destruir_paquete(paqueteEscritura);
				}

				break;

			case CAMBIO_PROCESO_ACTIVO:

				proceso_activo=*((int32_t*)pedido->datos);
				log_info(logUMC,"Se informo que el pid del programa es: %d",proceso_activo);
				//TODO Buscar y devolver estructuras del nuevo proceso
				break;

			case FINALIZA_PROGRAMA:
				log_info(logUMC,"Llego un pedido para finalizar un programa");

				//Recibo el id del programa a finalizar
				int32_t *programaAFinalizar=pedido->datos;

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

					//Elimino las estructuras creadas para el manejo del programa
					dictionary_remove_and_destroy(tablasDePagina,i_to_s(*programaAFinalizar),destruir_lista);

					//TODO Marcar los frames asignados como libres en el bitmap

					//Libero el acceso a la tabla de páginas
					pthread_mutex_unlock(&mutex_pags);

					log_info(logUMC,"Se elimino el programa correctamente");
					enviar(OK,1,&socket_conexion,*socket_conexion);
					log_info(logUMC,"Se informo al nucleo que se elimino el programa correctamente");

				//Si el swap me informa que el programa no se pudo eliminar le aviso al nucleo
				}else if(pedidoFinalizar->cod_op==NO_OK){

					log_info(logUMC,"No  se pudo eliminar el programa");
					enviar(NO_OK,1,&socket_conexion,*socket_conexion);
					log_info(logUMC,"Se le informo al nucleo que no se pudo eliminar el programa");

				//Si el swap me informa que se desconecto el socket
				}else if(pedidoFinalizar->cod_op==ERROR_COD_OP){
					log_warning(logUMC,"Se desconecto el socket %d",*socket_conexion);
					se_cerro = true;
				}

				destruir_paquete(pedidoFinalizar);

				break;

			//Si me llega un codigo de error porque se desconecto el socket
			case ERROR_COD_OP:

				log_warning(logUMC,"Se desconecto el socket %d",*socket_conexion);
				se_cerro = true;

				break;
		}
		destruir_paquete(pedido);
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



