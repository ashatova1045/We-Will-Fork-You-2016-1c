/*
 * estructurasSwap.h
 *
 *  Created on: 22/4/2016
 *      Author: utnso
 */

#ifndef ESTRUCTURASSWAP_H_
#define ESTRUCTURASSWAP_H_

typedef struct{
	int puerto_escucha;
	int ip_umc;
	char* nombre_swap;
	int cantidad_paginas;
	int tamanio_pagina;
	int retardo_compactacion;
}t_swapcfg;

#endif /* ESTRUCTURAS_SWAP_H_ */
