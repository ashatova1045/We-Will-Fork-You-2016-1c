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
	int bytesDeArray = ((config_umc->cant_marcos)/8);

	//Creo el bitmap de frames

	bitmap_frames = bitarray_create(arrayFrames,bytesDeArray);

	int max = bitarray_get_max_bit(bitmap_frames);

	log_info(logUMC,"Bitarray de frames creado correctamente con %d frames",max);

	//Crea la TLB
	if((config_umc->entradas_tlb)>0){
		log_info(logUMC,"Creación de TLB");
		crearTLB(config_umc->entradas_tlb);
	}

}

void crearTLB(int entradasTLB){
	tlb = list_create();
	int i;
	for(i = 0 ; i < entradasTLB ; i++) {
		t_entrada_tlb* entrada = malloc(sizeof(t_entrada_tlb));
		entrada->en_uso = false;
		entrada->nro_marco = -1;
		entrada->pid = -1;
		list_add(tlb, entrada);
	}
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
		entradaTablaPaginas->uso=false;

		//Agrego la entrada a la lista
		list_add(tablaDePaginas,entradaTablaPaginas);

	}
	//memcpy(entrada_diccionario->tablaDePaginas,tablaDePaginas,(list_size(tablaDePaginas)*config_umc->marco_size));
	entrada_diccionario->tablaDePaginas = tablaDePaginas;
	entrada_diccionario->manecilla=0;
	entrada_diccionario->pid=pid;

	//dictionary_put(tablasDePagina,i_to_s(pid),tablaDePaginas);
	dictionary_put(tablasDePagina,i_to_s(pid),entrada_diccionario);
}

//Funcion para buscar una página en la tabla de páginas

t_entrada_tabla_paginas* buscar_pagina_en_tabla(int pid,int pagina){

	t_entrada_tabla_paginas *entrada_pagina;

	//Busco la entrada al diccionario de la página
	t_entrada_diccionario *entrada_diccionario = dictionary_get(tablasDePagina,i_to_s(pid));

	//t_list *tablaDePaginas=dictionary_get(tablasDePagina,i_to_s(pid));
	//t_list *tablaDePaginas=list_create();
	//list_add_all(tablaDePaginas,entrada_diccionario->tablaDePaginas);

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
			//t_entrada_tabla_paginas* entrada_pag_pedida_actualizada = reemplazarPagina(pid,pagina,entrada_pag_pedida,tablaDePaginas);
			t_entrada_tabla_paginas* entrada_pag_pedida_actualizada = reemplazarPagina(pagina,entrada_diccionario);

			entrada_pagina = entrada_pag_pedida_actualizada;

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
				entrada_pag_pedida->nro_marco=frameAOcupar;
				entrada_pag_pedida->presencia=true;

				//Cambio bit de uso porque me la acaban de pedir
				entrada_pag_pedida->uso=true;

				//Marco el frame como ocupado en el bitmap
				bitarray_set_bit(bitmap_frames,frameAOcupar);

				entrada_pagina = entrada_pag_pedida;

				log_info(logUMC,"La página %d del proceso %d ahora está en memoria en el frame %d",pagina,pid,frameAOcupar);
				log_info(logUMC,"El bit de presencia de la pagina %d del proceso %d es %d",pagina,pid,entrada_pagina->presencia);

			//Si no hay frames libres elijo uno de los del proceso
			}else{

				log_info(logUMC,"No quedan frames libres en memoria");

				//Reemplazo una página del proceso
				//t_entrada_tabla_paginas* entrada_pag_pedida_actualizada = reemplazarPagina(pid,pagina,entrada_pag_pedida,tablaDePaginas);

				t_entrada_tabla_paginas* entrada_pag_pedida_actualizada = reemplazarPagina(pagina,entrada_diccionario);

				entrada_pagina = entrada_pag_pedida_actualizada;

			}
		}

	}
	return entrada_pagina;
}

//TODO Ver que pasa si no hay espacio y el programa no tiene frames en memoria

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
	t_entrada_tabla_paginas *entrada_pagina_victima;

	//Mientras no se encuentre una página que sea víctima
	while(encontro_pag_victima==false){

		//Traigo los elementos de la lista(entradas)
		t_entrada_tabla_paginas *entrada_pag_victima = list_get(tablaDePaginas,entrada_diccionario->manecilla);

		//Si la página está presente
		if(entrada_pag_victima->presencia==true){

			//Si la página está en uso
			if(entrada_pag_victima->uso==true){

				entrada_pag_victima->uso=false;
				//log_debug(logUMC,"La página %d del proceso %d ahora tiene el bit de uso %d")

				entrada_diccionario->manecilla+=sizeof(t_entrada_tabla_paginas);

			}else if(entrada_pag_victima->uso==false){

				encontro_pag_victima=true;
				entrada_pagina_victima = entrada_pag_victima;
			}
		}else if(entrada_pag_victima->presencia==false){

			entrada_diccionario->manecilla+=sizeof(t_entrada_tabla_paginas);
		}

		if(entrada_diccionario->manecilla == list_size(tablaDePaginas)){
			entrada_diccionario->manecilla=0;
		}
	}

	return entrada_pagina_victima;
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
	int i, max = bitarray_get_max_bit(bitmap_frames);
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
	int max = bitarray_get_max_bit(bitmap_frames);
	int i, framesLibres = 0;
	for (i = 0; i < max; i++) {
		if (bitarray_test_bit(bitmap_frames, i) == false) {
			framesLibres++;
		}
	}
	return framesLibres;
}

int encontrarPrimerVacio(){
	int i, max = bitarray_get_max_bit(bitmap_frames);
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

	//Elijo una víctima de entre las páginas del proceso
	t_entrada_tabla_paginas *entrada_pag_victima = elegir_victima_clock(entrada_diccionario);


	//Si la página está modificada busco los datos y se los mando al swap
	if(entrada_pag_victima->modificado==true){

		log_info(logUMC,"La página víctima del proceso %d está modificada",entrada_diccionario->pid);

		//Busco los datos en memoria de la página víctima
		char* datos_pagina_victima = datos_pagina_en_memoria(entrada_pag_victima->nro_marco);

		//Busco el id de la página víctima
		int pagina_victima = list_get_index(tablaDePaginas,entrada_pag_victima);

		log_info(logUMC,"Se eligio como víctima la página %d del proceso %d",pagina_victima,entrada_diccionario->pid);

		//Escribo la página víctima en memoria
		escribirEnSwap(pagina_victima,datos_pagina_victima,entrada_diccionario->pid);
	}

	//Le pido al swap la página
	char* datos_pagina = leerDeSwap(entrada_diccionario->pid,pagina);

	//Guardo la nueva página en la memoria
	char* espacioEnMemoria = (memoria_principal+(entrada_pag_victima->nro_marco*config_umc->marco_size));

	memcpy(espacioEnMemoria,datos_pagina,config_umc->marco_size);

	//Busca la página pedida en la lista
	t_entrada_tabla_paginas *entrada_pag_pedida = list_get(tablaDePaginas,pagina);

	//Actualizo la entrada a la tabla de la página
	entrada_pag_pedida->nro_marco=entrada_pag_victima->nro_marco;
	entrada_pag_pedida->presencia=true;

	//Cambio bit de uso porque la acabo de cargar
	entrada_pag_pedida->uso=true;

	//Cambio el bit de presencia y limpio el frame de la página que acabo de sacar
	entrada_pag_victima->presencia=false;
	entrada_pag_victima->nro_marco=-1;

	log_info(logUMC,"Ahora la página %d del proceso %d está en memoria en el frame %d",pagina,entrada_diccionario->pid,entrada_pag_pedida->nro_marco);
	log_debug(logUMC,"El bit de presencia de la pagina victima del proceso %d ahora es %d",entrada_diccionario->pid,entrada_pag_victima->presencia);
	log_debug(logUMC,"El numero de frame de la página víctima del proceso %d es %d",entrada_diccionario->pid,entrada_pag_victima->nro_marco);
	log_debug(logUMC,"El bit de presencia de la página %d del proceso %d es %d",pagina,entrada_diccionario->pid,entrada_pag_pedida->presencia);

	return entrada_pag_pedida;

}

