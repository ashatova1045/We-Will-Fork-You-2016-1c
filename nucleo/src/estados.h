/*
 * estados.h
 *
 *  Created on: 6/5/2016
 *      Author: utnso
 */

#ifndef ESTADOS_H_
#define ESTADOS_H_

#include <commons/collections/queue.h>
#include "../../general/pcb.h"
#include <commons/log.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/* link referencia http://stackoverflow.com/questions/1433204/how-do-i-use-extern-to-share-variables-between-source-files-in-c
 *
 * Usar las variables como extern te permite decir que existe un tipo de dato o variable
 *  para que lo uses pero que este declarado en algun otro lado fuera de tu codigo*/

extern t_queue *colaNew;
extern t_queue *colaReady;
extern t_queue *colaExec;
extern t_queue *colaBlocked;
extern t_queue *colaExit;

void crear_colas();
void destruir_colas();

t_pcb *sacar_pcb_por_pid(t_list *listaAct, uint32_t pidBuscado);

void moverA_colaExit(t_pcb *pcb);
void moverA_colaBlocked(t_pcb *pcb);
void moverA_colaExec(t_pcb *pcb);
void moverA_colaReady(t_pcb *pcb);

t_pcb *sacarDe_colaNew(uint32_t pid);
t_pcb *sacarDe_colaReady(uint32_t pid);
t_pcb *sacarDe_colaExec(uint32_t pid);
t_pcb *sacarDe_colaBlocked(uint32_t pid);

#endif /* ESTADOS_H_ */
