#ifndef GENERAL_H_
#define GENERAL_H_

#include <sys/types.h>
#include <parser/metadata_program.h>

typedef struct
{
	char nombre;
	int32_t valor;
}t_variable_completa;

typedef struct {
	int32_t pag;
	int32_t offset;
	int32_t size;
} t_posMemoria;

typedef struct {
	char id;
	t_posMemoria posicionVar;
} t_variable;

typedef struct {
	int32_t posicion;
	t_posMemoria * argumentos;
	t_variable * variables;
	int32_t pos_retorno;
	t_posMemoria * pos_var_retorno;
} registro_indice_stack;

typedef struct {
	int32_t pid;
	int32_t pc;
	int32_t pag_codigo;
	t_intructions* indice_codigo;
	int32_t indice_etiquetas;
	int32_t cant_etiquetas;
	int32_t fin_stack;
	u_int32_t cant_instrucciones;
	registro_indice_stack* indice_stack;
}t_pcb;



#endif /* GENERAL_H_ */
