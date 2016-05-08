#ifndef ESTRUCTURASSWAP_H_
#define ESTRUCTURASSWAP_H_

#include <stdbool.h>

typedef struct{
	int puerto_escucha;
	char* nombre_swap;
	int cantidad_paginas;
	int tamanio_pagina;
	int retardo_compactacion;
}t_swapcfg;

typedef struct{
	int PId;
	int nroPagina;
	int posicion;
}t_control_swap;

#endif /* ESTRUCTURAS_SWAP_H_ */
