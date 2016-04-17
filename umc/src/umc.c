#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>//Incluyo commons
#include <commons/collections/list.h> //Incluyo commons
#include <commons/string.h>
#include <parser/metadata_program.h>//Incluyo el parser
#include "../../sockets/Sockets.h"

int socketswap;

//Función para manejar los mensajes
void manejar_paquete(int socket,t_paquete paq){
	printf("Llego un pedido de conexion de %d\n",socket);
	printf("El socket %d dice:\n",socket);
	puts(paq.datos);
	//Envío datos recibidos al área de swap
	enviar(1,paq.tamano_datos,paq.datos,socketswap);
	//destruir_paquete(paq);
}

//Cerrar puerto de socket conectado
void cerrar_conexion(int socket){
	printf("Se cerro %d\n",socket);
}

//Recibe nuevas conexiones
void nueva_conexion(int socket){
	printf("Se conecto %d\n",socket);
}
//Creo el socket al que le voy a mandar el mensaje

int main(int argc, char **argv){

	//Me conecto al área de swap
	socketswap = conectar("192.168.43.188",4100);

	//Creo el server multiconección
	fd_set set_de_fds;
	int* fdmax = malloc(sizeof(int));
	if (fdmax==NULL){
		perror("Error al alocar memoria.");
		return EXIT_FAILURE;
	}
	int puerto=4200;
	int socketServer = crear_server_multiconexion(&set_de_fds,puerto,fdmax);

	//Mensajes de conexión exitosa
	printf("Se creo un socket multiconexion. Su fd es: %d \n",socketServer);
	puts("Escuchando conexiones y corriendo!");

	//correr_server_multiconexion(fdmax,&set_de_fds,socketServer,manejar_paquete,cerrar_conexion,nueva_conexion);
	correr_server_multiconexion(fdmax,&set_de_fds,socketServer,manejar_paquete,cerrar_conexion,nueva_conexion);

	//Cierro el puerto y libero la memoria del socket
	close(socketServer);
	free(fdmax);
	return EXIT_SUCCESS;
}




