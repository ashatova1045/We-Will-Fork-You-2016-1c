#include <stdio.h>
#include <stdlib.h>
#include <parser/metadata_program.h>
#include "/home/utnso/workspace/tp-2016-1c-We-Will-Fork-You/sockets/Sockets.h"
#include <commons/string.h>
#include <commons/collections/list.h>
#include <pthread.h>

typedef struct {
	int puerto;
	void (*manejar_pedido)(int,t_paquete);
	void (*socket_cerrado)(int);
	void (*conexion_nueva_aceptada)(int);
} t_estructura_server;
int cpu;

void manejar_socket_consola(int socket,t_paquete paquete){
	printf("Llego un pedido de %d\n",socket);
	printf("El socket %d dice:\n",socket);
	puts(paquete.datos);
	enviar(1,paquete.tamano_datos,paquete.datos,cpu);
}

void cerrar_socket_consola(int socket){
	printf("Se cerro %d\n",socket);
}

void nueva_conexion_consola(int socket){
	printf("Se conecto %d\n",socket);
}



void manejar_socket_cpu(int socket,t_paquete paquete){
	printf("Llego un pedido de %d\n",socket);
	printf("El socket %d dice:\n",socket);
	puts(paquete.datos);
}

void cerrar_socket_cpu(int socket){
	printf("Se cerro %d\n",socket);
}


void nueva_conexion_cpu(int socket){
	printf("Se conecto %d\n",socket);
	cpu = socket;
}


void funcion_hilo_servidor(t_estructura_server *conf){

	fd_set set_de_fds;
	int* fdmax = malloc(sizeof(int));
	int socketserver = crear_server_multiconexion(&set_de_fds,conf->puerto,fdmax);
	printf("Se creo un socket multiconexion. Su fd es: %d \n",socketserver);
	puts("Escuchando conexiones y corriendo!");
	correr_server_multiconexion(fdmax,&set_de_fds,socketserver,conf->manejar_pedido,conf->socket_cerrado,conf->conexion_nueva_aceptada);

	close(socketserver);
	free(fdmax);

}
int main(int argc, char **argv){
	t_estructura_server conf_consola;
	conf_consola.conexion_nueva_aceptada = nueva_conexion_consola;
	conf_consola.manejar_pedido = manejar_socket_consola;
	conf_consola.socket_cerrado = cerrar_socket_consola;
	conf_consola.puerto=4000;

	t_estructura_server conf_cpu;
	conf_cpu.conexion_nueva_aceptada=nueva_conexion_cpu;
	conf_cpu.manejar_pedido=manejar_socket_cpu;
	conf_cpu.socket_cerrado=cerrar_socket_cpu;
	conf_cpu.puerto=4050;

	pthread_t thread_actual;
	if (pthread_create(&thread_actual, NULL, (void*)funcion_hilo_servidor, &conf_cpu)){
	        perror("Error el crear el thread.");
	        exit(EXIT_FAILURE);
	    }

	funcion_hilo_servidor(&conf_consola);


	return EXIT_SUCCESS;

}
