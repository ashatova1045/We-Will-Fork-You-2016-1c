#ifndef FUNCIONES_SWAP_H_
#define FUNCIONES_SWAP_H_

#include "estructuras_swap.h"

#include <stdint.h>

#include "../../general/Operaciones_umc.h"
#include "../../general/operaciones_swap.h"

#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "../../sockets/Sockets.h"

t_log* crearLog();
t_swapcfg* levantarConfiguracion(t_config*);
void manejar_socket_umc(t_paquete*);
void inicializaSwapFile(t_swapcfg*);
void manejarOperaciones(t_paquete* paquete);
void inicializarNuevoPrograma(t_paquete* paquete);
void leerPagina(t_paquete* paquete);
void escribirPagina(t_paquete* paquete);
void finalizarPrograma(t_paquete* paquete);
int verificarPaginasLibres(int cantidadPaginas);
int encontrar_espacio(int cantidadPaginas);
void agregarNuevoProceso(int posicion,int cantidadPaginas,t_pedido_inicializar_swap* pedido);

t_bitarray* bitarray;
FILE* swapFile;
t_log* logSwap;
t_swapcfg* config_swap;
int tamanioPagina;

#endif /* FUNCIONES_SWAP_H_ */
