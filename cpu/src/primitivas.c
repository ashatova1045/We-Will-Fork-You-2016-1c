#include "primitivas.h"
#include "cpu.h"

int pc;

t_posicion* definirVariable(t_nombre_variable identificador_variable){

	t_posicion* posicion = malloc(sizeof(t_posicion));

	log_info(logcpu,"Se solicita definir la variable %d\n", identificador_variable);

	enviar(DEFINIR_VARIABLE,sizeof(t_nombre_variable),&identificador_variable,socket_nucleo);

	t_paquete *respuesta_defVar = recibir_paquete(socket_nucleo);
	switch (respuesta_defVar->cod_op) {
		case OK:
			posicion = (t_posicion*)respuesta_defVar->datos;
			log_debug(logcpu,"Se ha obtenido el valor correctamente");
			break;
		case NO_OK:
			log_error(logcpu,"El nucleo reportó un error");
			break;
		default:
			log_error(logcpu,"Se desconectó el núcleo!");
			destruir_paquete(respuesta_defVar);
			exit(EXIT_FAILURE);
			break;
	}

	destruir_paquete(respuesta_defVar);

	return posicion;

}

t_posicion* obtenerPosicionVariable(t_nombre_variable identificador_variable){
	t_posicion* posicion = malloc(sizeof(t_posicion));

	log_info(logcpu,"Se solicita obtener la posición de %s\n", identificador_variable);

	enviar(OBTENER_POSICION,sizeof(t_nombre_variable),&identificador_variable,socket_nucleo);

	t_paquete *respuesta_posicion = recibir_paquete(socket_nucleo);
	switch (respuesta_posicion->cod_op) {
		case OK:
			posicion->offset = (int)respuesta_posicion->datos;
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

t_valor_variable dereferenciar(t_posicion* direccion_variable){

	t_valor_variable* valorVariable = malloc(sizeof(t_valor_variable));

	log_info(logcpu,"Se solicita dereferenciar la dirección %d\n", direccion_variable);

	enviar(DEREFERENCIAR,sizeof(t_nombre_variable),direccion_variable,socket_nucleo);

	t_paquete *respuesta_deref = recibir_paquete(socket_nucleo);
	switch (respuesta_deref->cod_op) {
		case OK:
			valorVariable = (t_valor_variable*)respuesta_deref->datos;
			log_debug(logcpu,"Se ha obtenido el valor correctamente");
			break;
		case NO_OK:
			log_error(logcpu,"El nucleo reportó un error");
			break;
		default:
			log_error(logcpu,"Se desconectó el núcleo!");
			destruir_paquete(respuesta_deref);
			exit(EXIT_FAILURE);
			break;
	}

	destruir_paquete(respuesta_deref);

	return *valorVariable;

}

void asignar(t_posicion	direccion_variable,	t_valor_variable valor){

	t_asignar* paquete_asignar = malloc(sizeof(t_asignar));

	paquete_asignar->direccion_variable = direccion_variable;
	paquete_asignar->valor = valor;

	log_info(logcpu,"Se solicita asignar la dirección %d con el valor %s\n", direccion_variable,valor);

	enviar(ASIGNAR,sizeof(t_asignar),paquete_asignar,socket_nucleo);

}

t_valor_variable obtenerValorCompartida(t_nombre_compartida	variable){
	log_info(logcpu,"Se solicita el valor de la variable compartida %s", variable);

	int32_t valor_compartida;

	enviar(OBTENER_VALOR_COMPARTIDA,strlen(variable)+1,variable,socket_nucleo);

	t_paquete *respuesta_valor_comp = recibir_paquete(socket_nucleo);
	switch (respuesta_valor_comp->cod_op) {
		case OK:
			valor_compartida = *((int32_t*)respuesta_valor_comp->datos);
			log_info(logcpu,"Se ha obtenido el valor de la variable compartida correctamente");
			break;
		case NO_OK:
			log_error(logcpu,"El nucleo reportó un error al obtener el valor de la variable compartida solicitada");
			break;
		default:
			log_error(logcpu,"Se desconectó el núcleo!");
			destruir_paquete(respuesta_valor_comp);
			exit(EXIT_FAILURE);
			break;
	}

	destruir_paquete(respuesta_valor_comp);

	return valor_compartida;
}

t_valor_variable asignarValorCompartida(t_nombre_compartida	variable, t_valor_variable valor_variable){
	t_varCompartida paquete_asignar_var_comp;
	paquete_asignar_var_comp.id_var = variable;
	paquete_asignar_var_comp.valor = valor_variable;

	t_pedido_serializado ser = serializar_asignar_compartida(paquete_asignar_var_comp);

	log_info(logcpu,"Se solicita asignar el valor %d a la variable compartida %s", valor_variable, variable);

	enviar(ASIGNAR_VALOR_COMPARTIDA,ser.tamanio,ser.pedido_serializado,socket_nucleo);

	free(ser.pedido_serializado);

	return valor_variable;
}

t_puntero_instruccion irAlLabel(t_nombre_etiqueta etiqueta){
	t_puntero_instruccion* instruccion = malloc(sizeof(t_puntero_instruccion));
	log_info(logcpu,"Se solicita la primera instrucción ejecutable de %s\n", etiqueta);

	enviar(OBTENER_INSTRUCCION,sizeof(t_nombre_etiqueta),etiqueta,socket_nucleo);

	t_paquete *respuesta_label = recibir_paquete(socket_nucleo);
	switch (respuesta_label->cod_op) {
		case OK:
			instruccion = (t_puntero_instruccion*)respuesta_label->datos;
			log_info(logcpu,"Se ha obtenido la instrucción solicitada correctamente");
			break;
		case NO_OK:
			*instruccion = -1;
			log_error(logcpu,"El nucleo reportó un error al obtener la instrucción solicitada");
			break;
		default:
			log_error(logcpu,"Se desconectó el núcleo!");
			destruir_paquete(respuesta_label);
			exit(EXIT_FAILURE);
			break;
	}

	destruir_paquete(respuesta_label);

	return *instruccion;
}

void llamarConRetorno(t_nombre_etiqueta	etiqueta, t_puntero donde_retornar){

}

t_puntero_instruccion retornar(t_valor_variable retorno){
	t_puntero_instruccion* instruccion = malloc(sizeof(t_puntero_instruccion));
	return *instruccion;
}

int	imprimir(t_valor_variable valor_mostrar){
	int cantCarImp;

	log_info(logcpu,"Se solicita imprimir el valor de la variable %s\n", valor_mostrar);

	enviar(IMPRIMIR_VARIABLE,sizeof(t_valor_variable),&valor_mostrar,socket_nucleo);

	t_paquete *respuesta_imprime_variable = recibir_paquete(socket_nucleo);
	switch (respuesta_imprime_variable->cod_op) {
		case OK:
			cantCarImp = (int)respuesta_imprime_variable->datos;
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

void imprimirTexto(char* texto) {
	log_info(logcpu,"PRIMITIVA: Imprimiendo el texto:\n%s",texto);

	enviar(IMPRIMIR_TEXTO,strlen(texto)+1,texto,socket_nucleo);
}

void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo){
	t_pedido_wait paquete_e_s;

	paquete_e_s.semaforo = dispositivo;
	paquete_e_s.tiempo = tiempo;
	paquete_e_s.pcb = pcb_ejecutandose;

	t_pedido_wait_serializado *paquete_e_s_serializado = serializar_wait(&paquete_e_s);

	log_info(logcpu,"Intentando usar %s por %d unidades de tiempo", dispositivo, tiempo);

	enviar(ENTRADA_SALIDA,paquete_e_s_serializado->tamano,paquete_e_s_serializado->contenido,socket_nucleo);

	log_debug(logcpu,"El proceso %d se bloqueo!",pcb_ejecutandose->pid);
	termino_programa = true;
	destruir_pcb(pcb_ejecutandose);
	free(paquete_e_s_serializado->contenido);
	free(paquete_e_s_serializado);
}

/*
int grabar_valor(t_nombre_variable identificador_variable, void* valorGrabar){
	int codResp;
	t_grabar_valor* paquete_grabar_valor = malloc(sizeof(t_grabar_valor));

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
 */
void signall(t_nombre_semaforo identificador_semaforo){
	log_info(logcpu,"Se solicita ejecutar la función signal para el semáforo %s", identificador_semaforo);

	enviar(SIGNAL,strlen(identificador_semaforo)+1,identificador_semaforo,socket_nucleo);
}

void wait(t_nombre_semaforo identificador_semaforo){
	log_info(logcpu,"Se solicita ejecutar la función wait para el semáforo %s", identificador_semaforo);

	t_pedido_wait* pedidowait = malloc(sizeof(t_pedido_wait));
	pedidowait->semaforo = identificador_semaforo;
	pedidowait->pcb = pcb_ejecutandose;

	t_pedido_wait_serializado *pwaitser = serializar_wait(pedidowait);

	enviar(WAIT,pwaitser->tamano,pwaitser->contenido,socket_nucleo);

	t_paquete *respuesta_signal = recibir_paquete(socket_nucleo);
	switch (respuesta_signal->cod_op) {
		case OK:
			log_debug(logcpu,"El proceso %d no se bloqueo!",pcb_ejecutandose->pid);
			break;
		case NO_OK:
			log_debug(logcpu,"El proceso %d se bloqueo!",pcb_ejecutandose->pid);
			termino_programa = true;
			destruir_pcb(pcb_ejecutandose);
			break;
		default:
			log_error(logcpu,"Se desconectó el núcleo!");
			destruir_paquete(respuesta_signal);
			exit(EXIT_FAILURE);
			break;
	}

	destruir_paquete(respuesta_signal);
//	free(pwaitser->contenido);
//	free(pwaitser);
}

void finalizar() {
	termino_programa = true;

	t_pcb_serializado pcbs = serializar(*pcb_ejecutandose);
	enviar(FINALIZA_PROGRAMA,pcbs.tamanio,pcbs.contenido_pcb,socket_nucleo);

	free(pcbs.contenido_pcb);

	destruir_pcb(pcb_ejecutandose);

	log_info(logcpu,"PRIMITIVA: Fin");
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
		.AnSISOP_finalizar = finalizar,
		.AnSISOP_entradaSalida =entradaSalida,
		.AnSISOP_obtenerValorCompartida = obtenerValorCompartida,
		.AnSISOP_asignarValorCompartida = asignarValorCompartida,
		// .AnSISOP_definirVariable = dummy_definirVariable,
		// .AnSISOP_obtenerPosicionVariable = dummy_obtenerPosicionVariable,
		// .AnSISOP_dereferenciar = dummy_dereferenciar, .AnSISOP_asignar =
		// dummy_asignar, .AnSISOP_imprimir = dummy_imprimir,

	};

	kernel_functions =(AnSISOP_kernel) {
		.AnSISOP_wait = wait,
		.AnSISOP_signal = signall
	};

 }
