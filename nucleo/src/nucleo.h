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

t_dictionary *semaforos;
t_dictionary *entradasalida;
t_dictionary *variablesCompartidas;

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

typedef struct{
	int socket;
	t_pcb* pcb;
	t_paquete *paquete;
	t_metadata_program* metadata;
} t_new_ready_args;


void cargar_cpu(int32_t socket);
void cargar_programa(int32_t socket, int pid);
void relacionar_cpu_programa(t_cpu *cpu, t_consola *programa, t_pcb *pcb);
void liberar_una_relacion(t_pcb *pcb_devuelto);
void liberar_una_relacion_porsocket_cpu(int socketcpu);
t_consola* matchear_consola_por_pid(int pid);
t_relacion* matchear_relacion_por_socketcpu(int socket);
void elminar_consola_por_socket(int socket);
t_log* crearLog();
t_nucleoConfig* cargarConfiguracion(t_config* config);
void destruirNucleoConfig(t_nucleoConfig* datosADestruir);
t_pcb* armar_nuevo_pcb (t_paquete paquete,t_metadata_program* metadata);
char* armar_codigo(t_pcb* nuevo_pcb,char* codigo,t_metadata_program* metadata);
void enviar_a_cpu();
bool inicializar_programa(t_pcb* nuevo_pcb,t_paquete paquete, t_metadata_program* metadata);
void manejar_socket_consola(int socket,t_paquete paquete);
void entrada_salida(t_pedido_wait *pedido);
void manejar_socket_cpu(int socket,t_paquete paquete);
void cerrar_socket_cpu(int socket);
void funcion_hilo_servidor(t_estructura_server *conf_server);
void crear_semaforos();
void crear_dispositivos_es();
void cargar_varCompartidas();

#endif /* NUCLEO_H_ */
