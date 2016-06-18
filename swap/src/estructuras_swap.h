#ifndef ESTRUCTURASSWAP_H_
#define ESTRUCTURASSWAP_H_

#include <stdbool.h>
#include <commons/config.h>
#include <commons/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../../sockets/Sockets.h"

#include <commons/collections/list.h>
#include <commons/bitarray.h>

#include <stdint.h>

#include "../../general/Operaciones_umc.h"
#include "../../general/operaciones_swap.h"

#include <stddef.h>
#include <sys/types.h>


typedef struct{
	int puerto_escucha;
	char* nombre_swap;
	int cantidad_paginas;
	int tamanio_pagina;
	int retardo_acceso;
	int retardo_compactacion;
}t_swapcfg;

typedef struct{
	int PId;
	int cantPaginas;
	int posicion;
}t_control_swap;

t_bitarray* bitarray;
FILE* swapFile;
int tamanioPagina;
int primerPosicionVacia;

int socket_memoria;
t_swapcfg* datosSwap;
t_log* logSwap;
t_list* lista_procesos;

#endif /* ESTRUCTURAS_SWAP_H_ */
