#ifndef OPERACIONES_UMC_H_
#define OPERACIONES_UMC_H_

#include <sys/types.h>

typedef struct
{
	int32_t idPrograma;
	int32_t pagRequeridas;
}t_pedido_inicializar;

typedef struct
{
	int32_t nroPagina;
	int32_t offset;
	int32_t tamanioDatos;
}t_pedido_solicitarBytes;

typedef struct
{
	int32_t nroPagina;
	int32_t offset;
	int32_t tamanioDatos;
	char* buffer;
}t_pedido_almacenarBytes;

typedef struct
{
	int32_t idPrograma;
}t_pedido_finalizar;



#endif /* OPERACIONES_UMC_H_ */
