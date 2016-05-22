#include "Operaciones_umc.h"

t_pedido_almacenarBytes_serializado* serializar_pedido_almacenar(t_pedido_almacenarBytes *pedido){
	int tamano_nroPagina = sizeof(pedido->nroPagina);
	int tamano_offset = sizeof(pedido->offset);
	int tamano_tamanioDatos = sizeof(pedido->tamanioDatos);

	t_pedido_almacenarBytes_serializado* respuesta = malloc(sizeof(t_pedido_almacenarBytes_serializado));
	respuesta->tamano = tamano_nroPagina+tamano_offset+tamano_tamanioDatos+pedido->tamanioDatos * sizeof(char);
	respuesta->pedido_serializado = malloc(respuesta->tamano);

	memcpy(respuesta->pedido_serializado,&pedido->nroPagina,tamano_nroPagina);
	int offset = tamano_nroPagina;

	memcpy(respuesta->pedido_serializado+offset,&pedido->offset,tamano_offset);
	offset += tamano_offset;

	memcpy(respuesta->pedido_serializado+offset,&pedido->tamanioDatos,tamano_tamanioDatos);
	offset += tamano_offset;

	memcpy(respuesta->pedido_serializado+offset,pedido->buffer,pedido->tamanioDatos);

	return respuesta;
}

t_pedido_almacenarBytes* deserializar_pedido_almacenar(char *pedido_serializado){
	t_pedido_almacenarBytes *respuesta = malloc(sizeof(t_pedido_almacenarBytes));

	respuesta->nroPagina =*pedido_serializado;
	int offset = sizeof(respuesta->nroPagina);

	respuesta->offset =*(pedido_serializado+offset);
	offset += sizeof(respuesta->offset);

	respuesta->tamanioDatos =*(pedido_serializado+offset);
	offset += sizeof(respuesta->tamanioDatos);

	respuesta->buffer = malloc(respuesta->tamanioDatos);
	memcpy(respuesta->buffer,pedido_serializado+offset,respuesta->tamanioDatos);

	return respuesta;
}

t_pedido_inicializar* deserializar_pedido_inicializar(char *pedido_serializado){
	t_pedido_inicializar *respuesta = malloc(sizeof(t_pedido_inicializar));

	int offset = 0;
	respuesta->idPrograma = *((int32_t*)pedido_serializado);
	offset += sizeof(respuesta->idPrograma);
	respuesta->pagRequeridas = *((int32_t*)(pedido_serializado+offset));
	offset += sizeof(respuesta->pagRequeridas);
	respuesta->codigo = strdup(pedido_serializado+offset);
	
	return respuesta;
}
t_pedido_inicializar_serializado* serializar_pedido_inicializar(t_pedido_inicializar *pedido){
	t_pedido_inicializar_serializado *respuesta = malloc(sizeof(t_pedido_inicializar_serializado));

	int tamanoidprograma = sizeof(pedido->idPrograma);
	int tamanopagsrequeridas = sizeof(pedido->pagRequeridas);
	int tamanocodigo = (strlen(pedido->codigo)+1);

	respuesta->tamano = tamanoidprograma+ tamanopagsrequeridas + tamanocodigo;
	respuesta->pedido_serializado = malloc(respuesta->tamano);

	int offset = 0;
	memcpy(respuesta->pedido_serializado,&pedido->idPrograma,tamanoidprograma);
	offset += tamanoidprograma;

	memcpy(respuesta->pedido_serializado+offset,&pedido->pagRequeridas,tamanopagsrequeridas);
	offset += tamanopagsrequeridas;

	strcpy(respuesta->pedido_serializado+offset,pedido->codigo);

	return respuesta;
}
