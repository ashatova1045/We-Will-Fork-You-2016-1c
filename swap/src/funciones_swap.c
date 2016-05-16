#include "funciones_swap.h"
#include "estructuras_swap.h"

extern int socket_memoria;
extern t_list* lista_procesos;
extern t_bitarray* bitarray;
t_swapcfg* datosSwap;

t_log* crearLog(){
	t_log *logSwap = log_create("log.txt", "swap.c", false, LOG_LEVEL_INFO);
	return logSwap;
}

t_swapcfg* levantarConfiguracion(t_config* config){
	datosSwap = malloc(sizeof(t_swapcfg));
	if(datosSwap != NULL){
		datosSwap->puerto_escucha = config_get_int_value(config,"PUERTO_ESCUCHA");
		datosSwap->nombre_swap = config_get_string_value(config, "NOMBRE_SWAP");
		datosSwap->cantidad_paginas = config_get_int_value(config, "CANTIDAD_PAGINAS");
		datosSwap->tamanio_pagina = config_get_int_value(config, "TAMANIO_PAGINA");
		datosSwap->retardo_compactacion = config_get_int_value(config, "RETARDO_COMPACTACION");
	}
	return datosSwap;
}

void inicializaSwapFile(t_swapcfg* config_swap){
	// Crea estructura bitmap
	char* array = malloc(sizeof(tamanioPagina));
	int cantBytes = (config_swap->cantidad_paginas) / 8;

	bitarray = bitarray_create(array, cantBytes);

	// Crea archivo SWAP
	char command[128];
	size_t tamanio_swap = config_swap->tamanio_pagina * config_swap->cantidad_paginas;

	snprintf(command, sizeof(command), "dd if=/dev/zero of=%s bs=%d count=1",config_swap->nombre_swap,tamanio_swap);
	system(command);
	swapFile = fopen("swap.dat","a+");

	/*char* bufferAux = malloc(sizeof("\0"));
	int posicionSwap = 0;
	do{
		fread(bufferAux,sizeof(bufferAux),1,swap);
		posicionSwap++;
	}while(bufferAux == "\0");
	printf("posicionSwap: %d \n",posicionSwap);*/

	//return swap;
}

void manejar_socket_umc(t_paquete* paquete){
	switch(paquete->cod_op){
	// Handshake
	case HS_UMC_SWAP:
		enviar(OK_HS_UMC,1,&socket_memoria,socket_memoria);
		printf("Handshake correcto! \n");
		log_info(logSwap,"Handshake correcto!");
		break;
	case ERROR_COD_OP:
		puts("Error en el recibir. Recibio codigo de error");
		log_error(logSwap,"Recibio codigo de error");
		exit(EXIT_FAILURE);
		break;
	case TAMANIO_PAGINA:
		 puts("Proceso recibido");
		 printf("Codigo de operacion: %d\n",paquete->cod_op);
		 printf("Tamano de los datos: %d\n",paquete->tamano_datos);
		 printf("Tamaño de los frames: %d\n",*((int*)paquete->datos));
		 tamanioPagina = *((int*)paquete->datos);
		 break;
	default:
		manejarOperaciones(paquete);
		break;
	}
}

void manejarOperaciones(t_paquete* paquete){
	switch(paquete->cod_op){
	// Operaciones
	case NUEVO_PROGRAMA:
		inicializarNuevoPrograma(paquete);
		break;
	case LECTURA_PAGINA:
		leerPagina(paquete);
		break;
	case ESCRITURA_PAGINA:
		escribirPagina(paquete);
		break;
	case FINALIZA_PROGRAMA:
		finalizarPrograma(paquete);
		break;
	}
}

void inicializarNuevoPrograma(t_paquete* paquete){
	//todo: Compactar partición en caso de fragmentación

	log_info(logSwap,"Inicio de proceso de inicialización de un nuevo programa");
	puts("INICIALIZAR NUEVO PROGRAMA");
	t_pedido_inicializar_swap* pedido = (t_pedido_inicializar_swap*)paquete->datos;

	printf("ProcessID: %d\n",pedido->idPrograma);
	printf("Cantidad de Paginas: %d\n",pedido->pagRequeridas);

	int cantidadPaginas = pedido->pagRequeridas;

	//char* msg = paginasPendientes
	//puts(msg);
	log_info(logSwap,"Id Proceso: %d",pedido->idPrograma);
	log_info(logSwap,"Cantidad de páginas requeridas: %d",cantidadPaginas);

	int codOp = NO_OK;

	if(verificarPaginasLibres(cantidadPaginas) == 0){

		int posicion = encontrar_espacio(cantidadPaginas);
		if (posicion == -1){

			//compactar();
			posicion = encontrar_espacio(cantidadPaginas);
		}

		//si hay lugar armar t_control_swap y sumarlo a la lista de procesos
		agregarNuevoProceso(posicion,cantidadPaginas,pedido);
		codOp = OK;
	}else{
		//fallar
		log_info(logSwap,"No se ha podido asignar el espacio necesario");
		puts("No se ha podido asignar el espacio necesario");
	}

	//printf("%d %d \n",codOp,socket_memoria);
	enviar(codOp,1,"OK",socket_memoria);
	puts("Envia respuesta a la UMC");
}


void leerPagina(t_paquete* paquete){
	//todo: Devolver página

	log_info(logSwap,"Inicia proceso de lectura de página");
	puts("LEER PAGINA");

	t_pedido_leer_swap* pedido = (t_pedido_leer_swap*)paquete->datos;

	int pid_enviado = pedido->pid;
	char* buffer = malloc(tamanioPagina);


	//Buscar el proceso en la lista
	int codOp = NO_OK;
	bool matchPID(void* controlSwap){
		return ((t_control_swap*)controlSwap)->PId == pid_enviado;
	}
	t_control_swap* controlSwap = list_find(lista_procesos,matchPID);

	if(controlSwap != NULL){
		int pos = (controlSwap->posicion * tamanioPagina);
		printf("Posición: %d \n",controlSwap->posicion);
		log_info(logSwap,"Posición: %d \n",controlSwap->posicion);
		fseek(swapFile,pos,SEEK_SET);
		sleep(datosSwap->retardo_compactacion);
		if(fread(buffer,tamanioPagina,1,swapFile) < 1){
			puts("Error en la lectura de la página");
			log_error(logSwap,"Error en la lectura de la página");
		}else{
			printf("Posición: %d \n",posicion);
			log_info(logSwap,"pid: %d \n",pid_enviado);
			log_info(logSwap,"Buffer %*.s",tamanioPagina,buffer);

			codOp = BUFFER_LEIDO;
			puts("Lectura de la página exitosa");
			log_info(logSwap,"Lectura de la página exitosa");
		}
	}else{
		puts("Error - No se ha encontrado el proceso a leer");
		log_error(logSwap,"Error - No se ha encontrado el proceso a leer");
	}

	// Responde a la UMC el resultado de la operación
	// Sin serializar - manda buffer
	enviar(codOp,1,buffer,socket_memoria);
}

void escribirPagina(t_paquete* paquete){
	//todo: Deserializar el paquete
	//todo: Sobreescribir página

	log_info(logSwap,"Inicia proceso de escritura de página");
	puts("ESCRIBE PAGINA");

	t_pedido_almacenar_swap* pedido = deserializar_pedido_almacenar_swap(paquete->datos,tamanioPagina);

	int pid_enviado = pedido->pid;
	//int pagina = pedido->nroPagina;

	char* buffer = malloc(sizeof(pedido->buffer));
	buffer = pedido->buffer;

	printf("Posición: %d \n",posicion);
	log_info(logSwap,"pid: %d \n",pid_enviado);
	log_info(logSwap,"Buffer %*.s",tamanioPagina,buffer);

	// Grabar y mandar resultado a la umc
	fseek(swapFile,posicion,SEEK_SET);

	int codOp = NO_OK;
//	if(fseek(swapFile,1,SEEK_SET) == 0){
		if(fwrite(buffer,tamanioPagina,1,swapFile) != 1){
			log_error(logSwap, "No se pudo grabar la página en el archivo");
			puts("No se pudo grabar la página en el archivo");
		}else{
			codOp = OK;
			log_info(logSwap,"Grabación exitosa de la página en el archivo");
			puts("Grabación exitosa");
		}
	/*}else{
		puts("Error al setear la posición en el archivo_");
		log_error(logSwap,"Error al setear la posición en el archivo_");
	}*/

	// Responde a la UMC el resultado de la operación
	enviar(codOp,3,&socket_memoria,socket_memoria);

}

void finalizarPrograma(t_paquete* paquete){
	//todo: Liberar espacio en caso que se finalice el proceso
	log_info(logSwap,"Inicia proceso de finalización de programa");
	puts("FINALIZA PROGRAMA");

	t_pedido_finalizar_swap* pedido = (t_pedido_finalizar_swap*)paquete->datos;

	int pid_enviado = pedido->idPrograma;
	int i;
	int encuentraProceso = 0;

	//Buscar el proceso en la lista
	bool matchPID(void* controlSwap){
		return ((t_control_swap*)controlSwap)->PId == pid_enviado;
	}
	t_control_swap* controlSwap = list_find(lista_procesos,matchPID);

	int posAux = controlSwap->posicion;
	int cantidadPaginas = controlSwap->cantPaginas;

	// Establece como libre los bitmaps que ocupó el proceso
	for(i=0;i<cantidadPaginas;i++){
		bitarray_clean_bit(bitarray,posAux);
		posAux++;
	}

	free(list_remove(lista_procesos,i));
	encuentraProceso = 1;

	int codOp;
	if(encuentraProceso == 1){
		codOp = TERMINO_BIEN_PROGRAMA;
		log_info(logSwap,"Finalización del programa exitosa");
		puts("Finalización del programa exitosa");
	}else{
		codOp = TERMINO_MAL_PROGRAMA;
		log_error(logSwap,"La finalización del programa ha fallado");
		puts("La finalización del programa ha fallado");
	}
	// Responde a la UMC el resultado de la operación
	enviar(codOp,3,"OK",socket_memoria);
}

int verificarPaginasLibres(int cantidadPaginas) {
	// Revisa el bitmap si hay lugar para el proceso
	int paginasPendientes = cantidadPaginas;
	int i, max = bitarray_get_max_bit(bitarray);
	for (i = 0; i < max; i++) {
		if (bitarray_test_bit(bitarray, i) == false) {
			paginasPendientes--;
			if(paginasPendientes == 0){
				return 0;
			}
		}
	}
	return paginasPendientes;
}

int encontrar_espacio(int cantidadPaginas) {
	int i, max = bitarray_get_max_bit(bitarray);
	for (i = 0; i < max; i++) {
		if (bitarray_test_bit(bitarray, i) == false) {
			int j;
			for (j = 0; j < cantidadPaginas; j++) {
				if (bitarray_test_bit(bitarray, i + j))
					break;
			}
			if (j == cantidadPaginas)
				return i;
		}
	}
	return -1;
}

void agregarNuevoProceso(int posicion, int cantidadPaginas, t_pedido_inicializar_swap* pedido){
	int i, posAux = posicion;

	// Establece como ocupado los bitmaps que ocupa el proceso si hay disponibilidad
	for(i=0;i<cantidadPaginas;i++){
		bitarray_set_bit(bitarray,posAux);
		posAux++;
	}

	t_control_swap* controlSwap = malloc(sizeof(t_control_swap));
	controlSwap->PId = pedido->idPrograma;
	controlSwap->cantPaginas = pedido->pagRequeridas;
	controlSwap->posicion = posicion;

	list_add(lista_procesos,controlSwap);
	log_info(logSwap,"Se inicializó el espacio necesario");

}
