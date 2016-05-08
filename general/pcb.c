/*
 * pcb.c
 *
 *  Created on: 8/5/2016
 *      Author: utnso
 */
#include "pcb.h"

int32_t tamano_etiqueta(t_indice_etiq etiqueta){
	return sizeof(etiqueta.pos_real)+
			sizeof(etiqueta.tamano_etiqueta)+
			sizeof(char) * etiqueta.tamano_etiqueta;
}

int32_t tamanio_indice_etiquetas(t_pcb pcb){
	int i;
	int32_t contador = 0;
	for(i=0; i < (pcb.cant_etiquetas); i++)
		contador += tamano_etiqueta(pcb.indice_etiquetas[i]);

	return contador;
}

int32_t tamanio_indice_stack(t_pcb pcb){
	int i;
	int32_t contador = 0;
	for(i=0; i < (pcb.cant_entradas_indice_stack); i++)
		contador += sizeof(pcb.indice_stack->posicion) +

					sizeof(pcb.indice_stack->cant_argumentos) +
					sizeof(t_posMemoria) * pcb.indice_stack[i].cant_argumentos+

					sizeof(pcb.indice_stack->cant_variables) +
					sizeof(t_variable) * pcb.indice_stack[i].cant_variables+

					sizeof(pcb.indice_stack->pos_retorno)+
					sizeof(pcb.indice_stack->pos_var_retorno);

	return contador;
}

int32_t tamanio_pcb(t_pcb pcb){
	return
			sizeof(pcb.pid)+
			sizeof(pcb.pc)+
			sizeof(pcb.cant_pags_totales)+
			sizeof(pcb.fin_stack)+

			sizeof(pcb.cant_instrucciones)+
			sizeof(t_posMemoria) * pcb.cant_instrucciones +

			sizeof(pcb.cant_etiquetas)+
			tamanio_indice_etiquetas(pcb)+

			sizeof(pcb.cant_entradas_indice_stack)+
			tamanio_indice_stack(pcb);
}

int agregar(char *to,int32_t tamano, void* from){
	memcpy(to,from,tamano);
	return tamano;
}

t_pcb_serializado serializar(t_pcb pcb)
{
	t_pcb_serializado pcb_serializado;
	pcb_serializado.tamanio = tamanio_pcb(pcb);
	pcb_serializado.contenido_pcb = malloc(pcb_serializado.tamanio);

	int i;
	int offset=0;
	int32_t proximo_tamano_a_serializar;

	offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.pid),&pcb.pid);
	offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.pc),&pcb.pc);
	offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.cant_pags_totales),&pcb.cant_pags_totales);
	offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.fin_stack),&pcb.fin_stack);

	offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.cant_instrucciones),&pcb.cant_instrucciones);
	for(i=0;i<pcb.cant_instrucciones;i++)
		offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(t_posMemoria) ,pcb.indice_codigo+i);

	offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.cant_etiquetas),&pcb.cant_etiquetas);
	for(i=0;i<pcb.cant_etiquetas;i++){
		offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.indice_etiquetas->tamano_etiqueta),&pcb.indice_etiquetas[i].tamano_etiqueta);
		offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.indice_etiquetas->pos_real),&pcb.indice_etiquetas[i].pos_real);
		offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.indice_etiquetas[i].tamano_etiqueta),pcb.indice_etiquetas[i].etiq);
	}

	offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.cant_entradas_indice_stack),&pcb.cant_entradas_indice_stack);
	for(i=0;i<pcb.cant_entradas_indice_stack;i++){
		int j;
		offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.indice_stack->posicion),&pcb.indice_stack[i].posicion);

		offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.indice_stack->cant_argumentos),&pcb.indice_stack[i].cant_argumentos);
		for(j=0;j<pcb.indice_stack->cant_argumentos;j++){
			offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(t_posMemoria),&pcb.indice_stack[i].argumentos[j]);
		}

		offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.indice_stack->cant_variables),&pcb.indice_stack[i].cant_variables);
		for(j=0;j<pcb.indice_stack->cant_variables;j++){
			offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(t_variable),&pcb.indice_stack[i].variables[j]);
		}

		offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.indice_stack->pos_retorno),&pcb.indice_stack[i].pos_retorno);
		offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.indice_stack->pos_var_retorno),&pcb.indice_stack[i].pos_var_retorno);
	}

	return pcb_serializado;
}

destruir_pcb_serializado(t_pcb_serializado pcbs){
	free(pcbs.contenido_pcb);
}

void deserializar(t_pcb pcb)
{

}
