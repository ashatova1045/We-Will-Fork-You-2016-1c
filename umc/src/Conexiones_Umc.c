#include "Conexiones_Umc.h"
#include "../../sockets/Sockets.h"
//------------------------------------------------------------------------------------------------------
//Sockets
//------------------------------------------------------------------------------------------------------


//Funcion para manejar los pedidos de las cpu

void atender_conexion(int* socket_conexion){
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
				//TODO Traducir página a frame y devolver contenido
				//TODO Si no se encuentra la pagina se la pide al swap (algoritmo?)
				break;
			case ESCRITURA_PAGINA:
				//TODO Traducir página a frame y actualizar contenido
				//TODO Si no encuentra la pagina se la pide al swap (algoritmo?)
				break;
			case CAMBIO_PROCESO_ACTIVO:
				//TODO Guardar datos del proceso actual
				//TODO Buscar y devolver estructuras del nuevo proceso
				break;
			case FINALIZA_PROGRAMA:
				//TODO Eliminar estructuras usadas para administrar programa
				//TODO Informar de fin de un programa al swap
				break;
			case 100:
				printf("El swap mando mensaje");
				puts(pedido->datos);
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
	int* socket_actual =malloc(sizeof(int));
	*socket_actual=socket;
	//pthread_attr_init(&datosConexion->attr);
	//pthread_attr_setdetachstate(&datosConexion->attr,PTHREAD_CREATE_DETACHED);
	pthread_t threadAux;

	int hilo_create = pthread_create(&threadAux,NULL,(void*)atender_conexion,socket_actual);
	if(hilo_create==0){
		log_trace(logUMC,"Se creo el hilo para las conexion %d",socket);
	}else{
		log_error(logUMC,"No se pudo crear el hilo para las conexiones");
	}

	return hilo_create;

	//TODO agregar pthread_attr_destroy(&attr);
}


//Función para manejar los mensajes
void manejar_paquete(int socket,t_paquete paq){
	log_info(logUMC,"Llego un paquete");

	log_debug(logUMC,"Llego un pedido de conexion de %d\n",socket);

	log_debug(logUMC,"El socket %d dice:\n",socket);

	log_debug(logUMC,"Datos recibidos: %s",paq.datos);
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


		//Llega un nuevo programa y lo reenvio al swap
		/*case NUEVO_PROGRAMA:

			log_debug(logUMC,"Llegaron los datos %d",paq.datos);

			//Le reenvio el programa al swap
			enviar(paq.cod_op,paq.tamano_datos,paq.datos,socketswap);

			log_info(logUMC,"Paquete mandado a swap");

			break;

		//Si llega el codigo 50 de pedido de paquete de CPU
		case 50:
			puts("Llego 50");
			//Le pido el paquete al swap
			enviar(50,1,&socketswap,socketswap);
			//El swap me manda el paquete pedido
			t_paquete* nuevo_paq=recibir_paquete(socketswap);
			log_info(logUMC,"Pedido de paquete al swap");
			//Reenvio paquete a la CPU
			enviar(50,nuevo_paq->tamano_datos,nuevo_paq->datos,socket);
			log_info(logUMC,"Paquete reenviado a la CPU");
			break;
		default:
			break;*/
	}

	//Envío datos recibidos al área de swap
	//enviar(1,paq.tamano_datos,paq.datos,socketswap);
}

//Cerrar puerto de socket conectado
void cerrar_conexion(int socket){
	printf("Se cerro %d\n",socket);
}

//Recibe nuevas conexiones
void nueva_conexion(int socket){
	printf("Se conecto %d\n",socket);
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



