#include "pcb.h"
#include <stdio.h>


int main(int argc, char **argv){
		t_pcb pcb_prueba;
		pcb_prueba.cant_etiquetas=3;
		pcb_prueba.cant_instrucciones=4;
		pcb_prueba.indice_codigo=malloc(sizeof(t_posMemoria)*pcb_prueba.cant_instrucciones);
		pcb_prueba.indice_etiquetas=malloc(sizeof(t_indice_etiq)*pcb_prueba.cant_etiquetas);

		t_posMemoria indice_codigo_prueba;
		t_posMemoria indice_codigo_prueba2;
		t_indice_etiq indice_etiqueta_prueba,indice_etiqueta_prueba2,indice_etiqueta_prueba3;
		registro_indice_stack indice_stack_prueba;

		indice_codigo_prueba.pag=11;
		indice_codigo_prueba.offset=12;
		indice_codigo_prueba.size=13;

		indice_codigo_prueba2.pag=21;
		indice_codigo_prueba2.offset=22;
		indice_codigo_prueba2.size=33;

		indice_etiqueta_prueba.tamano_etiqueta=7;
		indice_etiqueta_prueba.etiq = "sarasa";
		indice_etiqueta_prueba.pos_real.pag=2;
		indice_etiqueta_prueba.pos_real.offset=10;
		indice_etiqueta_prueba.pos_real.size=19;

		indice_etiqueta_prueba2.tamano_etiqueta=10;
		indice_etiqueta_prueba2.etiq = "123456789";
		indice_etiqueta_prueba2.pos_real.pag=4;
		indice_etiqueta_prueba2.pos_real.offset=20;
		indice_etiqueta_prueba2.pos_real.size=38;

		indice_etiqueta_prueba3.tamano_etiqueta=5;
		indice_etiqueta_prueba3.etiq = "hola";
		indice_etiqueta_prueba3.pos_real.pag=8;
		indice_etiqueta_prueba3.pos_real.offset=40;
		indice_etiqueta_prueba3.pos_real.size=76;

		indice_stack_prueba.posicion=7;
		indice_stack_prueba.cant_argumentos=2;
		indice_stack_prueba.argumentos=malloc(sizeof(t_posMemoria)*indice_stack_prueba.cant_argumentos);
		indice_stack_prueba.argumentos[0]=indice_codigo_prueba2;
		indice_stack_prueba.argumentos[1]=indice_codigo_prueba;
		indice_stack_prueba.cant_variables=3;
		indice_stack_prueba.variables=malloc(sizeof(t_variable)*indice_stack_prueba.cant_variables);
		indice_stack_prueba.variables[0].id='a';
		indice_stack_prueba.variables[0].posicionVar=indice_codigo_prueba;
		indice_stack_prueba.variables[1].id='b';
		indice_stack_prueba.variables[1].posicionVar=indice_codigo_prueba2;
		indice_stack_prueba.variables[2].id='c';
		indice_stack_prueba.variables[2].posicionVar=indice_codigo_prueba;
		indice_stack_prueba.pos_retorno=28;
		indice_stack_prueba.pos_var_retorno=indice_codigo_prueba2;


		pcb_prueba.pid=1;
		pcb_prueba.pc=0;
		pcb_prueba.cant_pags_totales=15;
		pcb_prueba.fin_stack=0;

		pcb_prueba.indice_codigo[0]=indice_codigo_prueba;
		pcb_prueba.indice_codigo[1]=indice_codigo_prueba2;
		pcb_prueba.indice_codigo[2]=indice_etiqueta_prueba2.pos_real;
		pcb_prueba.indice_codigo[3]=indice_etiqueta_prueba3.pos_real;


		pcb_prueba.indice_etiquetas[0]=indice_etiqueta_prueba;
		pcb_prueba.indice_etiquetas[1]=indice_etiqueta_prueba2;
		pcb_prueba.indice_etiquetas[2]=indice_etiqueta_prueba3;

		pcb_prueba.cant_entradas_indice_stack=2;
		pcb_prueba.indice_stack=malloc(sizeof(registro_indice_stack)*pcb_prueba.cant_entradas_indice_stack);
		pcb_prueba.indice_stack[0]=indice_stack_prueba;
		pcb_prueba.indice_stack[1]=indice_stack_prueba;
		pcb_prueba.indice_stack[1].cant_argumentos = 2; //no serializa el 3ro
		pcb_prueba.indice_stack[1].cant_variables = 2; //no serializa el 3ro
		pcb_prueba.indice_stack[1].variables[0].id = 'm';
		pcb_prueba.indice_stack[1].variables[1].id = 'z';



puts("a punto de serializar");
		t_pcb_serializado pcbs = serializar(pcb_prueba);
		printf("pesa %d\n\n",pcbs.tamanio);
		t_pcb* nuevo = deserializar(pcbs.contenido_pcb);
		int i;
		for(i=0;i<nuevo->cant_instrucciones;i++)
			printf("instruccion %d. pag %d offset %d size %d \n",i,nuevo->indice_codigo[i].pag,nuevo->indice_codigo[i].offset,nuevo->indice_codigo[i].size);

		printf("\n\n");

		for(i=0;i<nuevo->cant_etiquetas;i++)
			printf("etiqueta %d. tamano %d etiq %s.posreal: pag %d offset %d size %d \n",i,nuevo->indice_etiquetas[i].tamano_etiqueta,nuevo->indice_etiquetas[i].etiq,nuevo->indice_etiquetas[i].pos_real.pag,nuevo->indice_etiquetas[i].pos_real.offset,nuevo->indice_etiquetas[i].pos_real.size);

		printf("\n\n");

		for(i=0;i<nuevo->cant_entradas_indice_stack;i++){
			printf("entrada de stack %d\n",i);
			int j;
			for(j=0;j<nuevo->indice_stack[i].cant_argumentos;j++)
				printf("argumento %d. pag %d offset %d size %d  \n",j,nuevo->indice_stack[i].argumentos[j].pag,nuevo->indice_stack[i].argumentos[j].offset,nuevo->indice_stack[i].argumentos[j].size);
			for(j=0;j<nuevo->indice_stack[i].cant_variables;j++)
				printf("variable %c. pag %d offset %d size %d  \n",nuevo->indice_stack[i].variables[j].id,nuevo->indice_stack[i].variables[j].posicionVar.pag,nuevo->indice_stack[i].variables[j].posicionVar.offset,nuevo->indice_stack[i].variables[j].posicionVar.size);
		}
destruir_pcb(nuevo);
puts("ejecute destruir_pcb");
	return EXIT_SUCCESS;
}
