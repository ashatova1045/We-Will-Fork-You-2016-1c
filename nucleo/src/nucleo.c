#include <stdio.h>
#include <stdlib.h>
#include <parser/metadata_program.h>
#include "../../sockets/Sockets.h"
#include <commons/string.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/config.h>

typedef struct {
	int puerto;
	void (*manejar_pedido)(int,t_paquete);
	void (*socket_cerrado)(int);
	void (*conexion_nueva_aceptada)(int);
} t_estructura_server;

int cpu;

t_log* crearLog(){
	t_log *logNucleo = log_create("log.txt", "nucleo.c", false, LOG_LEVEL_INFO);
	return logNucleo;
}

typedef struct{
	int puerto_prog;
	int puerto_cpu;
	int quantum;
	int quantum_sleep;
	char** semaf_ids;
	char** semaf_init; //int* lo dejo como char** para usar get_array_value
	char** io_ids;
	char** io_retardo; //int* lo dejo como char** para usar get_array_value
	char** shared_vars;
} t_nucleoConfig;

		/*semaforo = malloc(sizeof(semaforo_t));
		semaforo->valor = atoi(valorSemaforosArray[i]);
		semaforo->cola = queue_create();
		usando el atoi dentro de un while*/

t_nucleoConfig* cargarConfiguracion(){
	t_nucleoConfig* datosNucleo = (t_nucleoConfig*)malloc(sizeof(t_nucleoConfig));
	t_config* config = config_create("../nucleo/nucleo.cfg");
	datosNucleo->puerto_prog=config_get_int_value(config,"PUERTO_PROG");
	datosNucleo->puerto_cpu=config_get_int_value(config,"PUERTO_CPU");
	datosNucleo->quantum=config_get_int_value(config,"QUANTUM");
	datosNucleo->quantum_sleep=config_get_int_value(config,"QUANTUM_SLEEP");
	datosNucleo->semaf_ids=config_get_array_value(config,"SEMAF_IDS");
	datosNucleo->semaf_init=config_get_array_value(config,"SEMAF_INIT");
	datosNucleo->io_ids=config_get_array_value(config,"IO_IDS");
	datosNucleo->io_retardo=config_get_array_value(config,"IO_RETARDO");
	datosNucleo->shared_vars=config_get_array_value(config,"SHARED_VARS");
	config_destroy(config);
	return datosNucleo;
}

void destruirNucleoConfig(t_nucleoConfig* datosADestruir){
	if (!datosADestruir)
		return;
	free(datosADestruir->io_ids);
	free(datosADestruir->io_retardo);
	free(datosADestruir->semaf_ids);
	free(datosADestruir->semaf_init);
	free(datosADestruir->shared_vars);
	free(datosADestruir);
}

void manejar_socket_consola(int socket,t_paquete paquete){
	printf("Llego un pedido de %d\n",socket);
	printf("El socket %d dice:\n",socket);
	puts(paquete.datos);
	enviar(1,paquete.tamano_datos,paquete.datos,cpu);
	//TODO atender pedidos de la consola
}

void cerrar_socket_consola(int socket){
	printf("Se cerro %d\n",socket);
	//TODO manejar el cierre de socket de la consola
}


void nueva_conexion_consola(int socket){
	printf("Se conecto %d\n",socket);
	//TODO crear PCB
	//TODO pedir espacio a UMC y enviar codigo del programa y paginas, y luego almacenar estructuras.
}



void manejar_socket_cpu(int socket,t_paquete paquete){
	printf("Llego un pedido de %d\n",socket);
	printf("El socket %d dice:\n",socket);
	puts(paquete.datos);
	//TODO manejar pedidos del CPU
}

void cerrar_socket_cpu(int socket){
	printf("Se cerro %d\n",socket);
	//TODO manejar desconexion de un cpu
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

	//Crear archivo de log
	t_log* logNucleo = crearLog();
	log_info(logNucleo, "Ejecucion del Proceso NUCLEO");

	//Levantar archivo de config del proceso Nucleo
	t_nucleoConfig* config_nucleo = cargarConfiguracion();
	printf("El puerto para Proceso Consola es: %d \n", config_nucleo->puerto_prog);
	printf("El puerto para Proceso CPU es: %d \n", config_nucleo->puerto_cpu);
	//TODO  setear el kernel agregar puerto/ip de la UMC

	//TODO conectarme con UMC
	t_estructura_server conf_consola;
	conf_consola.conexion_nueva_aceptada = nueva_conexion_consola;
	conf_consola.manejar_pedido = manejar_socket_consola;
	conf_consola.socket_cerrado = cerrar_socket_consola;
	conf_consola.puerto=config_nucleo->puerto_prog;

	t_estructura_server conf_cpu;
	conf_cpu.conexion_nueva_aceptada=nueva_conexion_cpu;
	conf_cpu.manejar_pedido=manejar_socket_cpu;
	conf_cpu.socket_cerrado=cerrar_socket_cpu;
	conf_cpu.puerto= config_nucleo->puerto_cpu;

	pthread_t thread_actual;
	if (pthread_create(&thread_actual, NULL, (void*)funcion_hilo_servidor, &conf_cpu)){
	        perror("Error el crear el thread.");
	        exit(EXIT_FAILURE);
	    }

	funcion_hilo_servidor(&conf_consola); //FIXME pasar a un hilo para que deje de ser bloqueante


	//TODO planificacion de los procesos

	destruirNucleoConfig(config_nucleo);

	return EXIT_SUCCESS;

}
