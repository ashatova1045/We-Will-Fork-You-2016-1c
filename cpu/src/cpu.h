#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/log.h>
#include <errno.h>
#include <commons/collections/list.h>
#include "../../sockets/Sockets.h"
#include "../../general/pcb.h"
#include "../../general/Operaciones_umc.h"
#include <pthread.h>
#include <parser/parser.h>
#include "primitivas.h"

int socket_umc, socket_nucleo;
int32_t quantumCpu,retardo, tamPag;
t_pcb* pcb_ejecutandose;
bool termino_programa;
t_log* logcpu;
bool llego_sugus;

char* pedir_lectura_de_umc(t_pedido_solicitarBytes pedido);

#endif /* CPU_H_ */
