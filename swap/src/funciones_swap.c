#include "funciones_swap.h"
#include "estructuras_swap.h"

t_log* crearLog(){
	t_log *logSwap = log_create("logSwap.log", "swap.c", false, LOG_LEVEL_TRACE);
	return logSwap;
}

void levantarConfiguracion(t_config* config){
	datosSwap = malloc(sizeof(t_swapcfg));
	if(datosSwap != NULL){
		datosSwap->puerto_escucha = config_get_int_value(config,"PUERTO_ESCUCHA");
		datosSwap->nombre_swap = config_get_string_value(config, "NOMBRE_SWAP");
		datosSwap->cantidad_paginas = config_get_int_value(config, "CANTIDAD_PAGINAS");
		datosSwap->retardo_acceso = config_get_int_value(config, "RETARDO_ACCESO");
		datosSwap->retardo_compactacion = config_get_int_value(config, "RETARDO_COMPACTACION");
	}
}

int inicializaSwapFile(){
	// Inicializa variables
	char* array = malloc(((datosSwap->cantidad_paginas)/8)+1);
	int i;
	for(i=0;i<(((datosSwap->cantidad_paginas))/8)+1;i++)
	        array[i]=0;

	//Se agrega un byte para evitar errores en la creacón del bitmap
	int cantBytes = ((datosSwap->cantidad_paginas)/8)+1 ;

	int codRet = 0;

	// Crea estructura bitmap
	bitarray = bitarray_create(array, cantBytes);

	// Crea archivo SWAP
	char command[128];
	size_t tamanio_swap = datosSwap->tamanio_pagina * datosSwap->cantidad_paginas;

	snprintf(command, sizeof(command), "dd if=/dev/zero of=%s bs=%zu count=1",datosSwap->nombre_swap,tamanio_swap);

	if(system(command) != -1){
		swapFile = fopen(datosSwap->nombre_swap,"r+");
	}else{
		codRet = -1;
		log_error(logSwap,"No se pudo crear el archivo");
	}

	return codRet;
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
		log_warning(logSwap,"Se ha desconectado la UMC");
		exit(EXIT_FAILURE);
		break;
	case TAMANIO_PAGINA:
		 puts("Proceso recibido");
		 printf("Codigo de operacion: %d\n",paquete->cod_op);
		 printf("Tamano de los datos: %d\n",paquete->tamano_datos);
		 printf("Tamaño de los frames: %d\n",*((int*)paquete->datos));
		 tamanioPagina = *((int*)paquete->datos);
		 // Se sobreescribe el tamaño de página por el que envía la UMC
		 datosSwap->tamanio_pagina = tamanioPagina;
		 // Inicializa archivo Swap
		 inicializaSwapFile(datosSwap);
		 break;
	default:
		manejarOperaciones(paquete);
		break;
	}
}

void manejarOperaciones(t_paquete* paquete){
	usleep((datosSwap->retardo_acceso)*1000);
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

int encontrar_posicion(int pid_enviado, int pagina){
	bool matchPID(void* controlSwap){
		return ((t_control_swap*)controlSwap)->PId == pid_enviado;
	}
	t_control_swap* controlSwap = list_find(lista_procesos,matchPID);
	if(controlSwap != NULL)
			return (controlSwap->posicion+pagina) * tamanioPagina;

	return -1;
}

int grabarArchivo(int posicion_posta, char* buffer){
	fseek(swapFile,posicion_posta,SEEK_SET);

	int codOp = NO_OK;

	if(fwrite(buffer,tamanioPagina,1,swapFile) != 1){
		log_error(logSwap, "No se pudo grabar la página en el archivo");
		//puts("No se pudo grabar la página en el archivo");
	}else{
		codOp = OK;
		log_info(logSwap,"Grabación exitosa de la página en el archivo");
		//puts("Grabación exitosa");
	}

	return codOp;
}

void inicializarNuevoPrograma(t_paquete* paquete){
	log_info(logSwap,"Inicio de proceso de inicialización de un nuevo programa");
	puts("INICIALIZAR NUEVO PROGRAMA");

	//t_pedido_inicializar_swap* pedido = (t_pedido_inicializar_swap*)paquete->datos;

	t_pedido_inicializar_swap* pedido = deserializar_pedido_inicializar_swap(paquete->datos);

	printf("ProcessID: %d\n",pedido->idPrograma);
	printf("Cantidad de Paginas: %d\n",pedido->pagRequeridas);

	//printf("Estado del bitmap antes:\n");
	//log_info(logSwap,"Estado del bitmap antes de inicializar programa");
	loggearBitmap("Estado del bitmap antes de inicializar programa:");

	int cantidadPaginas = pedido->pagRequeridas;

	log_info(logSwap,"Id Proceso: %d",pedido->idPrograma);
	log_info(logSwap,"Cantidad de páginas requeridas: %d",cantidadPaginas);
	log_info(logSwap,"Código recibido: %s",pedido->codigo);

	int codOp = NO_OK;

	if(verificarPaginasLibres(cantidadPaginas) == 0){

		log_info(logSwap,"Hay espacio disponible");
		log_info(logSwap,"Buscando espacio contiguo...");

		int posicion = encontrar_espacio(cantidadPaginas);
		if (posicion == -1){
			log_info(logSwap,"No hay espacio contiguo");
			compactar();
			posicion = encontrar_espacio(cantidadPaginas);
		}

		//si hay lugar armar t_control_swap y sumarlo a la lista de procesos
		agregarNuevoProceso(posicion,cantidadPaginas,pedido);

		// Grabar
		int pagscodigo = (strlen(pedido->codigo)+1)/tamanioPagina;
		int i;
		for(i=0;i<=pagscodigo;i++){
			int posicion_posta = encontrar_posicion(pedido->idPrograma,i);
			char* bufferCorrido = pedido->codigo+(i*tamanioPagina);
			codOp = grabarArchivo(posicion_posta,bufferCorrido);
			if(codOp == NO_OK)
				break;
		}

		//printf("Estado del bitmap despues:\n");
		//log_info(logSwap,"Estado del bitmap después de inicializar programa");
		loggearBitmap("Estado del bitmap después de inicializar programa:");

	}else{
		//fallar
		log_info(logSwap,"No se ha podido asignar el espacio necesario");
		puts("No se ha podido asignar el espacio necesario");
	}

	log_info(logSwap,"Envía respuesta a la UMC");

	enviar(codOp,1,"OK",socket_memoria);
	puts("Envia respuesta a la UMC");
}

void leerPagina(t_paquete* paquete){
	log_info(logSwap,"Inicia proceso de lectura de página");
	puts("LEER PAGINA");

	t_pedido_leer_swap* pedido = (t_pedido_leer_swap*)paquete->datos;

	int pid_enviado = pedido->pid;
	int pagina = pedido->nroPagina;
	char* buffer = malloc(tamanioPagina);


	//Buscar el proceso en la lista
	int codOp = NO_OK;

	int posicion_posta = encontrar_posicion(pid_enviado,pagina);

	if(posicion_posta != -1){
		printf("Posición: %d \n",posicion_posta);
		log_info(logSwap,"Posición: %d \n",posicion_posta);
		fseek(swapFile,posicion_posta,SEEK_SET);

		if(fread(buffer,tamanioPagina,1,swapFile) < 1){
			puts("Error en la lectura de la página");
			log_error(logSwap,"Error en la lectura de la página");
		}else{
			log_info(logSwap,"Buffer\n%.*s",tamanioPagina,buffer);

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
	enviar(codOp,tamanioPagina,buffer,socket_memoria);
}

void escribirPagina(t_paquete* paquete){
	log_info(logSwap,"Inicia proceso de escritura de página");
	puts("ESCRIBE PAGINA");

	t_pedido_almacenar_swap* pedido = deserializar_pedido_almacenar_swap(paquete->datos,tamanioPagina);

	int pid_enviado = pedido->pid;
	int pagina = pedido->nroPagina;
	int posicion_posta = encontrar_posicion(pid_enviado,pagina);

	printf("Posición: %d \n",posicion_posta);
	log_info(logSwap,"pid: %d \n",pid_enviado);
	log_info(logSwap,"Buffer %.*s",tamanioPagina,pedido->buffer);

	// Grabar
	int codOp = grabarArchivo(posicion_posta,pedido->buffer);

	// Responde a la UMC el resultado de la operación
	enviar(codOp,1,&socket_memoria,socket_memoria);

}

void finalizarPrograma(t_paquete* paquete){
	int32_t* pedido = (int32_t*)paquete->datos;
	int pid_enviado = *pedido;

	log_info(logSwap,"Inicia proceso de finalización de programa %d",pid_enviado);
	puts("FINALIZA PROGRAMA");


	int codOp = NO_OK;

	bool matchswap(void* constrolswapentrada){
		return ((t_control_swap*)constrolswapentrada)->PId == pid_enviado;
	}

	t_control_swap* controlSwap = list_remove_by_condition(lista_procesos,matchswap);

	if(controlSwap){

		int posAux = controlSwap->posicion;
		int cantidadPaginas = controlSwap->cantPaginas;

		// Establece como libre los bitmaps que ocupó el proceso
		int pagina_actual;
		for(pagina_actual=0;pagina_actual<cantidadPaginas;pagina_actual++){
			bitarray_clean_bit(bitarray,posAux);
			posAux++;
		}

		//printf("Se limpia el bitmap \n");
		//log_info(logSwap,"Al finalizar el programa se limpia el bitmap");
		loggearBitmap("Al finalizar el programa se limpia el bitmap:");

		free(controlSwap);

		codOp = OK;
		log_info(logSwap,"Finalización del programa exitosa");
		puts("Finalización del programa exitosa");
	}else{
		log_error(logSwap,"La finalización del programa ha fallado");
		puts("La finalización del programa ha fallado");
	}

	// Responde a la UMC el resultado de la operación
	enviar(codOp,1,&socket_memoria,socket_memoria);
}

int verificarPaginasLibres(int cantidadPaginas) {
	// Revisa el bitmap si hay lugar para el proceso
	int paginasPendientes = cantidadPaginas;
	int i, max = datosSwap->cantidad_paginas;
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
	int i, max =  datosSwap->cantidad_paginas;
	for (i = 0; i < max; i++) {
		if (bitarray_test_bit(bitarray, i) == false) {
			int j;
			for (j = 0; j < cantidadPaginas && i+j < max; j++) {
				if (bitarray_test_bit(bitarray, i + j))
					break;
			}
			if (j == cantidadPaginas){
				return i;
			}else{
				break;
			}
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

void compactar(){

	//puts("Bitmap antes de la compactación");
	//log_info(logSwap,"Estado del bitmap antes de la compactación");
	loggearBitmap("Estado del bitmap antes de la compactación:"); // Log del bitmap antes de la compactación

	log_info(logSwap,"Compactando...");
	puts("COMPACTANDO...");

	log_info(logSwap,"Comienza proceso de compactación");
	usleep((datosSwap->retardo_compactacion)*1000); // retardo en microsegundos

	//Ordena lista por posicion
	list_sort(lista_procesos,ordenarPorPosicion);

	int z; //Limpia Bitmap
	int max =  datosSwap->cantidad_paginas;
	for(z=0;z<max;z++){
		bitarray_clean_bit(bitarray,z);
	}

	primerPosicionVacia = 0;
	list_iterate(lista_procesos,moverProcesos);

	//puts("Bitmap luego de la compactación");
	//log_info(logSwap,"Estado del bitmap después de la compactación");
	loggearBitmap("Estado del bitmap después de la compactación:"); // Log del bitmap después de la compactación
	puts("Compactación finalizada");

	log_info(logSwap,"Compactación finalizada");
}

void moverProcesos(void *proceso){
	t_control_swap* procesoACorrer = (t_control_swap*)proceso;

	//Inicializar varaibles
	int posProc = procesoACorrer->posicion;
	int cantPags = procesoACorrer->cantPaginas;
	char* buffer = malloc(tamanioPagina*cantPags);

	//Leer buffer del proceso
	fseek(swapFile,posProc*tamanioPagina,SEEK_SET);
	fread(buffer,tamanioPagina,cantPags,swapFile);

	//Actualiza lista de procesos
	procesoACorrer->posicion = primerPosicionVacia;

	//Actualiza archivo
	fseek(swapFile,primerPosicionVacia*tamanioPagina,SEEK_SET);
	fwrite(buffer,tamanioPagina,cantPags,swapFile);

	free(buffer);

	//Actualiza Bitmap + primerPosicionVacia
	actualizarBitMap(cantPags);
}

int encontrarPrimerVacio(){
	int i, max =  datosSwap->cantidad_paginas;
	for (i = 0; i < max; i++) {
		if (bitarray_test_bit(bitarray, i) == false) {
			return i;
		}
	}
	return -1;
}

t_control_swap* buscarProcesoACorrer(int primerPosicionVacia){
	int i = 0;
	while(!list_is_empty(lista_procesos)){
		t_control_swap* procesoACorrer = malloc(sizeof(t_control_swap));
		procesoACorrer = list_get(lista_procesos,i);
		if(procesoACorrer->posicion>primerPosicionVacia){
			return procesoACorrer;
		}
		i++;
	}
	return NULL;
}

void actualizarBitMap(int cantPags){
	while(cantPags>0){
		bitarray_set_bit(bitarray,primerPosicionVacia);
		primerPosicionVacia++;
		cantPags--;
	}
}

bool ordenarPorPosicion(void *p1, void *p2){
	t_control_swap* proceso1 = (t_control_swap*)p1;
	t_control_swap* proceso2 = (t_control_swap*)p2;

	return (proceso1->posicion)<(proceso2->posicion);
}

void loggearBitmap(char* mensaje){
	int i;
	char bufferBitMap[datosSwap->cantidad_paginas+1]; //sumo 1 para el null
	for(i=0;i<(datosSwap->cantidad_paginas);i++){
		if(bitarray_test_bit(bitarray,i)){
			bufferBitMap[i] = '1';
		}else{
			bufferBitMap[i]= '0';
		}
	}
	bufferBitMap[datosSwap->cantidad_paginas] = '\0';
	//printf("%s\n",buffer2);
	log_info(logSwap,"%s \n %s",mensaje, bufferBitMap);
}
