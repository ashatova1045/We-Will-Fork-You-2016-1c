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

