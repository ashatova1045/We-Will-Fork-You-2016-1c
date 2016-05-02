#include "Conexiones_Umc.h"

//------------------------------------------------------------------------------------------------------
//Sockets
//------------------------------------------------------------------------------------------------------

//Función para manejar los mensajes
void manejar_paquete(int socket,t_paquete paq){
	log_info(logUMC,"Llego un paquete");
	//Comprueba el codigo de operacion
	switch (paq.cod_op) {
		//Si es handshake de CPU le manda ok
		case HS_CPU_UMC:
			enviar(OK_HS_CPU,1,&socket,socket);
			puts("Handshake CPU correcto");
			log_info(logUMC,"Handshake con CPU exitoso");
			break;
		//Si es handshake de nucleo le manda ok
		case HS_NUCLEO_UMC:
			enviar(OK_HS_NUCLEO,1,&socket,socket);
			puts("Handshake Nucleo correcto");
			log_info(logUMC,"Handshake con nucleo exitoso");
			break;
		//Si es el código de error
		case ERROR_COD_OP:
			log_error(logUMC,"Llego el codigo de error");
			exit(EXIT_FAILURE);
		//Si llega un nuevo programa lo reenvio al swap
		case NUEVO_PROGRAMA:
			puts(paq.datos);
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
			break;
	}

	printf("Llego un pedido de conexion de %d\n",socket);
	//log_info("Llego un pedido de conexion de %d\n",socket);

	printf("El socket %d dice:\n",socket);
	//log_info("El socket %d dice: \n",socket);

	puts(paq.datos);

	//Envío datos recibidos al área de swap
	enviar(1,paq.tamano_datos,paq.datos,socketswap);
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

	log_debug(logUMC,"Entro a la funcion servidor");

	//Creo el server multiconexión
	fd_set set_de_fds;
	int fdmax;

	int puerto=config_umc->puerto;
	socketServerPedido = crear_server_multiconexion(&set_de_fds,puerto,&fdmax);
	if(socketServerPedido ==-1){
		log_error(logUMC,"Error al crear el server");
		exit(EXIT_FAILURE);
	}

	//Mensajes de conexión exitosa
	printf("Se creo un socket multiconexion. Su fd es: %d \n",socketServerPedido);
	puts("Escuchando conexiones y corriendo!");

	//correr_server_multiconexion(fdmax,&set_de_fds,socketServer,manejar_paquete,cerrar_conexion,nueva_conexion);
	correr_server_multiconexion(&fdmax,&set_de_fds,socketServerPedido,manejar_paquete,cerrar_conexion,nueva_conexion);


	//Cierro el puerto y libero la memoria del socket
	close(socketServerPedido);
}
//------------------------------------------------------------------------------------------------------



