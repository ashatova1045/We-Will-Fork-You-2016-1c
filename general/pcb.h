#ifndef PCB_H_
#define PCB_H_

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
	char* contenido_pcb;
	int32_t tamanio;
} __attribute__((__packed__)) t_pcb_serializado;

typedef struct {
	int32_t pag;
	int32_t offset;
	int32_t size;
} __attribute__((__packed__)) t_posMemoria;

typedef struct {
	char id;
	t_posMemoria posicionVar;
} __attribute__((__packed__)) t_variable;

typedef struct {
	int32_t posicion;

	int32_t cant_argumentos;
	t_posMemoria * argumentos;

	int32_t cant_variables;
	t_variable * variables;

	int32_t pos_retorno;
	t_posMemoria pos_var_retorno;
} __attribute__((__packed__)) registro_indice_stack;

typedef struct {
	int32_t pid;
	int32_t pc;
	int32_t cant_pags_totales; //total de paginas que pidio codigo+stack
	t_posMemoria fin_stack;

	u_int32_t cant_instrucciones;
	t_posMemoria* indice_codigo;

	u_int32_t tamano_etiquetas;
	char* indice_etiquetas;

	u_int32_t cant_entradas_indice_stack;
	registro_indice_stack* indice_stack;
}__attribute__((__packed__)) t_pcb;

typedef struct {
	t_pcb* pcb;
	char* semaforo;
	int32_t tiempo;
}__attribute__((__packed__)) t_pedido_wait;

typedef struct {
	int32_t tamano;
	char* contenido;
}__attribute__((__packed__)) t_pedido_wait_serializado;

t_pcb_serializado serializar(t_pcb pcb);
t_pcb* deserializar(char* pcbs);
void destruir_pcb (t_pcb* pcbADestruir);


t_pedido_wait deserializar_wait(char* serializado);
t_pedido_wait_serializado* serializar_wait(t_pedido_wait* pedido);

#endif /* PCB_H_ */
