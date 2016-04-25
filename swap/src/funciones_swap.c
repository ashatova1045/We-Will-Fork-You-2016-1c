#include <commons/config.h>
#include <commons/log.h>
#include <stdbool.h>
#include <stdlib.h>
#include "estructuras_swap.h"
#include "funciones_swap.h"
//#include "../../sockets/Sockets.h"

t_log* logSwap;
t_swapcfg* config_swap;
int socket_memoria;

extern char* prog;
extern int tamanio;

t_log* crearLog(){
	t_log *logSwap = log_create("log.txt", "swap.c", false, LOG_LEVEL_INFO);
	return logSwap;
}

t_swapcfg* levantarConfiguracion(t_config* config){
	t_swapcfg* datosSwap = malloc(sizeof(t_swapcfg));
	if(datosSwap != NULL){

		datosSwap->puerto_escucha = config_get_int_value(config,"PUERTO_ESCUCHA");
		datosSwap->ip_umc = config_get_string_value(config,"IP_UMC");
		datosSwap->nombre_swap = config_get_string_value(config, "NOMBRE_SWAP");
		datosSwap->cantidad_paginas = config_get_int_value(config, "CANTIDAD_PAGINAS");
		datosSwap->tamanio_pagina = config_get_int_value(config, "TAMANIO_PAGINA");
		datosSwap->retardo_compactacion = config_get_int_value(config, "RETARDO_COMPACTACION");
	}
	return datosSwap;
}

void manejar_socket_umc(t_paquete* paquete){
	printf("Socket memoria %d \n",socket_memoria);
	printf("Código Operación %d \n",paquete->cod_op);
	switch(paquete->cod_op){
	case HS_UMC_SWAP:
		enviar(OK_HS_UMC,1,&socket_memoria,socket_memoria);
		printf("Handshake correcto! \n");
		break;
	case NUEVO_PROGRAMA:
		prog = malloc(paquete->tamano_datos);
		tamanio = paquete->tamano_datos;
		strcpy(prog,paquete->datos);
		puts(prog);
		break;
	case 50:
		enviar(50,tamanio,prog,socket_memoria);
		break;
	case ERROR_COD_OP:
		puts("Error en el recibir. Recibio codigo de error");
		log_error(logSwap,"Recibio codigo de error");
		exit(EXIT_FAILURE);
		break;
	default:
		break;
	}
}
