#ifndef PRIMITIVAS_H_
#define PRIMITIVAS_H_

#include <stdio.h>
#include <string.h>
#include "../../general/pcb.h"
#include <parser/parser.h>
#include "cpu.h"

typedef struct{
	t_nombre_dispositivo dispositivo;
	int tiempo;
}t_entrada_salida;


/*che esto no estaria mal? t_nombre_variable es un solo char y no char*
 * deberia usar t_nombre_compartida y valor deberia ser t_valor_variable */
typedef struct{
	t_nombre_variable variable;
	void* valorGrabar;
}t_grabar_valor;

typedef struct{
	int offset;
}t_posicion;

typedef struct{
	t_posicion direccion_variable;
	t_valor_variable valor;
}t_asignar;

AnSISOP_funciones functions;
AnSISOP_kernel kernel_functions;

void inicializar_primitivas();
t_posicion* definirVariable(t_nombre_variable identificador_variable);
t_posicion* obtenerPosicionVariable(t_nombre_variable identificador_variable);
t_valor_variable dereferenciar(t_posicion* direccion_variable);
void asignar(t_posicion	direccion_variable,	t_valor_variable valor);
t_valor_variable	obtenerValorCompartida(t_nombre_compartida	variable);
t_valor_variable asignarValorCompartida(t_nombre_compartida	variable, t_valor_variable valor_variable);
t_puntero_instruccion irAlLabel(t_nombre_etiqueta etiqueta);
void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar);
t_puntero_instruccion retornar(t_valor_variable retorno);
int	imprimir(t_valor_variable valor_mostrar);
void imprimirTexto(char*);
void entradaSalida(t_nombre_dispositivo, int);
void wait(t_nombre_semaforo identificador_semaforo);
void signal(t_nombre_semaforo identificador_semaforo);
int grabar_valor(t_nombre_variable identificador_variable, void* valorGrabar);
void finalizar();

#endif /* PRIMITIVAS_H_ */
