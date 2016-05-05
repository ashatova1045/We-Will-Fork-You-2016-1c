#include "funciones_swap.h"

#include <commons/bitarray.h>
#include <commons/config.h>
#include <commons/log.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../sockets/Sockets.h"
#include "estructuras_swap.h"

t_log* logSwap;
t_swapcfg* config_swap;
int socket_memoria;

extern char* prog;
extern int tamanio;
extern t_bitarray* bitarray;

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
	// Crea estructura bitmap
	char* array = malloc(sizeof(tamanio));
	int cantBytes = (config_swap->cantidad_paginas) / 8;

	bitarray = bitarray_create(array, cantBytes);

	// Crea archivo SWAP
	char command[128];
	size_t tamanio_swap = config_swap->tamanio_pagina * config_swap->cantidad_paginas;

	snprintf(command, sizeof(command), "dd if=/dev/zero of=%s bs=%d count=1",config_swap->nombre_swap,tamanio_swap);
	system(command);
	FILE* swap = fopen("swap.dat","rb+");

	return swap;
}

void manejar_socket_umc(t_paquete* paquete){
	switch(paquete->cod_op){
	// Handshake
	case HS_UMC_SWAP:
		enviar(OK_HS_UMC,100,"Medio panqueque vomitado",socket_memoria);
		printf("Handshake correcto! \n");
		break;
	case ERROR_COD_OP:
		puts("Error en el recibir. Recibio codigo de error");
		log_error(logSwap,"Recibio codigo de error");
		exit(EXIT_FAILURE);
		break;
	default:
		manejarOperaciones(paquete);
		break;
	}
	 /*puts("Proceso recibido");
	 printf("Codigo de operacion: %d\n",paquete->cod_op);
	 printf("Tamano de los datos: %d\n",paquete->tamano_datos);
	 printf("Datos: %s\n",(char*)paquete->datos);*/
}

void manejarOperaciones(t_paquete* paquete){
	switch(paquete->cod_op){
	// Operaciones
	case NUEVO_PROGRAMA:
		inicializarNuevoPrograma(paquete);
		break;
	case LECTURA_PAGINA:
		leerPagina(paquete);
		break;
	case ESCRITURA_PAGINA:
		escribirPagina(paquete);
		break;
	case FINALIZA_PROGRAMA:
		finalizarPrograma(paquete);
		break;
	}
}

void inicializarNuevoPrograma(t_paquete* paquete){
	//todo: Administrar espacio libre - Control de Bitmap
	//todo: Asignar tamaño necesario para el proceso en caso de solicitarse
	//todo: Compactar partición en caso de fragmentación
	prog = malloc(paquete->tamano_datos);
	tamanio = paquete->tamano_datos;
	strcpy(prog,paquete->datos);
	puts(prog);
}

void leerPagina(t_paquete* paquete){
	//todo: Devolver página
	puts("LEER PAGINA");
}

void escribirPagina(t_paquete* paquete){
	//todo: Sobreescribir página
	puts("ESCRIBE PAGINA");
}

void finalizarPrograma(t_paquete* paquete){
	//todo: Liberar espacio en caso que se finalice el proceso
	puts("FINALIZA PROGRAMA");
}
