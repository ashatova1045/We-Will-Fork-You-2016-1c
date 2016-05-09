#include "pcb.h"
#include <stdio.h>


int main(int argc, char **argv){
		t_pcb pcb_prueba;
		pcb_prueba.cant_etiquetas=1;
		pcb_prueba.cant_instrucciones=1;
		pcb_prueba.indice_codigo=malloc(sizeof(t_posMemoria)*pcb_prueba.cant_instrucciones);
		pcb_prueba.indice_etiquetas=malloc(sizeof(t_indice_etiq)*pcb_prueba.cant_etiquetas);

		t_posMemoria indice_codigo_prueba;
		t_posMemoria indice_codigo_prueba2;
		t_indice_etiq indice_etiqueta_prueba;
		registro_indice_stack indice_stack_prueba;

		indice_codigo_prueba.pag=11;
		indice_codigo_prueba.offset=12;
		indice_codigo_prueba.size=13;

		indice_codigo_prueba2.pag=21;
		indice_codigo_prueba2.offset=22;
		indice_codigo_prueba2.size=33;

		indice_etiqueta_prueba.tamano_etiqueta=7;
		indice_etiqueta_prueba.etiq = malloc(7);
		indice_etiqueta_prueba.etiq="sarasa";
		indice_etiqueta_prueba.pos_real.pag=2;
		indice_etiqueta_prueba.pos_real.offset=10;
		indice_etiqueta_prueba.pos_real.size=19;


		indice_stack_prueba.posicion=7;
		indice_stack_prueba.cant_argumentos=1;
		indice_stack_prueba.argumentos=malloc(sizeof(t_posMemoria)*indice_stack_prueba.cant_argumentos);
		indice_stack_prueba.argumentos[0]=indice_codigo_prueba2;
		indice_stack_prueba.cant_variables=1;
		indice_stack_prueba.variables=malloc(sizeof(t_variable)*indice_stack_prueba.cant_variables);
		indice_stack_prueba.variables->id=1;
		indice_stack_prueba.variables[0].posicionVar=indice_codigo_prueba;
		indice_stack_prueba.pos_retorno=28;
		indice_stack_prueba.pos_var_retorno=indice_codigo_prueba2;


		pcb_prueba.pid=1;
		pcb_prueba.pc=0;
		pcb_prueba.cant_pags_totales=15;
		pcb_prueba.fin_stack=0;
		pcb_prueba.indice_codigo[0]=indice_codigo_prueba;
		pcb_prueba.indice_etiquetas[0]=indice_etiqueta_prueba;
		pcb_prueba.cant_entradas_indice_stack=1;
		pcb_prueba.indice_stack=malloc(sizeof(registro_indice_stack)*pcb_prueba.cant_entradas_indice_stack);
		pcb_prueba.indice_stack[0]=indice_stack_prueba;



puts("a punto de serializar");
		t_pcb_serializado pcbs = serializar(pcb_prueba);
		printf("pesa %d\n",pcbs.tamanio);
		t_pcb* nuevo = deserializar(pcbs.contenido_pcb);
		nuevo->indice_codigo[0];
		nuevo->indice_codigo[1];

	return EXIT_SUCCESS;
}
