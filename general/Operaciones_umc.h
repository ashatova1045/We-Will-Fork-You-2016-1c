#ifndef OPERACIONES_UMC_H_
#define OPERACIONES_UMC_H_

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

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
	int32_t tamano;
	char* pedido_serializado;
}t_pedido_almacenarBytes_serializado;


t_pedido_almacenarBytes* deserializar_pedido_almacenar(char *pedido_serializado);
t_pedido_almacenarBytes_serializado* serializar_pedido_almacenar(t_pedido_almacenarBytes *pedido);

#endif /* OPERACIONES_UMC_H_ */
