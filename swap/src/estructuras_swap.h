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
	char* ip_umc;
	char* nombre_swap;
	int cantidad_paginas;
	int tamanio_pagina;
	int retardo_compactacion;
}t_swapcfg;

typedef struct{
	char tipo_operacion[1];
	char* datos;
}t_prot_swap_umc;

typedef struct{
	int processId;
	char tipo_operacion[1];
	char* datos;
}t_prot_umc_swap;

#endif /* ESTRUCTURAS_SWAP_H_ */
