#ifndef OPERACIONES_SWAP_H_
#define OPERACIONES_SWAP_H_

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

typedef struct
{
	int32_t idPrograma;
	int32_t pagRequeridas;
	char* codigo;
}t_pedido_inicializar_swap;

typedef struct
{
	int32_t pid;
	int32_t nroPagina;
}t_pedido_leer_swap;

typedef struct
{
	int32_t nroPagina;
	int32_t pid;
	char* buffer;
}t_pedido_almacenar_swap;

typedef struct
{
	int32_t tamano;
	char* pedido_serializado;
}t_pedido_almacenar_swap_serializado;

typedef struct
{
	int32_t tamano;
	char* pedido_serializado;
}t_pedido_inicializar_serializado_swap;

t_pedido_almacenar_swap_serializado* serializar_pedido_almacenar_swap(t_pedido_almacenar_swap *pedido,int tamano_pagina);
t_pedido_almacenar_swap* deserializar_pedido_almacenar_swap(char *pedido_serializado,int tamano_pagina);
t_pedido_inicializar_swap* deserializar_pedido_inicializar_swap(char *pedido_serializado);
t_pedido_inicializar_serializado_swap* serializar_pedido_inicializar_swap(t_pedido_inicializar_swap *pedido);


#endif /* OPERACIONES_SWAP_H_ */
