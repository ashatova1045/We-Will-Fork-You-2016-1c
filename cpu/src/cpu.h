#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
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
int32_t quantumCpu, tamPag;
t_pcb* pcb_ejecutandose;
bool termino_programa;
t_log* logcpu;

#endif /* CPU_H_ */
