#include "cpu.h"

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
	quantumCpu = (*(int32_t*) paqNucleo->datos);

	log_info(logcpu, "Quantum del CPU = : %d", quantumCpu);

	destruir_paquete(paqNucleo);

	if (handshake(socket_umc, HS_CPU_UMC, OK_HS_CPU) == -1) {
		log_error(logcpu, "Handshake de umc incorrecto");
		puts("No se pudo hacer un hansdhake con la umc");
		exit(EXIT_FAILURE);
	}
	puts("Handshake de umc correcto!");
	log_info(logcpu, "Handshake de umc correcto!");

	t_paquete *paqUmc = recibir_paquete(socket_umc);
	tamPag = (*(int32_t*) paqUmc->datos);
	log_info(logcpu, "Tamanio de pagina: %d", tamPag);

	destruir_paquete(paqUmc);

//Elimino la configuración que ya no necesito
	config_destroy(conf);
}

char* recibir_respuesta_lectura() {
	//Recibo el paquete
	t_paquete* paquete_actual = recibir_paquete(socket_umc);
	log_trace(logcpu,
			"Se recibio un mensaje de respuesta de lectura de la UMC. cod op %d",
			paquete_actual->cod_op);
	//Trato el paquete segun el codigo de operacion
	switch (paquete_actual->cod_op) {
	case BUFFER_LEIDO:
		log_debug(logcpu, "Datos leidos: %s", paquete_actual->datos);
		break;
	case NO_OK:
		log_info(logcpu, "NO OK recibido");
		//Respuesta a un pedido
		break;
	default:
		puts("Se recibio un codigo de error o desconocido de la UMC");
		log_error(logcpu,
				"Se recibio un codigo de error o desconocido de la UMC");
		destruir_paquete(paquete_actual);
		exit(EXIT_FAILURE);
		//En caso de que el paquete recibido sea del nucleo y no se la umc
		break;
	}
	char* respuesta = strdup(paquete_actual->datos);
	destruir_paquete(paquete_actual);
	if (respuesta == NULL) {
		puts("Error al reservar memoria para la lectura requerida");
		log_error(logcpu,
				"Error al reservar memoria para la lectura requerida");
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

int min (int a,int b){
	return a<b?a:b;
}

void correr_pcb() {
	//Seteo proceso activo
	enviar(CAMBIO_PROCESO_ACTIVO, sizeof(pcb_ejecutandose->pid),
			&pcb_ejecutandose->pid, socket_umc);
	log_debug(logcpu, "Se envio el cambio de proceso activo %d",
			pcb_ejecutandose->pid);

	termino_programa = false;
	int quantum_actual;
	for (quantum_actual = 1; quantum_actual <= quantumCpu && !termino_programa; quantum_actual++) {
		log_info(logcpu, "\n\n\n\nCorriendo instruccion %d de %d",
				quantum_actual, quantumCpu);
		t_posMemoria posicion_instruccion_actual =
				pcb_ejecutandose->indice_codigo[pcb_ejecutandose->pc];

		
		//Pido instrucción a umc

		int tamanoinstruccion_que_entra = min(tamPag-posicion_instruccion_actual.offset,posicion_instruccion_actual.size);

		t_pedido_solicitarBytes pedido;
		pedido.nroPagina = posicion_instruccion_actual.pag;
		pedido.offset = posicion_instruccion_actual.offset;
		pedido.tamanioDatos = tamanoinstruccion_que_entra;

		//Envío pedido a UMC
		char* instruccion_actual = pedir_lectura_de_umc(pedido);

		int tamanoinstruccionfaltante=posicion_instruccion_actual.size-tamanoinstruccion_que_entra;

		if(tamanoinstruccionfaltante>0){
			pedido.nroPagina++;
			pedido.offset=0;
			pedido.tamanioDatos = tamanoinstruccionfaltante;
			char* instruccion_actual2 = pedir_lectura_de_umc(pedido);
			instruccion_actual = realloc(instruccion_actual,tamanoinstruccion_que_entra+tamanoinstruccionfaltante);
			memcpy(instruccion_actual+tamanoinstruccion_que_entra,instruccion_actual2,tamanoinstruccionfaltante);
			free(instruccion_actual2);
		}

		//Parseo de la instruccion
		analizadorLinea(instruccion_actual, &functions, &kernel_functions);

		//Incremento el program counter
		pcb_ejecutandose->pc++;

		//Destuyo la instruccion
		free(instruccion_actual);
	}

	t_pcb_serializado pcb_serializado = serializar(*pcb_ejecutandose);

	int codigo_respuesta = FIN_QUANTUM;
	if(termino_programa)
		codigo_respuesta = FINALIZA_PROGRAMA;
	enviar(codigo_respuesta, pcb_serializado.tamanio, pcb_serializado.contenido_pcb,socket_nucleo);

	free(pcb_serializado.contenido_pcb);
	destruir_pcb(pcb_ejecutandose);
}

void matar_hilo_ejecucion() {
	log_warning(logcpu, "El programa se cerro forzosamente");
//	pthread_cancel(hilo_ejecucion);	//fixme llamar a la misma funcion que la senal sigusr1
}

int main() {

	//Creo archivo de log
	logcpu = log_create("logcpu.log", "cpu", false, LOG_LEVEL_DEBUG);

	inicializar_primitivas();

	conectar_cpu();
	log_trace(logcpu, "Termino la funcion conectar_cpu()");

	bool se_cerro_nucleo = false;
	//Mientras no se cierre la conexion con el nucleo
	while (!se_cerro_nucleo) {

		//Recibo el paquete
		t_paquete* paquete_actual = recibir_paquete(socket_nucleo);
		log_debug(logcpu, "Se recibio un pedido del nucleo. Cod op: %d",
				paquete_actual->cod_op);

		//Trato el paquete segun el codigo de operacion
		switch (paquete_actual->cod_op) {
		case CORRER_PCB:
			log_info(logcpu, "Se recibio un pcb para correr");
			//Deserializo el pcb
			pcb_ejecutandose = deserializar(paquete_actual->datos);
			log_trace(logcpu, "PCB deserializado");

			log_trace(logcpu, "Se creara el hilo para correr el PCB");
			correr_pcb();
			break;
		case FINALIZA_PROGRAMA:
			log_warning(logcpu, "El nucleo pidio cerrar el programa");
			matar_hilo_ejecucion();
			break;
			//En caso de que el paquete recibido sea de la umc y no del nucleo
		case ERROR_COD_OP:
			log_error(logcpu, "Se cerro el nucleo");
			se_cerro_nucleo = true;
			matar_hilo_ejecucion();
			break;
		default:
			break;
		}
		destruir_paquete(paquete_actual);
	}

	log_info(logcpu, "Termino el proceso CPU");
	log_destroy(logcpu);
	return (EXIT_SUCCESS);
}
