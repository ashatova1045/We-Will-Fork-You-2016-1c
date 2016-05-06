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
	char* etiq; //Este es el id de una funcion o etiqueta de programa
	t_posMemoria pos_real; // La posicion que debe tomar el PC para arrancar a ejecutar la funicion o etiqueta del programa
} t_indice_etiq;

typedef struct {
	int32_t pid;
	int32_t pc;
	int32_t pag_codigo;
	t_posMemoria* indice_codigo;
	t_indice_etiq* indice_etiquetas;
	int32_t cant_etiquetas;
	int32_t fin_stack;
	u_int32_t cant_instrucciones;
	registro_indice_stack* indice_stack;
}t_pcb;



#endif /* GENERAL_H_ */
