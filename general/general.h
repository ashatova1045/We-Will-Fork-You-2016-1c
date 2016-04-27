#ifndef GENERAL_H_
#define GENERAL_H_

#include <sys/types.h>

typedef struct
{
	char nombre;
	int32_t valor;
}t_variable_completa;

typedef struct {
	int pid;
	int pc;
	int pag_codigo;
	int indice_codigo;
	int indice_etiquetas;
	int fin_stack;
}t_pcb;

#endif /* GENERAL_H_ */
