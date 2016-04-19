#include <commons/log.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <commons/config.h>

#include "../../sockets/Sockets.h"

int responderUMC(){
	int socket = conectar("192.168.43.57",4200);
	if (socket == -1)
		exit(EXIT_FAILURE);

	char* mensaje = malloc(100*sizeof(char));
	for(;;){
		mensaje = "respuestaSwap";

		int msj_len = strlen(mensaje);

		if (mensaje==NULL){
			perror("Error al alocar memoria.");
			return EXIT_FAILURE;
		}

		enviar(1,msj_len,mensaje,socket); //como solo estoy mandando un string lo mando con el caracter de escape incluido
		puts("Mensaje enviado");

	}
	free(mensaje);

	close(socket);
	return EXIT_SUCCESS;
}

t_log* crearLog(){
	t_log *logSwap = log_create("log.txt", "swap.c", false, LOG_LEVEL_INFO);
	return logSwap;
}

typedef struct{
	int puerto_escucha;
	char* nombre_swap;
	int cantidad_paginas;
	int tamanio_pagina;
	int retardo_compactacion;
}t_swapcfg;

t_swapcfg* levantarConfiguracion(){
	t_swapcfg* datosSwap = (t_swapcfg*)malloc(sizeof(t_swapcfg));
	t_config* config = config_create("../swap/swap.cfg");
	datosSwap->puerto_escucha = config_get_int_value(config,"PUERTO_ESCUCHA");
	datosSwap->nombre_swap = config_get_string_value(config, "NOMBRE_SWAP");
	datosSwap->cantidad_paginas = config_get_int_value(config, "CANTIDAD_PAGINAS");
	datosSwap->tamanio_pagina = config_get_int_value(config, "TAMANIO_PAGINA");
	datosSwap->retardo_compactacion = config_get_int_value(config, "RETARDO_COMPACTACION");
	return datosSwap;
}

int main(int argc, char** argv) {

	// Crea archivo de log
	t_log* logSwap = crearLog();
	log_info(logSwap, "Ejecución del proceso SWAP");

	// Levanta la configuración del Swap
	t_swapcfg* config_swap = levantarConfiguracion();
	printf("Se conecta al puerto %d \n",config_swap->puerto_escucha);

	// Inicializa la conexión y queda a espera de procesos UMC
	int socketserver = crear_socket_escucha(4100);
	if (socketserver == -1)
		exit(EXIT_FAILURE);

	puts("Esperando procesos UMC...");

	// Acepta la conexón de un proceso UMC
	int socket_conectado =  aceptar_cliente(socketserver);

	if(socket_conectado == -1)
		exit(EXIT_FAILURE);

	printf("Se conectó el UMC con socket: %d \n",socket_conectado);

	t_paquete* paquete;

	// Recibe la petición
	paquete = recibir_paquete(socket_conectado);
	if(paquete->cod_op == ERROR_COD_OP)
		exit(EXIT_FAILURE);

	//uint16_t cod_op = paquete->cod_op;
 	//char *datos = paquete->datos;
	puts("Proceso recibido");
	printf("Codigo de operacion: %d\n",paquete->cod_op);
	printf("Tamano de los datos: %d\n",paquete->tamano_datos);
	printf("Datos: %s\n",(char*)paquete->datos);

	//todo: Asignar tamaño necesario para el proceso en caso de solicitarse
	//todo: Compactar partición en caso de fragmentación

	//todo: Devolver página / Sobreescribir página

	//todo: Administrar espacio libre - Control de Bitmap

	//todo: Liberar espacio en caso que se finalice el proceso

//	responderUMC();

	destruir_paquete(paquete);

	close(socketserver);
	close (socket_conectado);
	return EXIT_SUCCESS;
}
