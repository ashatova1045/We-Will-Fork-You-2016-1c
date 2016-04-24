/*
 ============================================================================
 Name        : abrir.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include "../../sockets/Sockets.h"

int socket_umc,socket_nucleo;
//Defino la función para leer la configuración

void conectar_cpu() {
	t_config* conf = config_create("cpu.cfg");
	if (!conf) {
		//      log_error(logCpu,"Error al abrir la configuracion");
		puts("Fallo config");
			  exit(EXIT_FAILURE);
	}

	socket_nucleo = conectar(config_get_string_value(conf, "IP_NUCLEO"),
			config_get_int_value(conf, "PUERTO_NUCLEO"));
	if (socket_nucleo == -1) {
		//  log_error(logCpu,"Error al conectarse al server! %s\n IP: %s PUERTO: %d",strerror(errno),config_get_string_value(conf,"IP_KERNEL"),config_get_int_value(conf,"PUERTO_KERNEL"));
		//  exit(EXIT_FAILURE);
	}

	socket_umc = conectar(config_get_string_value(conf, "IP_UMC"),
								config_get_int_value(conf, "PUERTO_UMC"));
	if (socket_umc == -1) {
		//  log_error(logCpu,"Error al conectarse al server! %s\n IP: %s PUERTO: %d",strerror(errno),config_get_string_value(conf,"IP_KERNEL"),config_get_int_value(conf,"PUERTO_KERNEL"));
		//  exit(EXIT_FAILURE);
	}

//Elimino la configuración que ya no necesito
	config_destroy(conf);
}

int main() {
	conectar_cpu();
	return (EXIT_SUCCESS);
}
