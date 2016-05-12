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
	u_int32_t tamano_etiqueta;
	char* etiq; //Este es el id de una funcion o etiqueta de programa
	t_posMemoria pos_real; // La posicion que debe tomar el PC para arrancar a ejecutar la funicion o etiqueta del programa
} __attribute__((__packed__)) t_indice_etiq;

typedef struct {
	int32_t pid;
	int32_t pc;
	int32_t cant_pags_totales; //total de paginas que pidio codigo+stack
	int32_t fin_stack;

	u_int32_t cant_instrucciones;
	t_posMemoria* indice_codigo;

	int32_t cant_etiquetas;
	t_indice_etiq* indice_etiquetas;

	u_int32_t cant_entradas_indice_stack;
	registro_indice_stack* indice_stack;
}__attribute__((__packed__)) t_pcb;

t_pcb_serializado serializar(t_pcb pcb);
t_pcb* deserializar(char* pcbs);



#endif /* PCB_H_ */
