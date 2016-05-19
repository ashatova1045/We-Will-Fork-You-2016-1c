#include "primitivas.h"

extern int32_t quantumRestante;

static const int CONTENIDO_VARIABLE = 20;
static const int POSICION_MEMORIA = 0x10;

/*
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

 void dummy_imprimir(t_valor_variable valor) {
 printf("Imprimir %d\n", valor);
 }

 void dummy_imprimirTexto(char* texto) {
 printf("ImprimirTexto: %s", texto);
 }


functions = {
 .AnSISOP_definirVariable = dummy_definirVariable,
 .AnSISOP_obtenerPosicionVariable = dummy_obtenerPosicionVariable,
 .AnSISOP_dereferenciar = dummy_dereferenciar, .AnSISOP_asignar =
 dummy_asignar, .AnSISOP_imprimir = dummy_imprimir,
 .AnSISOP_imprimirTexto = dummy_imprimirTexto,

 };

kernel_functions = { };

//	analizadorLinea("variables a", &functions, &kernel_functions);
//	puts("hola!");

*/
