/*
 * funciones_swap.h
 *
 *  Created on: 22/4/2016
 *      Author: utnso
 */

#ifndef FUNCIONES_SWAP_H_
#define FUNCIONES_SWAP_H_

t_log* crearLog(){
	t_log *logSwap = log_create("log.txt", "swap.c", false, LOG_LEVEL_INFO);
	return logSwap;
}

t_swapcfg* levantarConfiguracion(){
	t_swapcfg* datosSwap = malloc(sizeof(t_swapcfg));
	if(datosSwap != NULL){
		t_config* config = config_create("../swap/swap.cfg");
		datosSwap->puerto_escucha = config_get_int_value(config,"PUERTO_ESCUCHA");
		datosSwap->ip_umc = config_get_string_value(config,"IP_UMC");
		datosSwap->nombre_swap = config_get_string_value(config, "NOMBRE_SWAP");
		datosSwap->cantidad_paginas = config_get_int_value(config, "CANTIDAD_PAGINAS");
		datosSwap->tamanio_pagina = config_get_int_value(config, "TAMANIO_PAGINA");
		datosSwap->retardo_compactacion = config_get_int_value(config, "RETARDO_COMPACTACION");
		config_destroy(config);
	}
	return datosSwap;
}

int responderUMC(int *socket_memoria, t_swapcfg *config_swap){
	printf("Socket memoria: %d", *socket_memoria);
	int socket = conectar(config_swap->ip_umc,config_swap->puerto_escucha);
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

#endif /* FUNCIONES_SWAP_H_ */
