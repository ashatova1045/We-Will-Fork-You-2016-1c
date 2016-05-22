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

t_pedido_inicializar_swap* deserializar_pedido_inicializar_swap(char *pedido_serializado){
	t_pedido_inicializar_swap *respuesta= malloc(sizeof(t_pedido_inicializar_swap));

	int offset=0;
	respuesta->idPrograma= *pedido_serializado;
	offset+=sizeof(respuesta->idPrograma);
	respuesta->pagRequeridas=*(pedido_serializado+offset);
	offset+=sizeof(respuesta->pagRequeridas);
	respuesta->codigo = strdup(pedido_serializado+offset);

	return respuesta;
}

t_pedido_inicializar_serializado_swap* serializar_pedido_inicializar_swap(t_pedido_inicializar_swap *pedido){
	t_pedido_inicializar_serializado_swap *respuesta=malloc(sizeof(t_pedido_inicializar_serializado_swap));

	int tamano_idprograma = sizeof(pedido->idPrograma);
	int tamano_pags_requeridas = sizeof(pedido->pagRequeridas);
	int tamano_codigo = (strlen(pedido->codigo)+1);

	respuesta->tamano=tamano_idprograma + tamano_pags_requeridas + tamano_codigo;
	respuesta->pedido_serializado = malloc(respuesta->tamano);

	int offset=0;
	memcpy(respuesta->pedido_serializado,&pedido->idPrograma,tamano_idprograma);
	offset += tamano_idprograma;

	memcpy(respuesta->pedido_serializado+offset, &pedido->pagRequeridas, tamano_pags_requeridas);
	offset += tamano_idprograma;

	strcpy(respuesta->pedido_serializado+offset,pedido->codigo);

	return respuesta;
}
