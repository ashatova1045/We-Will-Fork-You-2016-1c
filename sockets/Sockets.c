#include "Sockets.h"

int tamano_del_paquete(t_paquete paquete)
{
	return paquete.tamano_datos + SIZE_HEADER;
}

int handshake(int socket, uint16_t cop_op_cliente, uint16_t cod_op_esperado)
{
	int respuesta = -1;
	enviar(cop_op_cliente,1,&socket,socket);

	fd_set set;
	struct timeval timeout;
	timeout.tv_sec = 5;
	FD_ZERO(&set);
	FD_SET(socket, &set);
	select(socket+1,&set,NULL,NULL,&timeout);
	if(FD_ISSET(socket,&set)){
		t_paquete *paquete = recibir_paquete(socket);
		if(paquete->cod_op == cod_op_esperado)
			respuesta = 1;
		destruir_paquete(paquete);
	}
	return respuesta;
}

int crear_socket_escucha(int puerto)
{
	struct sockaddr_in socket_info;
	int optval = 1;

	int socket_escucha = socket(AF_INET, SOCK_STREAM, 0);

	if (socket_escucha == -1) {
		perror("Error al crear el socket");
		return -1;
	}

	setsockopt(socket_escucha, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	socket_info.sin_family = AF_INET;
	socket_info.sin_addr.s_addr = INADDR_ANY;
	socket_info.sin_port = htons(puerto);

	memset(&(socket_info.sin_zero), '\0', 8);

	if (bind(socket_escucha, (struct sockaddr*) &socket_info, sizeof(socket_info)) != 0) {
		perror("Error al bindear socket escucha");
		return -1;
	}

	if (listen(socket_escucha, 100) != 0) {
		perror("Error al poner a escuchar socket");
		return -1;
	}

	return socket_escucha;
}

int conectar(char* direccion, int puerto){
	struct sockaddr_in socket_struct;
	socket_struct.sin_family = AF_INET;
	socket_struct.sin_addr.s_addr = inet_addr(direccion);
	socket_struct.sin_port = htons(puerto);
	memset(&(socket_struct.sin_zero), '\0', 8);

	// Crear un socket:
	// AF_INET: Socket de internet IPv4
	// SOCK_STREAM: Orientado a la conexion, TCP
	// 0: Usar protocolo por deft_paqueteecto para AF_INET-SOCK_STREAM: Protocolo TCP/IPv4
	int socket_emisor = socket(AF_INET, SOCK_STREAM, 0);

	if (socket_emisor < 0) {
		perror("Error al crear socket");
		return -1;
	}
	// Conectar el socket con la direccion 'socketInfo'.
	if (connect(socket_emisor, (struct sockaddr*) &socket_struct, sizeof(socket_struct))	!= 0) {
		perror("Error al conectar socket");
		return -1;
	}

	return socket_emisor;
}

int aceptar_cliente(int socket_server_fd) {
	struct sockaddr cliente;
	int addrlen = sizeof(struct sockaddr_in);

	int descriptor_cliente = accept(socket_server_fd, &cliente, (socklen_t*) &addrlen);
	if (descriptor_cliente == -1)
		perror("Error al aceptar");
	return descriptor_cliente;
}

static t_paquete armar_paquete(uint16_t cod_op, int tamano_datos, void* datos){
	t_paquete unPaquete;
	unPaquete.cod_op= cod_op;
	unPaquete.tamano_datos = tamano_datos;
	unPaquete.datos = datos;
	return unPaquete;
}

static char* paquete_a_stream(t_paquete* paquete){
	char* stream_salida =  (char*)malloc(tamano_del_paquete(*paquete));//armo un stream del tamano del paquete
	if(stream_salida != NULL)
	{
		memcpy(stream_salida, &paquete->cod_op, sizeof(paquete->cod_op));
		memcpy(stream_salida + sizeof(paquete->cod_op), &paquete->tamano_datos, sizeof(paquete->tamano_datos));
		memcpy(stream_salida + SIZE_HEADER, paquete->datos, paquete->tamano_datos);
	}
	return stream_salida;
}

int enviar(uint16_t cod_op, int tamano_datos_en_bytes, void* datos, int destino_fd){
	t_paquete paquete_a_enviar = armar_paquete(cod_op, tamano_datos_en_bytes, datos);
	const int tamano_paquete_a_enviar = tamano_del_paquete(paquete_a_enviar);

	char* bloque = paquete_a_stream(&paquete_a_enviar);
	//send devuelve la cantidad de bytes que envio
	int bytes_enviados = 0;

	do{
		//envio a partir del punto hasta donde llego la vez anterior
		//la cantidad de bytes a enviar es el tamano inicial menos lo que ya envio
		bytes_enviados += send(destino_fd, bloque + bytes_enviados, tamano_paquete_a_enviar - bytes_enviados, 0);
	} while(bytes_enviados != tamano_paquete_a_enviar);

	free(bloque);
	return bytes_enviados;
}

static t_paquete armar_paquete_de_error(){
	t_paquete paquete = armar_paquete(ERROR_COD_OP,ERROR_TAMANO,ERROR_DATOS);
	return paquete;
}

t_paquete* recibir_paquete(int socket_receptor_fd){
	t_paquete *nuevo_paquete = malloc(sizeof(t_paquete));
	*nuevo_paquete = armar_paquete_de_error();
	uint16_t codigo;
	uint32_t tamano;

	ssize_t recibidos;

	recibidos = recv(socket_receptor_fd, &codigo, sizeof(nuevo_paquete->cod_op), 0);

	if(recibidos <= 0){
		if(recibidos == -1)
			perror("Error al recibir codigo");
		else
			close(socket_receptor_fd);
		return nuevo_paquete;
	}

	recibidos = recv(socket_receptor_fd, &tamano, sizeof(nuevo_paquete->tamano_datos), 0);

	if(recibidos <= 0){
		if(recibidos == -1)
			perror("Error al recibir tamano");
		else
			close(socket_receptor_fd);
		return nuevo_paquete;
	}

	//hago que datos sea del tamano que me indica cliente
	void *datos = malloc(tamano);
	if (datos == NULL)
		return nuevo_paquete;

	//si los datos son demasiado grandes, se requiere iterar hasta tener el total
	recibidos = 0;
	do{
		recibidos += recv(socket_receptor_fd, datos + recibidos, tamano - recibidos, 0);
		if(recibidos <= 0){
			if(recibidos == -1)
				perror("Error al recibir datos");
			else
				close(socket_receptor_fd);
			return nuevo_paquete;
		}
	}while(recibidos != tamano);

	*nuevo_paquete = armar_paquete(codigo, tamano, datos);

	return nuevo_paquete;
}

void destruir_paquete(t_paquete* paquete)
{
	if(paquete->cod_op != ERROR_COD_OP)
		free(paquete->datos);
	free(paquete);
}

int aceptar_nueva_conexion_multiconexion(int socket_escucha,fd_set* fds, int* fdmax) {
	int nueva_conexion = aceptar_cliente(socket_escucha);
	if (nueva_conexion != -1){
		FD_SET(nueva_conexion, fds);
		if (*fdmax < nueva_conexion)
			*fdmax = nueva_conexion;
	}
	return nueva_conexion;
}

bool recibio_datos(int socket){
	char c; //un char es el minimo tamano que puedo pedir (1 byte)
	ssize_t size = recv(socket,&c,sizeof(c),MSG_PEEK);

	return size > 0;
}

int correr_server_multiconexion(int* fdmax,fd_set* fds, int socket_escucha,void (*manejar_pedido)(int,t_paquete),void (*socket_cerrado)(int),void (*conexion_nueva_aceptada)(int)){
//manejar_pedido, socket_cerrado y conexion_nueva_aceptada son funciones que reciben el socket del cliente
	fd_set* set_ite = malloc(sizeof(fd_set)); //necesito trabajar sobre otro fd_set para hacer el select
	for (;;) {
		*set_ite = *fds;  //son structs simples, por lo que puedo copiarlos asi

		if (select(*fdmax + 1, set_ite, NULL, NULL, NULL) == -1) { //bloqueante. solo continua cuando paso algo en fds
			perror("Error en la funcion select()");
			return -1;
		}

		int socket_ite; //para iterar
		for (socket_ite = 0; socket_ite <= *fdmax; socket_ite++) {
			if (FD_ISSET(socket_ite, set_ite)) {  //si paso algo en socket_ite
				if (socket_ite == socket_escucha) { //si el socket donde paso algo es el que escucha es porque hay nueva conexion
					int conexion_nueva = aceptar_nueva_conexion_multiconexion(socket_escucha,fds,fdmax);
					if (conexion_nueva == -1)
						return -1;
					conexion_nueva_aceptada(conexion_nueva);
				} else{//si el socket donde paso algo NO es el que escucha es porque ese cliente se cerro o mando un mensaje
					if(recibio_datos(socket_ite)){ //si mando un mensaje
						t_paquete* paquete =  recibir_paquete(socket_ite);
						if (paquete->cod_op==ERROR_COD_OP) {
							return -1;
						}
						manejar_pedido(socket_ite,*paquete);
						destruir_paquete(paquete);
					} else{//si se desconecto el socket cliente
						socket_cerrado(socket_ite);
						FD_CLR(socket_ite, fds);
						close(socket_ite);
					}
				}
			}
		}
	}
	free(set_ite);
	return EXIT_SUCCESS;
}

int crear_server_multiconexion(fd_set* fds, int puerto, int* fdmax) {
	FD_ZERO(fds);
	int socket_escucha = crear_socket_escucha(puerto);
	if (socket_escucha != -1){
		FD_SET(socket_escucha, fds);
		*fdmax = socket_escucha;
	}
	return socket_escucha;
}
