#include "operaciones_swap.h"

t_pedido_almacenar_swap_serializado* serializar_pedido_almacenar_swap(t_pedido_almacenar_swap *pedido,int tamano_pagina){
	int tamano_nroPagina = sizeof(pedido->nroPagina);
	int tamano_pid = sizeof(pedido->pid);

	t_pedido_almacenar_swap_serializado* respuesta = malloc(sizeof(t_pedido_almacenar_swap_serializado));
	respuesta->tamano = tamano_nroPagina+tamano_pid+tamano_pagina*sizeof(char);
	respuesta->pedido_serializado = malloc(respuesta->tamano);

	memcpy(respuesta->pedido_serializado,&pedido->nroPagina,tamano_nroPagina);
	int offset = tamano_nroPagina;

	memcpy(respuesta->pedido_serializado+offset,&pedido->pid,tamano_pid);
	offset += tamano_pid;

	memcpy(respuesta->pedido_serializado+offset,pedido->buffer,tamano_pagina);

	return respuesta;
}

t_pedido_almacenar_swap* deserializar_pedido_almacenar_swap(char *pedido_serializado,int tamano_pagina){
	t_pedido_almacenar_swap *respuesta = malloc(sizeof(t_pedido_almacenar_swap));

	respuesta->nroPagina =*pedido_serializado;
	int offset = sizeof(respuesta->nroPagina);

	respuesta->pid =*(pedido_serializado+offset);
	offset += sizeof(respuesta->pid);

	respuesta->buffer = malloc(tamano_pagina);
	memcpy(respuesta->buffer,pedido_serializado+offset,tamano_pagina);

	return respuesta;
}

