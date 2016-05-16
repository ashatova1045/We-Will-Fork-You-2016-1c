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

//Funcion para manejar los pedidos de las cpu

int32_t proceso_activo;
bool se_cerro = false;

/*void nuevo_programa(int* socket_conexion, t_paquete* pedido){

}*/

void atender_conexion(int* socket_conexion){


	while(!se_cerro){

		//Me llega un pedido
		t_paquete* pedido = recibir_paquete(*socket_conexion);
		log_info(logUMC,"Se recibio un pedido del socket %d",*socket_conexion);

		switch(pedido->cod_op){
			case NUEVO_PROGRAMA:

				log_info(logUMC,"Llego el aviso de un nuevo programa");

				//Simulo el tiempo de acceso a memoria con el tiempo de retardo ingresado en la configuracion
				sleep(config_umc->retardo);

				//Casteo el pedido como un pedido de inicializar
				t_pedido_inicializar nuevo_programa=*((t_pedido_inicializar*)pedido->datos);

				//Creo y cargo una estructura de lo que el swap tiene que recibir
				t_pedido_inicializar_swap nuevo_programa_swap;
				log_info(logUMC,"Cantidad de paginas pedidas: %d",nuevo_programa.pagRequeridas);

				nuevo_programa_swap.idPrograma=nuevo_programa.idPrograma;
				nuevo_programa_swap.pagRequeridas=nuevo_programa.pagRequeridas;

				//Le envío la estructura al swap para saber si tiene espacio para guardar el programa
				enviar(NUEVO_PROGRAMA,sizeof(nuevo_programa_swap),&nuevo_programa_swap,socketswap);
				log_info(logUMC,"Se le envio la cantidad de paginas al swap");

				//Recibo una respuesta del swap
				t_paquete *paquete=recibir_paquete(socketswap);
				log_info(logUMC,"Recibi respuesta de cantidad de páginas del swap");

				//Si la respuesta es que no hay espacio
				if(paquete->cod_op==NO_OK){
					log_info(logUMC,"No hay espacio sufuciente para el nuevo programa");

					//Le aviso al núcleo de que no hay espacio
					enviar(NO_OK,1,socket_conexion,*socket_conexion);
					log_debug(logUMC,"Se informó a nucleo de que no hay espacio para el nuevo programa");
				}else{
					log_info(logUMC,"Hay espacio sufuciente para el nuevo programa");

					//Le aviso al nucleo que hay espacio para el nuevo programa
					enviar(OK,1,socket_conexion,*socket_conexion);
					log_debug(logUMC,"Se informo al kernel que hay paginas para el programa");
				}

				//TODO Crear estructuras para el nuevo proceso (Que estructuras creo yo?

				break;

			case LECTURA_PAGINA:

				log_info(logUMC,"Llego un pedido de lectura de página");

				//Simulo el tiempo de acceso a memoria con el retardo ingresado en la configuracion
				sleep(config_umc->retardo);

				//TODO Poner semáforo mutex para acceder a la TLB
				//TODO Buscar la página en la TLB, si no esta la busco en la tabla de marcos

				//TODO Poner semáforo mutex para acceder a la tabla de marcos
				//TODO Buscar la página en la tabla de marcos, si no esta se la pido al swap

				//TODO Si la pagina la tengo en memoria le paso a la cpu el contenido pedido
				//TODO Si la pagina la tiene el swap me la pasa, la cargo en memoria y le paso a la CPU el contenido pedido

				//Casteo el pedido como una solicitud de lectura
				t_pedido_solicitarBytes solicitud=*((t_pedido_solicitarBytes*)pedido->datos);

				//Creo y cargo la estructura para mandarle el pedido de lectura al swap
				t_pedido_leer_swap pedidoASwap;
				pedidoASwap.pid=proceso_activo;
				pedidoASwap.nroPagina=solicitud.nroPagina;

				log_info(logUMC,"Se pidio leer la pagina %d del proceso %d la cantidad de bytes %d con el offset %d",pedidoASwap.nroPagina,pedidoASwap.pid,solicitud.tamanioDatos,solicitud.offset);

				//Le envío al swap el pedido de lectura
				enviar(LECTURA_PAGINA,sizeof(pedidoASwap),&pedidoASwap,socketswap);
				log_info(logUMC,"Se pidio pagina al swap");

				//Recibo una respuesta de la lectura de parte del swap
				t_paquete *paqueteLectura=recibir_paquete(socketswap);
				log_info(logUMC,"Recibi respuesta del swap");

				//Si el swap tiene la pagina me la pasa y le paso los bytes pedidos a la cpu
				if(paqueteLectura->cod_op==BUFFER_LEIDO){
					log_info(logUMC,"Leido: %*.s",solicitud.tamanioDatos,paqueteLectura->datos+solicitud.offset);
					enviar(BUFFER_LEIDO,solicitud.tamanioDatos,paqueteLectura->datos+solicitud.offset,*socket_conexion);
					log_info(logUMC,"Se le envio el contenido de la pagina a la cpu");
				//Si el swap no tiene la pagina le aviso a la cpu que hubo un error
				}else{
					log_info(logUMC,"No se pudo leer la página");
					enviar(NO_OK,1,&socket_conexion,*socket_conexion);
					log_info(logUMC,"Se le informo a la cpu que no se pudo leer la pagina");
				}


				destruir_paquete(paqueteLectura);
				//TODO Traducir página a frame y devolver contenido


				break;
			case ESCRITURA_PAGINA:

				log_info(logUMC,"Llego un pedido de escritura de página");

				//Simulo el tirmpo de acceso a memoria con el retardo ingresado en la configuración
				sleep(config_umc->retardo);

				//TODO Poner semáforo mutex para acceder a la TLB
				//TODO Buscar página en la TLB, si no está la busco en memoria

				//TODO Poner semáforo mutex para acceder a la tabla de marcos
				//TODO Buscar página en la tabla de marcos, si no está se la pido al swap

				//TODO Si encuentro la página la modifico
				//TODO Si el swap tiene la página me fijo si tengo espacio para cargarla en memoria

				//TODO Si tengo espacio para cargar en memoria la página que tiene el swap se la pido y me la pasa
				//TODO Si no tengo espacio para cargar en memoria la página que tiene el swap selecciono una página de memoria para mandarla al swap, se la mando y el swap me manda la pagina que se necesitaba

				//TODO Cuando tenga la página la traduzco a frame y la modifico

				//Descerializo el paquete que me llego
				t_pedido_almacenarBytes *pedido_almacenar;
				pedido_almacenar=(deserializar_pedido_almacenar(pedido->datos));
				log_info(logUMC,"Se pidio escribir en la pagina %d con el offset %d la cantidad de bytes %d. Se pidio escribir buffer: %*.s",pedido_almacenar->nroPagina,pedido_almacenar->offset,pedido_almacenar->tamanioDatos,pedido_almacenar->tamanioDatos,pedido_almacenar->buffer);

				//Armo la estructura para pedirle la pagina al swap
				t_pedido_almacenar_swap pedido_almacenar_swap;
				pedido_almacenar_swap.pid=proceso_activo;
				pedido_almacenar_swap.nroPagina=pedido_almacenar->nroPagina;
				pedido_almacenar_swap.buffer=pedido_almacenar->buffer;


				//Serializo estructura para mandarsela al swap
				t_pedido_almacenar_swap_serializado *pedido_almacenar_swap_serializado;
				pedido_almacenar_swap_serializado=serializar_pedido_almacenar_swap(&pedido_almacenar_swap,config_umc->marco_size);


				//Envio el pedido de escritura serializado al swap
				enviar(ESCRITURA_PAGINA,pedido_almacenar_swap_serializado->tamano,pedido_almacenar_swap_serializado->pedido_serializado,socketswap);
				log_info(logUMC,"Se le mando el pedido de escritura al swap");

				//Recibo la respuesta del swap
				t_paquete *paqueteEscritura=recibir_paquete(socketswap);
				log_info(logUMC,"Recibi una respuesta del swap");

				//Si la respuesta del swap es que se pudo escribir la página
				if(paqueteEscritura->cod_op==OK){
					log_info(logUMC,"Se pudo escribir la pagina");
					enviar(OK,1,&socket_conexion,*socket_conexion);
					log_info(logUMC,"Se informo de que se escribio la pagina");
				//Si la respuesta del swap es que no pudo escribir la página
				}else{
					log_info(logUMC,"No se pudo escribir en la pagina");
					enviar(NO_OK,1,&socket_conexion,*socket_conexion);
					log_info(logUMC,"Se informo de que se no se pudo escribir la pagina");
				}

				destruir_paquete(paqueteEscritura);

				break;

			case CAMBIO_PROCESO_ACTIVO:
				proceso_activo=*((int32_t*)pedido->datos);
				log_info(logUMC,"Se informo que el pid del programa es: %d",proceso_activo);
				//TODO Guardar datos del proceso actual
				//TODO Buscar y devolver estructuras del nuevo proceso
				break;
			case FINALIZA_PROGRAMA:
				//TODO Eliminar estructuras usadas para administrar programa

				log_info(logUMC,"Llego un pedido para finalizar un programa");

				//Recibo el id del programa a finalizar
				int32_t *programaAFinalizar=pedido->datos;

				//TODO Eliminar estructuras usadas para administrar el programa

				//Le informo al swap que finalizo un programa y le paso el PID para que elimine las estructuas
				enviar(FINALIZA_PROGRAMA,sizeof(int32_t),programaAFinalizar,socketswap);
				log_info(logUMC,"Se le informo al swap que se elimino el proceso: %d",programaAFinalizar);

				//Recibo la respuesta del swap
				t_paquete *pedidoFinalizar=recibir_paquete(socketswap);

				//Si el swap me informa que el programa se elimino correctamente le aviso al nucleo
				if(pedidoFinalizar->cod_op==OK){
					log_info(logUMC,"Se elimino el programa correctamente");
					enviar(OK,1,&socket_conexion,*socket_conexion);
					log_info(logUMC,"Se informo al nucleo que se elimino el programa correctamente");
				//Si el swap me informa que el programa no se pudo eliminar le aviso al nucleo
				}else{
					log_info(logUMC,"No  se pudo eliminar el programa");
					enviar(NO_OK,1,&socket_conexion,*socket_conexion);
					log_info(logUMC,"Se le informo al nucleo que no se pudo eliminar el programa");
				}

				destruir_paquete(pedidoFinalizar);

				break;
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



