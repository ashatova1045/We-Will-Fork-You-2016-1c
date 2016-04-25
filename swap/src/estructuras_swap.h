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

#endif /* ESTRUCTURAS_SWAP_H_ */
