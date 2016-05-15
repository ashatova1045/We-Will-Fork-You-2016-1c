#include "Conexiones_Umc.h"
#include "../../sockets/Sockets.h"
//------------------------------------------------------------------------------------------------------
//Sockets
//------------------------------------------------------------------------------------------------------


//Funcion para manejar los pedidos de las cpu

void atender_conexion(int* socket_conexion){
	int32_t proceso_activo;

	bool se_cerro = false;
	while(!se_cerro){

		//Me llega un pedido
		t_paquete* pedido = recibir_paquete(*socket_conexion);
		log_info(logUMC,"Se recibio un pedido del socket %d",*socket_conexion);

		switch(pedido->cod_op){
			case NUEVO_PROGRAMA:
				//TODO Llega mensaje de un nuevo programa con la cantidad de páginas que necesita

				//Le envío al swap la cantidad de páginas que debe reservar
				log_info(logUMC,"Llego nuevo programa a la umc");

				enviar(NUEVO_PROGRAMA,pedido->tamano_datos,pedido->datos,socketswap);
				log_info(logUMC,"Se le envia la cantidad de paginas al swap");

				t_paquete *paquete=recibir_paquete(socketswap);
				log_info(logUMC,"Recibi respuesta de cantidad de páginas del swap");
				if(paquete->cod_op==NO_OK){
					log_info(logUMC,"No hay espacio sufuciente para el nuevo programa");
					enviar(NO_OK,1,socket_conexion,*socket_conexion);
					log_debug(logUMC,"Se informó a nucleo de que no hay espacio para el nuevo programa");
				}else{
					enviar(OK,1,socket_conexion,*socket_conexion);
					log_debug(logUMC,"Se informo al kernel que hay paginas para el programa");
				}

				//TODO Crear estructuras para el nuevo proceso (?
				//TODO Informar la creación de un nuevo proceso al swap con la cantidad de paginas a usar
				break;

			case LECTURA_PAGINA:

				log_info(logUMC,"Llego un pedido de lectura de página");
				t_pedido_solicitarBytes solicitud=*((t_pedido_solicitarBytes*)pedido->datos);
				t_pedido_leer_swap pedidoASwap;
				pedidoASwap.pid=proceso_activo;
				pedidoASwap.nroPagina=solicitud.nroPagina;
				enviar(LECTURA_PAGINA,sizeof(pedidoASwap),&pedidoASwap,socketswap);

				t_paquete *paqueteLectura=recibir_paquete(socketswap);
				log_info(logUMC,"Recibi pagina del swap");
				enviar(BUFFER_LEIDO,solicitud.tamanioDatos,paqueteLectura->datos+solicitud.offset,*socket_conexion);

				destruir_paquete(paqueteLectura);
				//TODO Traducir página a frame y devolver contenido
				//TODO Si no se encuentra la pagina se la pide al swap (algoritmo?)

				break;
			case ESCRITURA_PAGINA:
				//TODO Descerializar lo que recibe(pedido_almacenar)
				//Todo Serializar escritura a swap(con descerializar)
				//TODO Ver si respondio ok en el enviar(No debería dar error) destruyo lo que me mandaron y le digo al que me pidio que lo haga ok


				//TODO Traducir página a frame y actualizar contenido
				//TODO Si no encuentra la pagina se la pide al swap (algoritmo?)
				break;
			case CAMBIO_PROCESO_ACTIVO:
				proceso_activo=*((int32_t*)pedido->datos);
				//TODO Guardar datos del proceso actual
				//TODO Buscar y devolver estructuras del nuevo proceso
				break;
			case FINALIZA_PROGRAMA:
				//TODO mandar mensaje de eliminar a swap
				//TODO Eliminar estructuras usadas para administrar programa
				//TODO Informar de fin de un programa al swap
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
				//perror("Error al crear el hilo de la conexion");
				log_error(logUMC,"Error al crear el hilo para la conexion de CPU con socket %d",socket);
			}

			//Respondo al handshake
			enviar(OK_HS_CPU,1,&socket,socket);

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
				//perror("Error al crear el hilo de la conexion");
				log_error(logUMC,"Error al crear el hilo para la conexion de nucleo con socket %d",socket);
				close(socket);
				break;
			}else{
				log_info(logUMC,"Se creo el hilo para la conexion con el nucleo");
			}

			//Respondo al handshake del nucleo
			enviar(OK_HS_NUCLEO,sizeof(int),&config_umc->marco_size,socket);

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



