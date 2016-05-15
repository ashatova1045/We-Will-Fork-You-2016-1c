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
#include "../../sockets/Sockets.c"
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
	if(handshake(socket_nucleo,HS_CPU_NUCLEO,OK_HS_CPU) == -1){
		log_error(logcpu,"Handshake de nucleo incorrecto");
		puts("No se pudo hacer un hansdhake con el nucleo");
		exit(EXIT_FAILURE);
	}
	puts("Handshake de nucleo correcto!");
	log_info(logcpu,"Handshake de nucleo correcto!");

	//TODO recibir al nucleo y si lo que vino tiene codop quantum asignarlo a var global

	if(handshake(socket_umc,HS_CPU_UMC,OK_HS_CPU) == -1){
		log_error(logcpu,"Handshake de umc incorrecto");
		puts("No se pudo hacer un hansdhake con la umc");
		exit(EXIT_FAILURE);
	}
	puts("Handshake de umc correcto!");
	log_info(logcpu,"Handshake de umc correcto!");


//Elimino la configuración que ya no necesito
	config_destroy(conf);
}

void atender_pedido_nucleo(t_paquete* paquete){

	//TODO pedir instrucción a umc


	//TODO parsear


	//TODO responder al nucleo


}

int main() {
	//Creo archivo de log
	logcpu = log_create("logcpu.log", "cpu", false, LOG_LEVEL_DEBUG);
	conectar_cpu();
	sleep(15);
	puts("Pasaron 15");
	enviar(50,1,&socket_umc,socket_umc);
	t_paquete* paq =recibir_paquete(socket_umc);
	puts(paq->datos);


/*	while(true){
		t_paquete* paquete_actual =recibir_paquete(socket_nucleo);
		if(paquete_actual->cod_op == CORRER_PCB){
			pthread_t hilo_ejecucion;
			pthread_create(&hilo_ejecucion,NULL,(void *) atender_pedido_nucleo,paquete_actual);
			log_info(logcpu,"Se crea hilo para atender el pedido de correr un PCB");
		}
		destruir_paquete(paquete_actual);
	}
*/

	bool se_cerro_nucleo = false;
	pthread_t hilo_ejecucion;
	//Mientras no se cierre la conexion con el nucleo
	while(!se_cerro_nucleo){

		//Recibo el paquete
		t_paquete* paquete_actual =recibir_paquete(socket_nucleo);
		log_info(logcpu,"Se recibio un pedido del socket %d",socket_nucleo);

		//Trato el paquete segun el codigo de operacion
		switch(paquete_actual->cod_op){
			case CORRER_PCB:
				pthread_create(&hilo_ejecucion,NULL,(void *) atender_pedido_nucleo,paquete_actual);
				log_info(logcpu,"Se crea hilo para atender el pedido de correr un PCB");
				break;
			case FINALIZA_PROGRAMA:
				//TODO Finalizar el programa en ejecución
				break;
			//En caso de que el paquete recibido sea de la umc y no del nucleo
			default:
				se_cerro_nucleo=true;
				break;
		}
		destruir_paquete(paquete_actual);
	}

	bool se_cerro_umc = false;
	while(!se_cerro_umc){

		//Recibo el paquete
		t_paquete* paquete_actual =recibir_paquete(socket_umc);
		log_info(logcpu,"Se recibio un pedido del socket %d",socket_umc);

		//Trato el paquete segun el codigo de operacion
		switch(paquete_actual->cod_op){
			case BUFFER_LEIDO:
				//TODO Se leyo una pagina
				break;
			case OK:
				//Respuesta a un pedido
				break;
			case NO_OK:
				//Respuesta a un pedido
				break;
			default:
				//En caso de que el paquete recibido sea del nucleo y no se la umc
				se_cerro_umc=true;
				break;
		}
		destruir_paquete(paquete_actual);
	}


	log_destroy(logcpu);
	log_info(logcpu, "Termino el proceso CPU");
	return (EXIT_SUCCESS);
}
