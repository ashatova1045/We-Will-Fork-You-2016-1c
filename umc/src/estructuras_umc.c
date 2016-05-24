#include "estructuras_umc.h"
#include "Config_Umc.h"


//TODO Crear bitmap de memoria principal

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
	if(!tablasDePagina){
		log_error(logUMC,"Error al reservar memoria para el diccionario de la tablas de pagina");
		exit(EXIT_FAILURE);
	}
	log_info(logUMC,"Diccionario de tablas de pagina creado correctamente");

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
t_entrada_tabla_paginas* buscar_pagina_en_tabla(int pid,int pagina){

	//Busca la tabla de paginas en el directorio y la devuelve
	t_list *tablaDePaginas=dictionary_get(tablasDePagina,i_to_s(pid));

	//Busca la página pedida en la lista
	return list_get(tablaDePaginas,pagina);

}

char* datos_pagina_en_memoria(int marco){
	return (memoria_principal+(marco*config_umc->marco_size));
}


void destruir_lista(void *tablaDePaginas){
	list_destroy_and_destroy_elements(tablaDePaginas,free);
}

void destruir_estructuras(){
	free(memoria_principal);
	dictionary_destroy_and_destroy_elements(tablasDePagina,destruir_lista);

}
