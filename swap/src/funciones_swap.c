#include "funciones_swap.h"

#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../sockets/Sockets.h"
#include "estructuras_swap.h"

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
		datosSwap->nombre_swap = config_get_string_value(config, "NOMBRE_SWAP");
		datosSwap->cantidad_paginas = config_get_int_value(config, "CANTIDAD_PAGINAS");
		datosSwap->tamanio_pagina = config_get_int_value(config, "TAMANIO_PAGINA");
		datosSwap->retardo_compactacion = config_get_int_value(config, "RETARDO_COMPACTACION");
	}
	return datosSwap;
}

FILE* inicializaSwapFile(t_swapcfg* config_swap){
	FILE* swap = fopen(config_swap->nombre_swap, "ab+");
	size_t tamanio_swap = config_swap->tamanio_pagina * config_swap->cantidad_paginas;

	//t_bitarray* arrayBits = bitarray_create("arrayBits", tamanio_swap);

	int i;
	char cero = '\0';
	for (i = 0 ; i < tamanio_swap; i++){
		fwrite(&cero, sizeof(char), 1, swap);
		fseek(swap, 0, SEEK_SET);
	}
	return swap;

}

void manejar_socket_umc(t_paquete* paquete){
	printf("Socket memoria %d \n",socket_memoria);
	printf("Código Operación %d \n",paquete->cod_op);
	t_control_swap* elementoInicial = malloc(sizeof(t_control_swap*));
	switch(paquete->cod_op){
	// Verificaciones
	case HS_UMC_SWAP:
		enviar(OK_HS_UMC,1,&socket_memoria,socket_memoria);
		printf("Handshake correcto! \n");
		break;
	case ERROR_COD_OP:
		puts("Error en el recibir. Recibio codigo de error");
		log_error(logSwap,"Recibio codigo de error");
		exit(EXIT_FAILURE);
		break;
	// Operaciones
	case NUEVO_PROGRAMA:
		prog = malloc(paquete->tamano_datos);
		tamanio = paquete->tamano_datos;
		strcpy(prog,paquete->datos);
		puts(prog);

		/*int estadoSwap = controlEspacioSwap(cantPaginas);
		switch(estadoSwap){
			case 0:
				compactarSwap();
				cargarPrograma();
				break;
			case 1:
				cargarPrograma();
				break;
			default:
				log_error(logSwap,"No hay espacio disponible");
				break;
		}*/

		break;
	case 50:
		enviar(50,tamanio,prog,socket_memoria);
		break;
	default:
		break;
	}

}

void cargarPrograma(int PID, int cantPaginas){
	int i;
	for(i=0;i<cantPaginas;i++){
		//int posicion = obtenerPosicion();
		//t_pag_swap* pagina = setearPagina();
		//cargarPagina(swapFile, pagina);
	}
}

//todo: Administrar espacio libre - Control de Bitmap

//todo: Asignar tamaño necesario para el proceso en caso de solicitarse

//todo: Compactar partición en caso de fragmentación

//todo: Devolver página / Sobreescribir página

//todo: Liberar espacio en caso que se finalice el proceso
