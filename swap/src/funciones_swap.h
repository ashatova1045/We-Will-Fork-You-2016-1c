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

#endif /* FUNCIONES_SWAP_H_ */
