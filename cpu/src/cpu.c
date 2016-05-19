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
#include "../../general/pcb.h"
#include "../../general/Operaciones_umc.h"
#include <pthread.h>
#include <parser/parser.h>
#include "primitivas.h"

int socket_umc, socket_nucleo;
int32_t quantumCpu, tamPag;
pthread_t hilo_ejecucion;
t_pcb* pcb_ejecutandose;

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

char* recibir_respuesta_lectura() {
	//Recibo el paquete
	t_paquete* paquete_actual = recibir_paquete(socket_umc);
	log_trace(logcpu, "Se recibio un mensaje de respuesta de lectura de la UMC. cod op %d",paquete_actual->cod_op);
	//Trato el paquete segun el codigo de operacion
	switch (paquete_actual->cod_op) {
		case BUFFER_LEIDO:
			log_debug(logcpu, "Datos leidos: %s", paquete_actual->datos);
			break;
		case NO_OK:
			log_info(logcpu,"NO OK recibido");
			//Respuesta a un pedido
			break;
		default:
			puts("Se recibio un codigo de error o desconocido de la UMC");
			log_error(logcpu,"Se recibio un codigo de error o desconocido de la UMC");
			destruir_paquete(paquete_actual);
			exit(EXIT_FAILURE);
			//En caso de que el paquete recibido sea del nucleo y no se la umc
			break;
	}
	char* respuesta = strdup(paquete_actual->datos);
	destruir_paquete(paquete_actual);
	if(respuesta == NULL){
		puts("Error al reservar memoria para la lectura requerida");
		log_error(logcpu,"Error al reservar memoria para la lectura requerida");
		pthread_exit(NULL);
	}

	return respuesta;
}

char* pedir_lectura_de_umc(t_pedido_solicitarBytes pedido) {
	//Envío pedido a UMC
	enviar(LECTURA_PAGINA, sizeof(pedido), &pedido, socket_umc);
	log_info(logcpu, "Pedido enviado a UMC");
	log_info(logcpu, "Pagina: %d", pedido.nroPagina);
	log_info(logcpu, "Offset: %d", pedido.offset);
	log_info(logcpu, "Tamanio: %d", pedido.tamanioDatos);

	return recibir_respuesta_lectura();
}

void correr_pcb() {
	//Seteo proceso activo
	enviar(CAMBIO_PROCESO_ACTIVO,sizeof(pcb_ejecutandose->pid),&pcb_ejecutandose->pid,socket_umc);
	log_debug(logcpu,"Se envio el cambio de proceso activo %d",pcb_ejecutandose->pid);





	int quantum_actual;
	for(quantum_actual = 1;quantum_actual<=quantumCpu;quantum_actual++){
		log_info(logcpu,"\n\n\n\nCorriendo instruccion %d de %d",quantum_actual,quantumCpu);
		t_posMemoria posicion_instruccion_actual = pcb_ejecutandose->indice_codigo[pcb_ejecutandose->pc];

		//Pido instrucción a umc

		t_pedido_solicitarBytes pedido;
		pedido.nroPagina = posicion_instruccion_actual.pag;
		pedido.offset = posicion_instruccion_actual.offset;
		pedido.tamanioDatos = posicion_instruccion_actual.size;

		//Envío pedido a UMC
		char* instruccion_actual = pedir_lectura_de_umc(pedido);

		//Parseo de la instruccion
//		analizadorLinea(instruccion_actual,&functions,&kernel_functions);

		//Incremento el program counter
		pcb_ejecutandose->pc++;

		//Destuyo la instruccion
		free(instruccion_actual);
	}

	t_pcb_serializado pcb_serializado = serializar(*pcb_ejecutandose);
	enviar(FIN_QUANTUM,pcb_serializado.tamanio,pcb_serializado.contenido_pcb,socket_nucleo);

	free(pcb_serializado.contenido_pcb);
	destruir_pcb(pcb_ejecutandose);
}

void matar_hilo_ejecucion(){
	pthread_cancel(hilo_ejecucion);	//fixme llamar a la misma funcion que la senal sigusr1
	log_warning(logcpu,"El programa se cerro forzosamente");
}

int main() {

	//Creo archivo de log
	logcpu = log_create("logcpu.log", "cpu", false, LOG_LEVEL_TRACE);
	conectar_cpu();
	log_trace(logcpu,"Termino la funcion conectar_cpu()");
	bool se_cerro_nucleo = false;
	//Mientras no se cierre la conexion con el nucleo
	while(!se_cerro_nucleo){

		//Recibo el paquete
		t_paquete* paquete_actual =recibir_paquete(socket_nucleo);
		log_info(logcpu,"Se recibio un pedido del nucleo. Cod op: %d",paquete_actual->cod_op);

		//Trato el paquete segun el codigo de operacion
		switch(paquete_actual->cod_op){
			case CORRER_PCB:
				log_info(logcpu,"Se recibio un pcb para correr");
				//Deserializo el pcb
				pcb_ejecutandose = deserializar(paquete_actual->datos);
				log_trace(logcpu,"PCB deserializado");

				log_trace(logcpu,"Se creara el hilo para correr el PCB");
				pthread_create(&hilo_ejecucion,NULL,(void *) correr_pcb,NULL);
				break;
			case FINALIZA_PROGRAMA:
				log_info(logcpu,"El nucleo pidio cerrar el programa");
				matar_hilo_ejecucion();
				break;
			//En caso de que el paquete recibido sea de la umc y no del nucleo
			case ERROR_COD_OP:
				log_error(logcpu,"Se cerro el nucleo");
				se_cerro_nucleo=true;
				matar_hilo_ejecucion();
				break;
			default:
				break;
		}
		destruir_paquete(paquete_actual);
	}

		//Recibo el paquete
	log_destroy(logcpu);
	log_info(logcpu, "Termino el proceso CPU");
	return (EXIT_SUCCESS);
}
