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
#include "../../general/pcb.h"

#include "../../general/Operaciones_umc.h"
#include <pthread.h>

int socket_umc, socket_nucleo;
 int32_t quantumCpu, tamPag;


t_log* logcpu;
//Defino la función para leer la configuración

void conectar_cpu() {
	t_config* conf = config_create("../cpu/cpu.cfg");
	if (!conf) {
		log_error(logcpu, "Error al abrir la configuracion");
		puts("Fallo config");
		exit(EXIT_FAILURE);
	}

	socket_nucleo = conectar(config_get_string_value(conf, "IP_NUCLEO"),
			config_get_int_value(conf, "PUERTO_NUCLEO"));
	if (socket_nucleo == -1) {
		log_error(logcpu, "Error al conectarse al nucleo!");
		exit(EXIT_FAILURE);
	}

	socket_umc = conectar(config_get_string_value(conf, "IP_UMC"),
			config_get_int_value(conf, "PUERTO_UMC"));
	if (socket_umc == -1) {
		log_error(logcpu, "Error al conectarse a la umc!");
		exit(EXIT_FAILURE);
	}
	if (handshake(socket_nucleo, HS_CPU_NUCLEO, OK_HS_CPU) == -1) {
		log_error(logcpu, "Handshake de nucleo incorrecto");
		puts("No se pudo hacer un hansdhake con el nucleo");
		exit(EXIT_FAILURE);
	}
	puts("Handshake de nucleo correcto!");
	log_info(logcpu, "Handshake de nucleo correcto!");

	t_paquete *paqNucleo = recibir_paquete(socket_nucleo);
	quantumCpu = (*(int32_t*)paqNucleo->datos);

	log_info(logcpu, "Quantum del CPU = : %d", quantumCpu);

	destruir_paquete(paqNucleo);

	if(handshake(socket_umc,HS_CPU_UMC,OK_HS_CPU) == -1){
		log_error(logcpu,"Handshake de umc incorrecto");
		puts("No se pudo hacer un hansdhake con la umc");
		exit(EXIT_FAILURE);
	}
	puts("Handshake de umc correcto!");
	log_info(logcpu, "Handshake de umc correcto!");

	t_paquete *paqUmc = recibir_paquete(socket_umc);
	tamPag = (*(int32_t*)paqUmc->datos);
	log_info(logcpu, "Tamanio de pagina: %d", tamPag);

	destruir_paquete(paqUmc);

//Elimino la configuración que ya no necesito
	config_destroy(conf);
}

void recibir_umc() {
	//Recibo el paquete
	t_paquete* paquete_actual = recibir_paquete(socket_umc);
	log_info(logcpu, "Se recibio un mensaje de la UMC. cod op %d",paquete_actual->cod_op);
	//Trato el paquete segun el codigo de operacion
	switch (paquete_actual->cod_op) {
		case BUFFER_LEIDO:
			log_info(logcpu, "Datos leidos: %s", paquete_actual->datos); //FixMe
			break;
		case OK:
			//Respuesta a un pedido
			break;
		case NO_OK:
			log_info(logcpu,"NO OK recibido");
			//Respuesta a un pedido
			break;
		default:
			//En caso de que el paquete recibido sea del nucleo y no se la umc
			break;
	}
	destruir_paquete(paquete_actual);
}


void atender_pedido_nucleo(t_paquete* paquete) {

	t_pcb* pcb = deserializar(paquete->datos);
	enviar(CAMBIO_PROCESO_ACTIVO,sizeof(pcb->pid),&pcb->pid,socket_umc);
	log_debug(logcpu,"Se envio el cambio de proceso activo %d",pcb->pid);

	t_posMemoria instruccion_actual = pcb->indice_codigo[pcb->pc];

	//Pido instrucción a umc

	t_pedido_solicitarBytes pedido;
	pedido.nroPagina = instruccion_actual.pag;
	pedido.offset = instruccion_actual.offset;
	pedido.tamanioDatos = instruccion_actual.size;

	//Envío pedido a UMC
	enviar(LECTURA_PAGINA, sizeof(pedido), &pedido, socket_umc);

	log_info(logcpu, "Pedido enviado a UMC");
	log_info(logcpu, "Pagina: %d", pedido.nroPagina);
	log_info(logcpu, "Offset: %d", pedido.offset);
	log_info(logcpu, "Tamanio: %d", pedido.tamanioDatos);

	recibir_umc();
	//TODO parsear


	//TODO responder al nucleo
	destruir_paquete(paquete);
}



int main() {

	//Creo archivo de log
	logcpu = log_create("logcpu.log", "cpu", false, LOG_LEVEL_DEBUG);
	conectar_cpu();
	log_trace(logcpu,"Termino la funcion conectar_cpu()");
	bool se_cerro_nucleo = false;
	pthread_t hilo_ejecucion;
	//Mientras no se cierre la conexion con el nucleo
	while(!se_cerro_nucleo){

		//Recibo el paquete
		t_paquete* paquete_actual =recibir_paquete(socket_nucleo);
		log_info(logcpu,"Se recibio un pedido del nucleo. Cod op: %d",paquete_actual->cod_op);

		//Trato el paquete segun el codigo de operacion
		switch(paquete_actual->cod_op){
			case CORRER_PCB:
				pthread_create(&hilo_ejecucion,NULL,(void *) atender_pedido_nucleo,paquete_actual);
				log_info(logcpu,"Se crea hilo para atender el pedido de correr un PCB");
				break;
			case FINALIZA_PROGRAMA:
				//TODO Finalizar el programa en ejecución

				destruir_paquete(paquete_actual);
				break;
			//En caso de que el paquete recibido sea de la umc y no del nucleo
			case ERROR_COD_OP:
				se_cerro_nucleo=true;
				break;
				destruir_paquete(paquete_actual);
			default:
				break;
		}
	}

		//Recibo el paquete
	log_destroy(logcpu);
	log_info(logcpu, "Termino el proceso CPU");
	return (EXIT_SUCCESS);
}
