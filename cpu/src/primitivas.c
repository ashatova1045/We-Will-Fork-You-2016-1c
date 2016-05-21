#include "primitivas.h"

void imprimirTexto(char* texto) {
	log_info(logcpu,"Imprimiendo el texto:\n%s",texto);
	enviar(IMPRIMIR_TEXTO,strlen(texto)+1,texto,socket_nucleo);

	t_paquete *respuesta_imprimir = recibir_paquete(socket_nucleo);
	switch (respuesta_imprimir->cod_op) {
		case OK:
			log_debug(logcpu,"Impresion correcta de texto");
			break;
		case NO_OK:
			log_error(logcpu,"El nucleo reporto un error al imprimir texto");
			break;
		default:
			log_error(logcpu,"Se desconecto el nucleo!");
			destruir_paquete(respuesta_imprimir);
			exit(EXIT_FAILURE);
			break;
	}

	destruir_paquete(respuesta_imprimir);
}

typedef struct{
	t_nombre_dispositivo* dispositivo;
	int tiempo;
}t_entrada_salida;

int	entradaSalida(t_nombre_dispositivo dispositivo, int tiempo){
	int codResp;
	t_entrada_salida* paquete_e_s = malloc(sizeof(t_entrada_salida*));

	paquete_e_s->dispositivo = dispositivo;
	paquete_e_s->tiempo = tiempo;

	log_info(logcpu,"Intentando usar %s por %d unidades de tiempo:\n", dispositivo, tiempo);

	enviar(ENTRADA_SALIDA,sizeof(paquete_e_s),paquete_e_s,socket_nucleo);

	t_paquete *respuesta_e_s = recibir_paquete(socket_nucleo);
	switch (respuesta_e_s->cod_op) {
		case OK:
			codResp = 0;
			log_debug(logcpu,"Es permitido el uso del dispositivo de e/s");
			break;
		case NO_OK:
			codResp = 1;
			log_error(logcpu,"El nucleo reportó un error al permitir el uso del dispositivo de e/s");
			break;
		default:
			log_error(logcpu,"Se desconectó el núcleo!");
			destruir_paquete(respuesta_e_s);
			exit(EXIT_FAILURE);
			break;
	}

	destruir_paquete(respuesta_e_s);
	return codResp;
}

typedef struct{
	t_nombre_variable* variable;
	void* valorGrabar;
}t_grabar_valor;

int grabar_valor(t_nombre_variable identificador_variable, void* valorGrabar){
	int codResp;
	t_grabar_valor* paquete_grabar_valor = malloc(sizeof(t_grabar_valor*));

	paquete_grabar_valor->variable = identificador_variable;
	paquete_grabar_valor->valorGrabar = valorGrabar;

	log_info(logcpu,"Se solicita grabar en %s el valor %s\n", identificador_variable, (char*)valorGrabar);

	enviar(GRABAR_VALOR,sizeof(paquete_grabar_valor),paquete_grabar_valor,socket_nucleo);

	t_paquete *respuesta_grabar_valor = recibir_paquete(socket_nucleo);
	switch (respuesta_grabar_valor->cod_op) {
		case OK:
			codResp = 0;
			log_info(logcpu,"La petición de grabación ha sido realizada correctamente");
			break;
		case NO_OK:
			codResp = -1;
			log_error(logcpu,"El nucleo reportó un error al grabar la variable");
			break;
		default:
			log_error(logcpu,"Se desconectó el núcleo!");
			destruir_paquete(respuesta_grabar_valor);
			exit(EXIT_FAILURE);
			break;
	}

	destruir_paquete(respuesta_grabar_valor);

	return codResp;
}

typedef struct{
	int offset;
}t_posicion;


t_posicion obtenerPosicionVariable(t_nombre_variable identificador_variable){
	t_posicion* posicion = malloc(sizeof(t_posicion));

	log_info(logcpu,"Se solicita obtener la posición de %s\n", identificador_variable);

	enviar(OBTENER_POSICION,sizeof(t_nombre_variable),identificador_variable,socket_nucleo);

	t_paquete *respuesta_posicion = recibir_paquete(socket_nucleo);
	switch (respuesta_posicion->cod_op) {
		case OK:
			posicion->offset = (int*)respuesta_posicion->datos
			log_info(logcpu,"La petición de grabación ha sido realizada correctamente");
			break;
		case NO_OK:
			posicion->offset = -1;
			log_error(logcpu,"El nucleo reportó un error al grabar la variable");
			break;
		default:
			log_error(logcpu,"Se desconectó el núcleo!");
			destruir_paquete(respuesta_posicion);
			exit(EXIT_FAILURE);
			break;
	}

	destruir_paquete(respuesta_posicion);

	return posicion;
}

int	imprimir(t_valor_variable valor_mostrar){
	int cantCarImp;

	log_info(logcpu,"Se solicita imprimir el valor de la variable %s\n", valor_mostrar);

	enviar(IMPRIMIR_VARIABLE,sizeof(t_valor_variable),valor_mostrar,socket_nucleo);

	t_paquete *respuesta_imprime_variable = recibir_paquete(socket_nucleo);
	switch (respuesta_imprime_variable->cod_op) {
		case OK:
			cantCarImp = (int*)respuesta_imprime_variable->datos;
			log_info(logcpu,"Se ha impreso por pantalla el valor de la variable");
			break;
		case NO_OK:
			cantCarImp = -1;
			log_error(logcpu,"El nucleo reportó un error al imprimir el valor de la variable");
			break;
		default:
			log_error(logcpu,"Se desconectó el núcleo!");
			destruir_paquete(respuesta_imprime_variable);
			exit(EXIT_FAILURE);
			break;
	}

	destruir_paquete(respuesta_imprime_variable);

	return cantCarImp;
}


int	wait(t_nombre_semaforo identificador_semaforo){
	int codResp;

	log_info(logcpu,"Se solicita ejecutar la función wait para el semáforo %s\n", identificador_semaforo);

	enviar(WAIT,sizeof(t_nombre_semaforo),identificador_semaforo,socket_nucleo);

	t_paquete *respuesta_wait = recibir_paquete(socket_nucleo);
	switch (respuesta_wait->cod_op) {
		case OK:
			codResp = 0;
			log_info(logcpu,"Se ha realizado la función wait sobre el semáforo correctamente");
			break;
		case NO_OK:
			codResp = -1;
			log_error(logcpu,"El nucleo reportó un error al realizar la función wait");
			break;
		default:
			log_error(logcpu,"Se desconectó el núcleo!");
			destruir_paquete(respuesta_wait);
			exit(EXIT_FAILURE);
			break;
	}

	destruir_paquete(respuesta_wait);

	return codResp;
}

int	signal(t_nombre_semaforo identificador_semaforo){
	int codResp;

	log_info(logcpu,"Se solicita ejecutar la función signal para el semáforo %s\n", identificador_semaforo);

	enviar(SIGNAL,sizeof(t_nombre_semaforo),identificador_semaforo,socket_nucleo);

	t_paquete *respuesta_signal = recibir_paquete(socket_nucleo);
	switch (respuesta_signal->cod_op) {
		case OK:
			codResp = 0;
			log_info(logcpu,"Se ha realizado la función signal sobre el semáforo correctamente");
			break;
		case NO_OK:
			codResp = -1;
			log_error(logcpu,"El nucleo reportó un error al realizar la función signal");
			break;
		default:
			log_error(logcpu,"Se desconectó el núcleo!");
			destruir_paquete(respuesta_signal);
			exit(EXIT_FAILURE);
			break;
	}

	destruir_paquete(respuesta_signal);

	return codResp;
}

t_puntero_instruccion irAlLabel(t_nombre_etiqueta etiqueta){
	t_puntero_instruccion* instruccion = malloc(sizeof(t_puntero_instruccion));

	log_info(logcpu,"Se solicita la primera instrucción ejecutable de %s\n", etiqueta);

	enviar(SIGNAL,sizeof(t_nombre_etiqueta),etiqueta,socket_nucleo);

	t_paquete *respuesta_label = recibir_paquete(socket_nucleo);
	switch (respuesta_label->cod_op) {
		case OK:
			codResp = 0;
			log_info(logcpu,"Se ha obtenido la instrucción solicitada correctamente");
			break;
		case NO_OK:
			codResp = -1;
			log_error(logcpu,"El nucleo reportó un error al robtener la instrucción solicitada");
			break;
		default:
			log_error(logcpu,"Se desconectó el núcleo!");
			destruir_paquete(respuesta_label);
			exit(EXIT_FAILURE);
			break;
	}

	destruir_paquete(respuesta_label);

	return instruccion;
}



/*
 void dummy_imprimir(t_valor_variable valor) {
	 printf("Imprimir %d\n", valor);
 }
t_puntero dummy_definirVariable(t_nombre_variable variable) {
	uint32_t puntero = apilarVariable(variable);
	printf("definir la variable %c\n", variable);
	log_trace(logcpu,
			"Llamada a definirVariable, el identificador es: %c y esta en la posicion %d",
			variable, puntero);
	return POSICION_MEMORIA;
}

t_puntero dummy_obtenerPosicionVariable(t_nombre_variable variable) {
	uint32_t puntero = obtenerOffsetVarible(variable);
	printf("Obtener posicion de %c\n", variable);
	if (puntero == 0) {
		//PCB_enEjecucion.lastErrorCode = 3;
		quantumRestante = 0;
		log_error(logcpu,
				"Se solicito una posicion de una variable inexistente %c",
				variable);
	} else {
		log_trace(logcpu,
				"Llamada a obtenerPosicionVariable, el identificador es: %c, y esta en %d",
				variable, puntero);
	}
	return POSICION_MEMORIA;
}

t_valor_variable dummy_dereferenciar(t_puntero puntero) {

	printf("Dereferenciar %d y su valor es: %d\n", puntero, CONTENIDO_VARIABLE);
	log_trace(logcpu, "Llamada a dereferenciar, direccion: %d", puntero);
	return CONTENIDO_VARIABLE; //Revisar
	//return obtenerValor((uint32_t) puntero); Revisar

}

void dummy_asignar(t_puntero puntero, t_valor_variable variable) {
 printf("Asignando en %d el valor %d\n", puntero, variable);
 modificarVariable( (uint32_t) variable, (uint32_t) valor );
 log_trace(logcpu, "Llamada a asignar [ %d ] = %d ", variable, valor );
 }

*/

 void inicializar_primitivas(){

	 functions = (AnSISOP_funciones) {
		.AnSISOP_imprimirTexto = imprimirTexto,
		// .AnSISOP_definirVariable = dummy_definirVariable,
		// .AnSISOP_obtenerPosicionVariable = dummy_obtenerPosicionVariable,
		// .AnSISOP_dereferenciar = dummy_dereferenciar, .AnSISOP_asignar =
		// dummy_asignar, .AnSISOP_imprimir = dummy_imprimir,

	};

	kernel_functions =(AnSISOP_kernel) {

	};
 }
