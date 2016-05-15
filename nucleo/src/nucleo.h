/*
 * nucleo.h
 *
 *  Created on: 14/5/2016
 *      Author: utnso
 */

#ifndef NUCLEO_H_
#define NUCLEO_H_

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


t_nucleoConfig* cargarConfiguracion(t_config* config);

t_log* crearLog();

void destruirNucleoConfig(t_nucleoConfig* datosADestruir);

void manejar_socket_consola(int socket,t_paquete paquete);

void funcion_hilo_servidor(t_estructura_server *conf_server);



#endif /* NUCLEO_H_ */
