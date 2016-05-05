#ifndef FUNCIONES_SWAP_H_
#define FUNCIONES_SWAP_H_

#include "estructuras_swap.h"
#include <commons/log.h>
#include <commons/config.h>
#include <stdint.h>
#include "../../sockets/Sockets.h"

t_log* crearLog();
t_swapcfg* levantarConfiguracion(t_config*);
void manejar_socket_umc(t_paquete*);
FILE* inicializaSwapFile(t_swapcfg*);
void manejarOperaciones(t_paquete* paquete);
void inicializarNuevoPrograma(t_paquete* paquete);
void leerPagina(t_paquete* paquete);
void escribirPagina(t_paquete* paquete);
void finalizarPrograma(t_paquete* paquete);
#endif /* FUNCIONES_SWAP_H_ */
