/*
 * estados.c
 *
 *  Created on: 6/5/2016
 *      Author: utnso
 */

#include "estados.h"

t_queue *colaNew, *colaReady, *colaExec, *colaBlocked, *colaExit;
t_log *logNucleo_estados;
t_list *lista_programas_actuales;


void crear_colas()
{
	colaNew = queue_create();
	colaReady = queue_create();
	colaExec = queue_create();
	colaBlocked = queue_create();
	colaExit = queue_create();

	lista_programas_actuales = list_create();
}

void destruir_colas()
{
	queue_destroy_and_destroy_elements(colaNew,free);
	queue_destroy_and_destroy_elements(colaReady,free);
	queue_destroy_and_destroy_elements(colaExec,free);
	queue_destroy_and_destroy_elements(colaBlocked,free);
	queue_destroy_and_destroy_elements(colaExit,free);

	list_destroy_and_destroy_elements(lista_programas_actuales, free);

}

t_pcb *sacar_pcb_por_pid(t_list *listaAct, uint32_t pidBuscado)
{
	bool matchPID(void *pcb) {
		return ((t_pcb*)pcb)->pid == pidBuscado;
	}

	return list_remove_by_condition(listaAct, matchPID);
}


void moverA_colaExit(t_pcb *pcb)
{
	queue_push(colaExit, pcb);
	log_debug(logNucleo_estados, "El PCB: %d paso a la cola Exit",pcb->pid);
	//TODO: solicitar borrar Segmentos a la umc
}

void moverA_colaBlocked(t_pcb *pcb)
{
	queue_push(colaBlocked, pcb);
	log_debug(logNucleo_estados, "El PCB: %d paso a la cola Blocked",pcb->pid);
}

void moverA_colaExec(t_pcb *pcb)
{
	queue_push(colaExec, pcb);
	log_debug(logNucleo_estados, "El PCB: %d paso a la cola Exec",pcb->pid);
}

void moverA_colaReady(t_pcb *pcb)
{
	queue_push(colaReady, pcb);
	log_debug(logNucleo_estados, "El PCB: %d paso a la cola Ready",pcb->pid);
}

t_pcb *sacarDe_colaNew(uint32_t pid)
{
	t_pcb *pcb = sacar_pcb_por_pid(colaNew->elements, pid);
	log_debug(logNucleo_estados, "El PCB: %d salio de la cola New");
	return pcb;
}

t_pcb *sacarDe_colaReady(uint32_t pid)
{
	t_pcb *pcb = sacar_pcb_por_pid(colaReady->elements, pid);
	log_debug(logNucleo_estados, "El PCB: %d salio de la cola Ready",pid);
	return pcb;
}

t_pcb *sacarDe_colaExec(uint32_t pid)
{
	t_pcb *pcb = sacar_pcb_por_pid(colaExec->elements, pid);
	log_debug(logNucleo_estados, "Sacando PCB: %d de la cola Exec",pid);
	return pcb;
}

t_pcb *sacarDe_colaBlocked(uint32_t pid)
{
	t_pcb *pcb = sacar_pcb_por_pid(colaBlocked->elements, pid);
	log_debug(logNucleo_estados, "Sacando PCB: %d de la cola Blocked",pid);
	return pcb;
}
