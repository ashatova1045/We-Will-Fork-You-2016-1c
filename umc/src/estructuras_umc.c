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

}

char* i_to_s(int i){
	    int cant_chars = 3;
	    char *str= malloc(sizeof(char)*cant_chars);
	    snprintf(str, cant_chars, "%d", i);
	    return str;
}

// Creo la tabla con las paginas de un proceso en particular

void nuevaTablaDePaginas(int pid,int cantPaginas){
	t_list *tablaDePaginas =list_create();
	int i;

	//Mientras el programa tenga paginas
	for(i=0;i<cantPaginas;i++){
		//Reservo memoria para la entrada de tabla de paginas
		t_entrada_tabla_paginas *entradaTablaPaginas=malloc(sizeof(t_entrada_tabla_paginas));

		//Completo los datos de inicializacion de la entrada de la tabla de paginas
		entradaTablaPaginas->nro_marco=-1;
		entradaTablaPaginas->presencia=false;
		entradaTablaPaginas->modificado=false;

		//Agrego la entrada a la lista
		list_add(tablaDePaginas,entradaTablaPaginas);
	}

	dictionary_put(tablasDePagina,i_to_s(pid),tablaDePaginas);
}

//Funcion para buscar una página en la tabla de páginas

t_entrada_tabla_paginas* buscar_pagina_en_tabla(int pid,int pagina){

	t_entrada_tabla_paginas *entrada_pagina;

	//Busca la tabla de paginas en el directorio y la devuelve
	t_list *tablaDePaginas=dictionary_get(tablasDePagina,i_to_s(pid));

	//Busca la página pedida en la lista
	t_entrada_tabla_paginas *entrada_pag_pedida = list_get(tablaDePaginas,pagina);

	//Si la pagina no esta en la tabla es un error
	if(entrada_pag_pedida==NULL){

		log_warning(logUMC,"No se encontro la pagina %d",pagina);
		exit(EXIT_FAILURE);

	//Si la página está en memoria la devuelvo
	}else if(entrada_pag_pedida->presencia==true){

		entrada_pagina=entrada_pag_pedida;

	//Si la página no está en memoria le pido los datos al swap
	}else if(entrada_pag_pedida->presencia==false){

		//Cuento la cantidad de paginas del proceso presentes en memoria
		int paginasUsadas = list_count_satisfying(tablaDePaginas,paginaPresente);

		//Si el proceso tiene la cantidad máxima de frames usados
		if(paginasUsadas == config_umc->marco_x_proc){

			//Reemplazo una página del proceso
			t_entrada_tabla_paginas* entrada_pag_pedida_actualizada = reemplazarPagina(pid,pagina,entrada_pag_pedida,tablaDePaginas);

			entrada_pagina = entrada_pag_pedida_actualizada;

		//Si el proceso no pidió la cantidad máxima de frames
		}else if(paginasUsadas<config_umc->marco_x_proc){

			//Busco la cantidad de frames
			int framesLibres = cantidadFramesLibres();

			//Si hay frames libres
			if(framesLibres>0){

				//Traigo el primer frame libre
				int frameAOcupar = encontrarPrimerVacio();

				//Le pido al swap la página
				char* datos_pagina = leerDeSwap(pid,pagina);

				//Guardo la nueva página en memoria
				char* espacioEnMemoria = (memoria_principal+(frameAOcupar*config_umc->marco_size));

				memcpy(espacioEnMemoria,datos_pagina,config_umc->marco_size);

				//Actualizo la entrada a la tabla de la página
				entrada_pag_pedida->nro_marco=frameAOcupar;
				entrada_pag_pedida->presencia=true;

				//Marco el frame como ocupado en el bitmap
				bitarray_set_bit(bitmap_frames,frameAOcupar);

				entrada_pagina = entrada_pag_pedida;

			//Si no hay frames libres elijo uno de los del proceso
			}else{

				//Reemplazo una página del proceso
				t_entrada_tabla_paginas* entrada_pag_pedida_actualizada = reemplazarPagina(pid,pagina,entrada_pag_pedida,tablaDePaginas);

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
t_entrada_tabla_paginas* elegir_victima(t_list *tablaDePaginas){
	t_entrada_tabla_paginas *entrada_pag_victima =list_find(tablaDePaginas,paginaPresente);
	return entrada_pag_victima;

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

//Funcion para reemplazar una página del proceso

t_entrada_tabla_paginas* reemplazarPagina(int pid,int pagina,t_entrada_tabla_paginas* entrada_pag_pedida,t_list* tablaDePaginas){

	//Elijo una víctima de entre las páginas del proceso
	t_entrada_tabla_paginas *entrada_pag_victima = elegir_victima(tablaDePaginas);

	//Si la página está modificada busco los datos y se los mando al swap
	if(entrada_pag_victima->modificado==true){

		//Busco los datos en memoria de la página víctima
		char* datos_pagina_victima = datos_pagina_en_memoria(entrada_pag_victima->nro_marco);

		//Busco el id de la página víctima
		int pagina_victima = list_get_index(tablaDePaginas,entrada_pag_victima);

		//Escribo la página víctima en memoria
		escribirEnSwap(pagina_victima,datos_pagina_victima,pid);
	}

	//Le pido al swap la página
	char* datos_pagina = leerDeSwap(pid,pagina);

	//Guardo la nueva página en la memoria
	char* espacioEnMemoria = (memoria_principal+(entrada_pag_victima->nro_marco*config_umc->marco_size));

	memcpy(espacioEnMemoria,datos_pagina,config_umc->marco_size);

	//Actualizo la entrada a la tabla de la página
	entrada_pag_pedida->nro_marco=entrada_pag_victima->nro_marco;
	entrada_pag_pedida->presencia=true;

	//Cambio el bit de presencia y limpio el frame de la página que acabo de sacar
	entrada_pag_victima->presencia=false;
	entrada_pag_victima->nro_marco=-1;

	return entrada_pag_pedida;

}

