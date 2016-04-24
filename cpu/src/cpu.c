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
#include <errno.h>
#include <commons/collections/list.h>
#include "../../sockets/Sockets.h"
#include <pthread.h>

int socket_umc,socket_nucleo;
t_log* logcpu;
//Defino la función para leer la configuración

void conectar_cpu() {
	t_config* conf = config_create("../cpu/cpu.cfg");
	if (!conf) {
		log_error(logcpu,"Error al abrir la configuracion");
		puts("Fallo config");
			  exit(EXIT_FAILURE);
	}

	socket_nucleo = conectar(config_get_string_value(conf, "IP_NUCLEO"),
			config_get_int_value(conf, "PUERTO_NUCLEO"));
	if (socket_nucleo == -1) {
		log_error(logcpu,"Error al conectarse al nucleo!");
		  exit(EXIT_FAILURE);
	}

	socket_umc = conectar(config_get_string_value(conf, "IP_UMC"),
								config_get_int_value(conf, "PUERTO_UMC"));
	if (socket_umc == -1) {
		log_error(logcpu,"Error al conectarse a la umc!");
		  exit(EXIT_FAILURE);
	}

//Elimino la configuración que ya no necesito
	config_destroy(conf);
}

void atender_pedido_nucleo(t_paquete* paquete){
	//todo pedir codigo a umc
	//todo parsear
	//todo responder al nucleo
	enviar(CORRER_PCB,1,&socket_umc,socket_umc);
}

int main() {
	//Creo archivo de log
	logcpu = log_create("logcpu.log", "cpu", false, LOG_LEVEL_DEBUG);
	conectar_cpu();

	while(true){
		t_paquete* paquete_actual =recibir_paquete(socket_nucleo);
		if(paquete_actual->cod_op == CORRER_PCB){
			pthread_t hilo_ejecucion;
			pthread_create(&hilo_ejecucion,NULL,(void *) atender_pedido_nucleo,paquete_actual);
			log_info(logcpu,"Se crea hilo para atender el pedido de correr un PCB");
		}
		destruir_paquete(paquete_actual);
	}

	log_destroy(logcpu);
	log_info(logcpu, "Termino el proceso CPU");
	return (EXIT_SUCCESS);
}
