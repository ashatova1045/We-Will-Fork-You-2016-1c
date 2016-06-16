#include "primitivas.h"
#include "cpu.h"

t_puntero pos_fisica_a_logica(t_posMemoria posfisica){
	return posfisica.pag*tamPag+posfisica.offset;
}

t_posMemoria pos_logica_a_fisica(t_puntero poslogica){
	t_posMemoria respuesta;
	respuesta.pag = poslogica/tamPag;
	respuesta.offset = poslogica % tamPag;
	respuesta.size = 4; //todas las variables pesan 4
	return respuesta;
}

t_puntero definirVariable(t_nombre_variable identificador_variable){
	log_info(logcpu,"Se solicita definir %c", identificador_variable);

	if(pcb_ejecutandose->fin_stack.pag == pcb_ejecutandose->cant_pags_totales){
		log_warning(logcpu,"STACK OVERFLOW");
		puts("STACK OVERFLOW");
		//todo explotar
		exit(EXIT_FAILURE);
	}

	int entrada_actual = pcb_ejecutandose->cant_entradas_indice_stack-1;
	registro_indice_stack* stack_Actual = &pcb_ejecutandose->indice_stack[entrada_actual];

	int poslogica;

	//las variables son letras
	if(isalpha(identificador_variable)){
		log_debug(logcpu,"%c es una variable",identificador_variable);

		//incremento cant_varialbes para poder serializar
		stack_Actual->cant_variables++;
		int cant_variables = stack_Actual->cant_variables;

		//aumento el tamaño de las variables
		stack_Actual->variables=realloc(stack_Actual->variables,cant_variables*sizeof(t_variable));

		//creo la nueva variable
		t_variable nueva_variable;
		nueva_variable.id = identificador_variable;
		nueva_variable.posicionVar = pcb_ejecutandose->fin_stack; 	//me fijo donde va a ir la nueva variable (al final)

		stack_Actual->variables[cant_variables-1] = nueva_variable;

		poslogica = pos_fisica_a_logica(nueva_variable.posicionVar);
		log_info(logcpu,"Se definio la variable %c\n Posicion fisica: Pagina %d, offset %d\nPosicion logica: %d", identificador_variable,nueva_variable.posicionVar.pag,nueva_variable.posicionVar.offset,poslogica);

	}

	//los argumentos son numeros
	if(isdigit(identificador_variable)){
		log_debug(logcpu,"%c es un argumento",identificador_variable);

		//incremento cant_varialbes para poder serializar
		stack_Actual->cant_argumentos++;
		int cant_argumentos = stack_Actual->cant_argumentos;

		//aumento el tamaño de las variables
		stack_Actual->argumentos=realloc(stack_Actual->argumentos,cant_argumentos*sizeof(t_posMemoria));

		//el nuevo argumento va al final(donde marca el fin del stack ahora)
		stack_Actual->argumentos[cant_argumentos-1] = pcb_ejecutandose->fin_stack;

		poslogica = pos_fisica_a_logica(pcb_ejecutandose->fin_stack);
		log_info(logcpu,"Se definio el argumento %c\n Posicion fisica: Pagina %d, offset %d\nPosicion logica: %d", identificador_variable,pcb_ejecutandose->fin_stack.pag,pcb_ejecutandose->fin_stack.offset,poslogica);
	}

	//me muevo 4 lugares a la derecha para que la proxima variable no pise a esta
	pcb_ejecutandose->fin_stack.offset += 4;

	//me aseguro de nunca pasarme de la pagina
	if(pcb_ejecutandose->fin_stack.offset >= tamPag){
		pcb_ejecutandose->fin_stack.offset = 0;
		pcb_ejecutandose->fin_stack.pag++;
	}

	return poslogica;
}

t_puntero obtenerPosicionVariable(t_nombre_variable identificador_variable){
	log_info(logcpu,"Se solicita obtener la posición de %c", identificador_variable);

	registro_indice_stack* stack_actual = &pcb_ejecutandose->indice_stack[pcb_ejecutandose->cant_entradas_indice_stack-1];

	//las variables son letras
	if(isalpha(identificador_variable)){
		log_debug(logcpu,"%c parece ser una variable",identificador_variable);
		int i;
		for(i=stack_actual->cant_variables-1;i>=0;i--){
			if(stack_actual->variables[i].id == identificador_variable){
				t_puntero poslogica = pos_fisica_a_logica(stack_actual->variables[i].posicionVar);
				log_debug(logcpu,"La posicion logica de la variable %c es %d",identificador_variable,poslogica);
				return poslogica;
			}
		}
	}

	////los argumentos son numeros
	if(isdigit(identificador_variable)){
		int numero_variable = identificador_variable - '0';
		log_debug(logcpu,"%c parece ser un argumento",identificador_variable);

		if(numero_variable >= stack_actual->cant_argumentos)
			log_error(logcpu,"Es imposible que %d sea un argumento, porque solo hay %d argumentos",numero_variable,stack_actual->cant_argumentos);

		t_puntero poslogica = pos_fisica_a_logica(stack_actual->argumentos[numero_variable]);
		log_debug(logcpu,"La posicion logica del argumento %d es %d",numero_variable,poslogica);
		return poslogica;
	}

	log_error(logcpu,"No se pudo encontrar nada llamado %c",identificador_variable);
	//idealmente nunca llega a esto
	return -1;
}

t_valor_variable dereferenciar(t_puntero direccion_variable){

	t_posMemoria posfisica = pos_logica_a_fisica(direccion_variable);

	log_debug(logcpu,"Se solicita dereferenciar la dirección\nLogica: %d\nFisica: pag %d, offset %d", direccion_variable, posfisica.pag, posfisica.offset);

	t_pedido_solicitarBytes pedido;
	pedido.nroPagina = posfisica.pag;
	pedido.offset = posfisica.offset;
	pedido.tamanioDatos = posfisica.size;

	//pido a la umc lo que esta en ese pedido
	char* rta_umc = pedir_lectura_de_umc(pedido);

	//esta funcion tiene que devolver un t_valor_variable
	t_valor_variable valor = *(t_valor_variable *)rta_umc;

	free(rta_umc);

	log_info(logcpu,"Valor dereferenciado = %d",valor);
	return valor;

}

void asignar(t_puntero	direccion_variable,	t_valor_variable valor){

	t_posMemoria posf = pos_logica_a_fisica(direccion_variable);

	t_pedido_almacenarBytes pedido;
	pedido.nroPagina = posf.pag;
	pedido.offset = posf.offset;
	pedido.tamanioDatos = posf.size;
	pedido.buffer = (char *)&valor;

	t_pedido_almacenarBytes_serializado *pedidoserializado = serializar_pedido_almacenar(&pedido);

	log_info(logcpu,"Se solicita asignar la dirección logica %d con el valor %d (posicion fisica: pag %d, offset %d)", direccion_variable,valor,posf.pag,posf.offset);

	enviar(ESCRITURA_PAGINA,pedidoserializado->tamano,pedidoserializado->pedido_serializado,socket_umc);

	t_paquete *paquete_respuesta = recibir_paquete(socket_umc);

	switch (paquete_respuesta->cod_op) {
		case OK:
			log_info(logcpu,"Valor guardado correctamente");
			break;
		case NO_OK:
			log_warning(logcpu,"FALLO LA ASIGNACION");
			//todo explotar
			break;
		case ERROR_COD_OP:
			perror("Murio la UMC");
			log_error(logcpu,"Murio la UMC");
			exit(EXIT_FAILURE);
		default:
			perror("Codigo de operacion desconocido");
			log_error(logcpu,"Codigo de operacion desconocido");
			exit(EXIT_FAILURE);
			break;
	}

	free(pedidoserializado->pedido_serializado);
	free(pedidoserializado);
	destruir_paquete(paquete_respuesta);
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

void irAlLabel(t_nombre_etiqueta etiqueta){
	pcb_ejecutandose->pc = metadata_buscar_etiqueta(etiqueta,pcb_ejecutandose->indice_etiquetas,pcb_ejecutandose->tamano_etiquetas);
	log_info(logcpu,"Entrando al label: %s", etiqueta);
}

void llamarConRetorno(t_nombre_etiqueta	etiqueta, t_puntero donde_retornar){
	log_info(logcpu,"Llamando a la funcion %s", etiqueta);

	//agrego nuevo indice al stack
	pcb_ejecutandose->cant_entradas_indice_stack++;
	int tamano_stack = pcb_ejecutandose->cant_entradas_indice_stack;
	pcb_ejecutandose->indice_stack = realloc(pcb_ejecutandose->indice_stack,sizeof(registro_indice_stack)*tamano_stack);

	registro_indice_stack* stack_actual = &pcb_ejecutandose->indice_stack[tamano_stack-1];
	stack_actual->cant_argumentos=0;
	stack_actual->cant_variables=0;
	stack_actual->argumentos = NULL;
	stack_actual->variables = NULL;

	stack_actual->pos_retorno = pcb_ejecutandose->pc;
	stack_actual->pos_var_retorno = pos_logica_a_fisica(donde_retornar);

	irAlLabel(etiqueta);
}

void retornar(t_valor_variable retorno){
	log_info(logcpu,"Retornando de la funcion! Devolvio %d", retorno);

	int tamano_stack = pcb_ejecutandose->cant_entradas_indice_stack;
	registro_indice_stack* stack_actual = &pcb_ejecutandose->indice_stack[tamano_stack-1];

	//donde tengo que guardar lo que devuelve la funcion
	t_posMemoria pos_retorno = stack_actual->pos_var_retorno;
	t_puntero pos_retorno_logica = pos_fisica_a_logica(pos_retorno);

	//lo guardo ahi
	asignar(pos_retorno_logica,retorno);

	pcb_ejecutandose->cant_entradas_indice_stack--;


	pcb_ejecutandose->pc = stack_actual->pos_retorno;
	log_info(logcpu,"Vuelvo al PC %d", pcb_ejecutandose->pc);
}

void imprimir(t_valor_variable valor_mostrar){
	log_info(logcpu,"Se solicita imprimir el valor %d de una variable", valor_mostrar);

	enviar(IMPRIMIR_VARIABLE,sizeof(int32_t),&valor_mostrar,socket_nucleo);
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

 void inicializar_primitivas(){

	 functions = (AnSISOP_funciones) {
		.AnSISOP_imprimirTexto = imprimirTexto,
		.AnSISOP_imprimir = imprimir,
		.AnSISOP_finalizar = finalizar,
		.AnSISOP_entradaSalida =entradaSalida,
		.AnSISOP_obtenerValorCompartida = obtenerValorCompartida,
		.AnSISOP_asignarValorCompartida = asignarValorCompartida,
		.AnSISOP_irAlLabel = irAlLabel,
		.AnSISOP_definirVariable = definirVariable,
		.AnSISOP_obtenerPosicionVariable = obtenerPosicionVariable,
		.AnSISOP_dereferenciar = dereferenciar,
		.AnSISOP_asignar = asignar,
		.AnSISOP_llamarConRetorno = llamarConRetorno,
		.AnSISOP_retornar = retornar
	};

	kernel_functions =(AnSISOP_kernel) {
		.AnSISOP_wait = wait,
		.AnSISOP_signal = signall
	};

 }
