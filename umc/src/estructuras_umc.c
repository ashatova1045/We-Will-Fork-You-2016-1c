#include "estructuras_umc.h"
#include "Config_Umc.h"
#include "Conexiones_Umc.h"

void crear_estructuras(){
	//creo la memoria_principal
	int espacio_total = config_umc->cant_marcos*config_umc->marco_size;
	memoria_principal = malloc(espacio_total);
	if(!memoria_principal){
		log_error(logUMC,"Error al reservar memoria para la memoria principal");
		exit(EXIT_FAILURE);
	}
	log_info(logUMC,"Memoria principal creada. %d marcos * %d bytes = %d bytes",config_umc->cant_marcos,config_umc->marco_size,espacio_total);

	//Creo el diccionario de tablas de paginas
	tablasDePagina = dictionary_create();

	log_info(logUMC,"Diccionario de tablas de pagina creado correctamente");

	//Inicializo variables del bitarray
	char* arrayFrames = malloc(config_umc->cant_marcos);
	int bytesDeArray = ((config_umc->cant_marcos)/8)+1;

	//Creo el bitmap de frames
	bitmap_frames = bitarray_create(arrayFrames,bytesDeArray);

	int max = config_umc->cant_marcos;

	log_info(logUMC,"Bitarray de frames creado correctamente con %d frames",max);

	//Crea la TLB
	if(config_umc->entradas_tlb){
		log_info(logUMC,"Creación de TLB");
		crearTLB(config_umc->entradas_tlb);
	}
}

//Función para crear la TLB
void crearTLB(int entradasTLB){
	tlb = list_create();
}

char* i_to_s(int i){
	    int cant_chars = 3;
	    char *str= malloc(sizeof(char)*cant_chars);
	    snprintf(str, cant_chars, "%d", i);
	    return str;
}

// Creo la tabla con las paginas de un proceso en particular
void nuevaTablaDePaginas(int pid,int cantPaginas){

	//Declaro la tabla de páginas como una lista
	t_list *tablaDePaginas =list_create();

	//Reservo memoria para la entrada de diccionario
	t_entrada_diccionario *entrada_diccionario=malloc(sizeof(t_entrada_diccionario));
	int i;

	//Mientras el programa tenga paginas
	for(i=0;i<cantPaginas;i++){

		//Reservo memoria para la entrada de tabla de paginas
		t_entrada_tabla_paginas *entradaTablaPaginas=malloc(sizeof(t_entrada_tabla_paginas));

		//Completo los datos de inicializacion de la entrada de la tabla de paginas
		entradaTablaPaginas->nro_marco=-1;
		entradaTablaPaginas->presencia=false;
		entradaTablaPaginas->modificado=false;
		entradaTablaPaginas->uso=false;		//Agrego la entrada a la lista
		list_add(tablaDePaginas,entradaTablaPaginas);

	}
	entrada_diccionario->tablaDePaginas = tablaDePaginas;
	entrada_diccionario->manecilla=0;
	entrada_diccionario->pid=pid;

	dictionary_put(tablasDePagina,i_to_s(pid),entrada_diccionario);
}

//Funcion para buscar una página en la tabla de páginas
t_entrada_tabla_paginas* buscar_pagina_en_tabla(int pid,int pagina){

	t_entrada_tabla_paginas *entrada_pagina;

	//Protejo con un semáforo el acceso a la tabla de paginas
	pthread_mutex_lock(&mutex_pags);

	//Busco la entrada al diccionario de la página
	t_entrada_diccionario *entrada_diccionario = dictionary_get(tablasDePagina,i_to_s(pid));

	pthread_mutex_unlock(&mutex_pags);


	//Busco la tabla de páginas del proceso
	t_list *tablaDePaginas=entrada_diccionario->tablaDePaginas;

	//Busca la página pedida en la lista
	t_entrada_tabla_paginas *entrada_pag_pedida = list_get(tablaDePaginas,pagina);

	//Si la pagina no esta en la tabla es un error
	if(entrada_pag_pedida==NULL){

		log_warning(logUMC,"No se encontro la pagina %d del proceso %d",pagina,pid);
		exit(EXIT_FAILURE);

	//Si la página está en memoria la devuelvo
	}else if(entrada_pag_pedida->presencia==true){

		//Actualizo el bit de uso de la página porque me la acaban de pedir
		entrada_pag_pedida->uso=true;

		entrada_pagina=entrada_pag_pedida;

		log_info(logUMC,"La página %d del proceso %d estaba en memoria",pagina,pid);

	//Si la página no está en memoria le pido los datos al swap
	}else if(entrada_pag_pedida->presencia==false){

		log_info(logUMC,"La página %d del proceso %d no estaba en memoria",pagina,pid);

		//Cuento la cantidad de paginas del proceso presentes en memoria
		int paginasUsadas = list_count_satisfying(entrada_diccionario->tablaDePaginas,paginaPresente);

		log_info(logUMC,"El proceso %d va usando %d frames",pid,paginasUsadas);

		//Si el proceso tiene la cantidad máxima de frames usados
		if(paginasUsadas == config_umc->marco_x_proc){

			log_info(logUMC,"El proceso %d uso la máxima cantidad de frames permitida",pid);

			//Reemplazo una página del proceso
			t_entrada_tabla_paginas* entrada_pag_pedida_actualizada = reemplazarPagina(pagina,entrada_diccionario);

			entrada_pagina = entrada_pag_pedida_actualizada;
			log_info(logUMC,"entrada_pag_pedida_actualizada PID: %d",entrada_diccionario->pid);

		//Si el proceso no pidió la cantidad máxima de frames
		}else if(paginasUsadas<config_umc->marco_x_proc){

			log_info(logUMC,"El proceso %d aun no pidio el máximo número de frames permitidos",pid);

			//Busco la cantidad de frames
			int framesLibres = cantidadFramesLibres();

			//Si hay frames libres
			if(framesLibres>0){

				log_info(logUMC,"Todavía quedan frames libres en memoria");

				//Traigo el primer frame libre
				int frameAOcupar = encontrarPrimerVacio();

				log_debug(logUMC,"Se va a ocupar el frame %d",frameAOcupar);

				//Le pido al swap la página
				char* datos_pagina = leerDeSwap(pid,pagina);

				//Guardo la nueva página en memoria
				char* espacioEnMemoria = (memoria_principal+(frameAOcupar*config_umc->marco_size));
				memcpy(espacioEnMemoria,datos_pagina,config_umc->marco_size);

				//Actualizo la entrada a la tabla de la página
				log_info(logUMC,"frameAOcupar: %d",frameAOcupar);
				entrada_pag_pedida->nro_marco=frameAOcupar;
				entrada_pag_pedida->presencia=true;

				//Cambio bit de uso porque me la acaban de pedir
				entrada_pag_pedida->uso=true;

				//Marco el frame como ocupado en el bitmap
				bitarray_set_bit(bitmap_frames,frameAOcupar);

				entrada_pagina = entrada_pag_pedida;

				log_info(logUMC,"La página %d del proceso %d ahora está en memoria en el frame %d",pagina,pid,frameAOcupar);
				log_info(logUMC,"El bit de presencia de la pagina %d del proceso %d es %d",pagina,pid,entrada_pagina->presencia);

			//Si no hay frames libres y el proceso tiene frames usados
			}else if(paginasUsadas>0){

				log_info(logUMC,"No quedan frames libres en memoria, uso una del proceso %d",pid);

				//Reemplazo una página del proceso
				t_entrada_tabla_paginas* entrada_pag_pedida_actualizada = reemplazarPagina(pagina,entrada_diccionario);

				entrada_pagina = entrada_pag_pedida_actualizada;

			//Si no hay frames libres y el proceso no usó ninguno
			}else entrada_pagina=NULL;
		}

	}
	return entrada_pagina;
}

//Función para buscar una página en la TLB
t_entrada_tabla_paginas* buscar_pagina_en_TLB(int32_t proceso, int32_t nro_pagina){
	int i;
	for(i=0;i<list_size(tlb);i++){

		t_entrada_tlb* elementoLista = list_get(tlb,i);

		if(elementoLista->pid == proceso && elementoLista->nroPagina == nro_pagina){
			list_add(tlb,list_remove(tlb,i)); //LRU

			return elementoLista->pagina;
		}

	}
	return NULL;
}

//Cargar una entrada en la TLB
void cargar_en_TLB(int32_t pid, int nroPagina, t_entrada_tabla_paginas* pagina){
	log_info(logUMC,"Se carga la página al TLB");

	t_entrada_tlb* entrada_tlb = malloc(sizeof(t_entrada_tlb));
	entrada_tlb->pid = pid;
	entrada_tlb->nroPagina = nroPagina;
	entrada_tlb->pagina = pagina;

	list_add(tlb,entrada_tlb);
}

void eliminarPaginasEnTLB(int32_t pid){
	log_info(logUMC,"Se borran la páginas en la TLB");

	int i;
	for(i=0;i<list_size(tlb);i++){
		t_entrada_tlb* entrada_tlb = list_get(tlb,i);

		if(entrada_tlb->pid == pid){
			free(list_remove(tlb,i));
		}
	}
}

void eliminar_menos_usado_en_TLB(){
	free(list_remove(tlb,0));
}

//Función para conseguir los datos que tiene una página en memoria
char* datos_pagina_en_memoria(int marco){
	return (memoria_principal+(marco*config_umc->marco_size));
}


//Funcion para saber si una página esta presente en memoria o no
bool paginaPresente(void* entrada_pag){
	t_entrada_tabla_paginas *entrada = (t_entrada_tabla_paginas*)entrada_pag;
	return entrada->presencia;
}

//Funcion para elegir una víctima a mandar al swap
t_entrada_tabla_paginas* elegir_victima_clock(t_entrada_diccionario *entrada_diccionario){

	//Traigo la tabla de páginas del proceso
	t_list *tablaDePaginas=entrada_diccionario->tablaDePaginas;
	log_info(logUMC,"Se consiguió la tabla de páginas del proceso %d",entrada_diccionario->pid);


	bool encontro_pag_victima = false;
	t_entrada_tabla_paginas *entrada_pag_victima;

	//Mientras no se encuentre una página que sea víctima
	while(encontro_pag_victima==false){

		//Traigo los elementos de la lista(entradas)
		entrada_pag_victima = list_get(tablaDePaginas,entrada_diccionario->manecilla);

		//Si la página está en uso la marco como que ya no está en uso
		if(entrada_pag_victima->presencia){

			if(entrada_pag_victima->uso){

				entrada_pag_victima->uso=false;

			}else encontro_pag_victima=true;
		}


		/*if(entrada_pag_victima->uso && entrada_pag_victima->presencia){
			entrada_pag_victima->uso=false;

		//Si la página no está en uso devuelvo esa entrada
		}else{
			encontro_pag_victima=true;
		}*/

		incrementarManecilla(entrada_diccionario,tablaDePaginas);
	}

	return entrada_pag_victima;
}

t_entrada_tabla_paginas *elegir_victima_clock_m(t_entrada_diccionario *entrada_diccionario){

	//Traigo la tabla de páginas del proceso
	t_list *tablaDePaginas=entrada_diccionario->tablaDePaginas;
	log_info(logUMC,"Se consiguió la tabla de páginas del proceso %d",entrada_diccionario->pid);

	bool encontro_pag_victima = false;
	t_entrada_tabla_paginas *entrada_pag_victima;

	//Mientras no se encuentre una página que sea víctima
	while(encontro_pag_victima==false){

		//Traigo los elementos de la lista(entradas)
		int cantidad_entradas;

		//Recorro la lista buscando una página que se esté en uso ni modificada
		for(cantidad_entradas = 0;cantidad_entradas<list_size(tablaDePaginas);cantidad_entradas++){

			entrada_pag_victima = list_get(tablaDePaginas,entrada_diccionario->manecilla);

			//Si la página no está en uso ni modificada
			if(!entrada_pag_victima->uso && !entrada_pag_victima->modificado && entrada_pag_victima->presencia){

				encontro_pag_victima = true;
				break;

			}
		incrementarManecilla(entrada_diccionario,tablaDePaginas);
		}

		//Si encontró la página víctima en el primer for sale del while
		if(encontro_pag_victima) break;

		//Recorro la lista una página que no se haya usado pero esté modificada
		for(cantidad_entradas = 0;cantidad_entradas<list_size(tablaDePaginas);cantidad_entradas++){

			entrada_pag_victima = list_get(tablaDePaginas,entrada_diccionario->manecilla);

			//Si la página no está en uso pero si modificada
			if(!entrada_pag_victima->uso && entrada_pag_victima->modificado && entrada_pag_victima->presencia){

				encontro_pag_victima = true;
				break;

			}
			entrada_pag_victima->uso=false;

			incrementarManecilla(entrada_diccionario,tablaDePaginas);
		}

	}
	return entrada_pag_victima;
}

//Función para incrementar la manecilla
void incrementarManecilla(t_entrada_diccionario *entrada_diccionario, t_list* tablaDePaginas){
	entrada_diccionario->manecilla++;

	//Si está al final de la lista vuelve a empezar
	if(entrada_diccionario->manecilla==list_size(tablaDePaginas)){
		entrada_diccionario->manecilla=0;
	}
}

void destruir_lista(void *tablaDePaginas){
	list_destroy_and_destroy_elements(tablaDePaginas,free);
}

void destruir_estructuras(){
	free(memoria_principal);
	dictionary_destroy_and_destroy_elements(tablasDePagina,destruir_lista);
}

int verificarFramesLibres(int cantidadFrames) {
	// Revisa el bitmap si hay lugar para el proceso
	int paginasPendientes = cantidadFrames;
	int i, max = config_umc->cant_marcos;
	for (i = 0; i < max; i++) {
		if (bitarray_test_bit(bitmap_frames, i) == false) {
			paginasPendientes--;
			if(paginasPendientes == 0){
				return 0;
			}
		}
	}
	return paginasPendientes;
}

int cantidadFramesLibres() {
	int max = config_umc->cant_marcos;
	int i, framesLibres = 0;
	for (i = 0; i < max; i++) {
		if (bitarray_test_bit(bitmap_frames, i) == false) {
			framesLibres++;
		}
	}
	return framesLibres;
}

int encontrarPrimerVacio(){
	int i, max = config_umc->cant_marcos;
	for (i = 0; i < max; i++) {
		if (bitarray_test_bit(bitmap_frames, i) == false) {
			return i;
		}
	}
	return -1;
}

void usarBitMapDesdePos(int cantFrames, int desdeEstaPosicion){
	int posicionAux = desdeEstaPosicion;
	while(cantFrames>0){
		bitarray_set_bit(bitmap_frames,posicionAux);
		posicionAux++;
		cantFrames--;
	}
}

void limpiarBitMapDesdePos(int cantFrames, int desdeEstaPosicion){
	int posicionAux = desdeEstaPosicion;
	while(cantFrames>0){
		bitarray_clean_bit(bitmap_frames,posicionAux);
		posicionAux++;
		cantFrames--;
	}
}

void eliminarPaginas(void *pagina){
	t_entrada_tabla_paginas* paginaABorrar = (t_entrada_tabla_paginas*)pagina;

	//Inicializar varaibles
	int32_t nro_marco = paginaABorrar->nro_marco;
	log_info(logUMC,"Eliminar Páginas - Nro de marco: %d",nro_marco);

	bitarray_clean_bit(bitmap_frames,nro_marco);

	free(pagina);
}

void loggearBitmap(){
	int i;
	for(i=0;i<(config_umc->cant_marcos);i++){
		if(bitarray_test_bit(bitmap_frames,i)){
			printf("1");
		}else{
			printf("0");
		}
	}
	printf("\n");
}

//Funcion para reemplazar una página del proceso

t_entrada_tabla_paginas* reemplazarPagina(int pagina,t_entrada_diccionario *entrada_diccionario){

	//t_list *tablaDePaginas=list_create();
	//list_add_all(tablaDePaginas,entrada_diccionario->tablaDePaginas);
	t_list *tablaDePaginas=entrada_diccionario->tablaDePaginas;

	t_entrada_tabla_paginas *entrada_pag_victima;

	if(config_umc->esClockM){
		entrada_pag_victima = elegir_victima_clock_m(entrada_diccionario);
	}else entrada_pag_victima = elegir_victima_clock(entrada_diccionario);

	//Busco el id de la página víctima
	int pagina_victima = list_get_index(tablaDePaginas,entrada_pag_victima);

	//Si la página está modificada busco los datos y se los mando al swap
	if(entrada_pag_victima->modificado==true){

		log_info(logUMC,"La página víctima del proceso %d está modificada",entrada_diccionario->pid);

		//Busco los datos en memoria de la página víctima
		log_info(logUMC,"Página Víctima - Nro de marco: %d",entrada_pag_victima->nro_marco);
		char* datos_pagina_victima = datos_pagina_en_memoria(entrada_pag_victima->nro_marco);

		//Busco el id de la página víctima
		//int pagina_victima = list_get_index(tablaDePaginas,entrada_pag_victima);

		//Escribo la página víctima en memoria
		escribirEnSwap(pagina_victima,datos_pagina_victima,entrada_diccionario->pid);
	}
	log_info(logUMC,"Se eligio como víctima la página %d del proceso %d",pagina_victima,entrada_diccionario->pid);

	//Le pido al swap la página
	char* datos_pagina = leerDeSwap(entrada_diccionario->pid,pagina);

	//Guardo la nueva página en la memoria
	log_info(logUMC,"Guarda nueva página - entrada víctima - Nro de marco: %d",entrada_pag_victima->nro_marco);
	char* espacioEnMemoria = (memoria_principal+(entrada_pag_victima->nro_marco*config_umc->marco_size));

	memcpy(espacioEnMemoria,datos_pagina,config_umc->marco_size);

	//Busca la página pedida en la lista
	t_entrada_tabla_paginas *entrada_pag_pedida = list_get(tablaDePaginas,pagina);

	//Actualizo la entrada a la tabla de la página
	log_info(logUMC,"Actualiza Páginas - entrada_pag_pedida - Nro de marco: %d",entrada_pag_victima->nro_marco);
	entrada_pag_pedida->nro_marco=entrada_pag_victima->nro_marco;
	entrada_pag_pedida->presencia=true;
	entrada_pag_pedida->modificado=false;

	//Cambio bit de uso porque la acabo de cargar
	entrada_pag_pedida->uso=true;

	//Cambio el bit de presencia y limpio el frame de la página que acabo de sacar
	entrada_pag_victima->presencia=false;
	entrada_pag_victima->nro_marco=-1;

	log_info(logUMC,"Ahora la página %d del proceso %d está en memoria en el frame %d",pagina,entrada_diccionario->pid,entrada_pag_pedida->nro_marco);
	log_debug(logUMC,"El bit de presencia de la pagina victima del proceso %d ahora es %d",entrada_diccionario->pid,entrada_pag_victima->presencia);
	log_debug(logUMC,"El numero de frame de la página víctima del proceso %d es %d",entrada_diccionario->pid,entrada_pag_victima->nro_marco);
	log_debug(logUMC,"El bit de presencia de la página %d del proceso %d es %d",pagina,entrada_diccionario->pid,entrada_pag_pedida->presencia);

	eliminar_pagina_TLB(entrada_diccionario->pid,pagina_victima);

	return entrada_pag_pedida;

}
