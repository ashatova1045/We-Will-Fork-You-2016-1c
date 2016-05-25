#include "pcb.h"
#include <stdlib.h>

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

			sizeof(pcb.tamano_etiquetas)+
			pcb.tamano_etiquetas+

			sizeof(pcb.cant_entradas_indice_stack)+
			tamanio_indice_stack(pcb);
}

int agregar(char *to,int32_t tamano, void* from){
	memcpy(to,from,tamano);
	return tamano;
}

t_pcb_serializado serializar(t_pcb pcb){
	t_pcb_serializado pcb_serializado;
	pcb_serializado.tamanio = tamanio_pcb(pcb);
	pcb_serializado.contenido_pcb = malloc(pcb_serializado.tamanio);
	int i;
	int offset=0;

	offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.pid),&pcb.pid);
	offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.pc),&pcb.pc);
	offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.cant_pags_totales),&pcb.cant_pags_totales);
	offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.fin_stack),&pcb.fin_stack);

	offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.cant_instrucciones),&pcb.cant_instrucciones);
	for(i=0;i<pcb.cant_instrucciones;i++)
		offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(t_posMemoria) ,&pcb.indice_codigo[i]);

	offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.tamano_etiquetas),&pcb.tamano_etiquetas);
	offset+=agregar(pcb_serializado.contenido_pcb+offset,pcb.tamano_etiquetas,pcb.indice_etiquetas);

	offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.cant_entradas_indice_stack),&pcb.cant_entradas_indice_stack);
	for(i=0;i<pcb.cant_entradas_indice_stack;i++){
		int j;
		offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.indice_stack->posicion),&pcb.indice_stack[i].posicion);

		offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.indice_stack->cant_argumentos),&pcb.indice_stack[i].cant_argumentos);
		for(j=0;j<pcb.indice_stack[i].cant_argumentos;j++){
			offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(t_posMemoria),&pcb.indice_stack[i].argumentos[j]);
		}

		offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.indice_stack->cant_variables),&pcb.indice_stack[i].cant_variables);
		for(j=0;j<pcb.indice_stack[i].cant_variables;j++){
			offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(t_variable),&pcb.indice_stack[i].variables[j]);
		}

		offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.indice_stack->pos_retorno),&pcb.indice_stack[i].pos_retorno);
		offset+=agregar(pcb_serializado.contenido_pcb+offset,sizeof(pcb.indice_stack->pos_var_retorno),&pcb.indice_stack[i].pos_var_retorno);
	}

	return pcb_serializado;
}

t_pcb* deserializar(char* pcbs)
{
	int i;
	t_pcb *pcb = malloc(sizeof(t_pcb));

	int offset = 0;

	pcb->pid = pcbs[offset];
	offset += sizeof(pcb->pid);

	pcb->pc = pcbs[offset];
	offset += sizeof(pcb->pc);

	pcb->cant_pags_totales = pcbs[offset];
	offset += sizeof(pcb->cant_pags_totales);

	pcb->fin_stack = *((t_posMemoria*)(pcbs+offset));
	offset += sizeof(pcb->fin_stack);

	pcb->cant_instrucciones = pcbs[offset];
	offset += sizeof(pcb->cant_instrucciones);

	pcb->indice_codigo = malloc(pcb->cant_instrucciones*sizeof(t_posMemoria));
	for(i=0;i < pcb->cant_instrucciones;i++){
		pcb->indice_codigo[i] = *((t_posMemoria*)(pcbs+offset));
		offset += sizeof(t_posMemoria);
	}

	pcb->tamano_etiquetas = pcbs[offset];
	offset += sizeof(pcb->tamano_etiquetas);

	pcb->indice_etiquetas = malloc(pcb->tamano_etiquetas*sizeof(char));
	memcpy(pcb->indice_etiquetas,pcbs+offset,pcb->tamano_etiquetas*sizeof(char));
	offset += pcb->tamano_etiquetas;

	pcb->cant_entradas_indice_stack = pcbs[offset];
	offset += sizeof(pcb->cant_entradas_indice_stack);

	pcb->indice_stack = malloc(pcb->cant_entradas_indice_stack * sizeof(registro_indice_stack));
	for(i=0;i<pcb->cant_entradas_indice_stack;i++){
		int j;
		pcb->indice_stack[i].posicion = pcbs[offset];
		offset += sizeof(pcb->indice_stack->posicion);

		pcb->indice_stack[i].cant_argumentos = pcbs[offset];
		offset += sizeof(pcb->indice_stack->cant_argumentos);

		pcb->indice_stack[i].argumentos = malloc(sizeof(t_posMemoria)*pcb->indice_stack[i].cant_argumentos);
		for(j=0;j<pcb->indice_stack[i].cant_argumentos;j++){
			pcb->indice_stack[i].argumentos[j] = *((t_posMemoria*)(pcbs+offset));
			offset +=sizeof(t_posMemoria);
		}

		pcb->indice_stack[i].cant_variables = pcbs[offset];
		offset += sizeof(pcb->indice_stack->cant_variables);

		pcb->indice_stack[i].variables = malloc(sizeof(t_variable) * pcb->indice_stack[i].cant_variables);
		for(j=0;j<pcb->indice_stack[i].cant_variables;j++){
			pcb->indice_stack[i].variables[j] = *((t_variable*)(pcbs+offset));
			offset +=sizeof(t_variable);
		}

		pcb->indice_stack[i].pos_retorno = pcbs[offset];
		offset += sizeof(pcb->indice_stack->pos_retorno);

		pcb->indice_stack[i].pos_var_retorno = *((t_posMemoria*)(pcbs+offset));
		offset += sizeof(pcb->indice_stack->pos_var_retorno);
	}

	return pcb;
}


void destruir_pcb (t_pcb *pcbADestruir){
	int i;

	if(pcbADestruir==NULL)
		return;

	free(pcbADestruir->indice_codigo); //hago este free por que se asigno la memoria con un unico malloc

	free(pcbADestruir->indice_etiquetas);

	for(i=0; i <(pcbADestruir->cant_entradas_indice_stack) ; i++){
		free(pcbADestruir->indice_stack[i].argumentos);
		free(pcbADestruir->indice_stack[i].variables);
	}
	free(pcbADestruir->indice_stack);
	free(pcbADestruir);
}

t_pedido_wait_serializado* serializar_wait(t_pedido_wait* pedido){
	int tamano_semaforo = strlen(pedido->semaforo)+1;

	t_pcb_serializado pcbserializado =  serializar(*pedido->pcb);

	t_pedido_wait_serializado* ser = malloc(sizeof(t_pedido_wait_serializado));
	ser->tamano = sizeof(pcbserializado.tamanio)+pcbserializado.tamanio+tamano_semaforo;
	ser->contenido = malloc(ser->tamano);
	int offset = 0;

	strcpy(ser->contenido,pedido->semaforo);
	offset+=tamano_semaforo;

	memcpy(ser->contenido+offset,&pcbserializado.tamanio,sizeof(pcbserializado.tamanio));
	offset+=sizeof(pcbserializado.tamanio);

	memcpy(ser->contenido+offset,pcbserializado.contenido_pcb,pcbserializado.tamanio);

	free(pcbserializado.contenido_pcb);

	return ser;
}

t_pedido_wait deserializar_wait(char* serializado){
	t_pedido_wait pedido;

	int tamano_sem = strlen(serializado)+1;
	pedido.semaforo=malloc(tamano_sem);
	strcpy(pedido.semaforo,serializado);

	int offset = tamano_sem;

	pedido.pcb = deserializar(serializado+sizeof(int32_t)+offset);

	return pedido;
}