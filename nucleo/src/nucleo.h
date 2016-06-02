/*
 * nucleo.h
 *
 *  Created on: 14/5/2016
 *      Author: utnso
 */

#ifndef NUCLEO_H_
#define NUCLEO_H_
#include <stdbool.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/dictionary.h>

t_dictionary *semaforos;
t_dictionary *entradasalida;

typedef struct {
	int valor;
	t_queue* cola;
} t_semaforo;

typedef struct {
	int retardo;
	pthread_mutex_t mutex;
} t_dispositivo_io;

typedef struct {
	int puerto;
	void (*manejar_pedido)(int,t_paquete);
	void (*socket_cerrado)(int);
	void (*conexion_nueva_aceptada)(int);
} t_estructura_server;

typedef struct{
	int puerto;
	char* direccion;
}t_estructura_cliente;

typedef struct{
	int32_t puerto_prog;
	int32_t puerto_cpu;
	int32_t puerto_umc;
	char* ip_umc;
	int32_t quantum;
	int32_t quantum_sleep;
	char** semaf_ids;
	char** semaf_init; //int* lo dejo como char** para usar get_array_value
	char** io_ids;
	char** io_retardo; //int* lo dejo como char** para usar get_array_value
	char** shared_vars;
	int32_t tamano_stack;
} t_nucleoConfig;

typedef struct{
	int socket;
	bool corriendo;
}t_cpu;

typedef struct{
	int pid;
	int socket;
	bool corriendo;
}t_consola;

typedef struct{
	t_cpu *cpu;
	t_consola *programa;

}t_relacion;

t_nucleoConfig* cargarConfiguracion(t_config* config);

t_log* crearLog();

void destruirNucleoConfig(t_nucleoConfig* datosADestruir);

void manejar_socket_consola(int socket,t_paquete paquete);

void funcion_hilo_servidor(t_estructura_server *conf_server);

void relacionar_cpu_programa(t_cpu* cpu, t_consola* programa,t_pcb* pcb);


#endif /* NUCLEO_H_ */
