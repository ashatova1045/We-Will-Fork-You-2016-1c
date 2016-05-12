#ifndef OPERACIONES_SWAP_H_
#define OPERACIONES_SWAP_H_

#include <sys/types.h>

typedef struct
{
	int32_t idPrograma;
	int32_t pagRequeridas;
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
	int32_t idPrograma;
}t_pedido_finalizar_swap;

#endif /* OPERACIONES_SWAP_H_ */
