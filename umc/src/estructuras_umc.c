#include "estructuras_umc.h"
#include "Config_Umc.h"


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

		//Si la página está en memoria
	}else if(entrada_pag_pedida->presencia==true){

		entrada_pagina=entrada_pag_pedida;

	//Si la página no está en memoria le pido los datos al swap
	}else if(entrada_pag_pedida->presencia==false){

		//Cuento la cantidad de paginas del proceso presentes en memoria
		int paginasUsadas = list_count_satisfying(tablaDePaginas,paginaPresente);

		//Si el proceso tiene la cantidad máxima de frames usados
		if(paginasUsadas == config_umc->marco_x_proc){

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

			entrada_pagina = entrada_pag_pedida;
		}
	}

	return entrada_pagina;
}
//TODO Ver si el programa ya uso la cantidad maxima permitida de frames en bitmap
//TODO Si ya los uso todos se elije una de sus páginas como víctima
//TODO Si la pagina fue modificada se manda al swap y sino se elimina
//TODO Cargar la pagina requerida en el frame que se libero en memoria

//TODO Si todavia no uso todos sus frames ver si hay frames libres en memoria
//TODO Si hay frames libres en memoria se carga la pagina en un frame libre y se setea la misma como usada en bitmap
//TODO Si no hay frames libres en memoria se elije una víctima de entre los frames que tiene el programa
//TODO Si la víctima fue modificada se la manda al swap y sino se elimina
//TODO Cargar la página requerida en el frame que se liberó

//TODO Ver que pasa si no hay espacio y el programa no tiene frames en memoria

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

//Datos que tiene la página en memoria
char* datos_pagina_en_memoria(int marco){
	return (memoria_principal+(marco*config_umc->marco_size));
}

void escribirEnSwap(int pagina,char* datos_pagina,int pid){

	//Armo la estructura para pasarle la pagina al swap
	t_pedido_almacenar_swap pedido_almacenar_swap;
	pedido_almacenar_swap.buffer = datos_pagina;

	pedido_almacenar_swap.pid=pid;
	pedido_almacenar_swap.nroPagina=pagina;

	//Serializo estructura para mandarsela al swap
	t_pedido_almacenar_swap_serializado *pedido_almacenar_swap_serializado;
	pedido_almacenar_swap_serializado=serializar_pedido_almacenar_swap(&pedido_almacenar_swap,config_umc->marco_size);

	//Bloqueo la conexión con el swap
	pthread_mutex_lock(&mutex);

	//Envio el pedido de escritura serializado al swap
	enviar(ESCRITURA_PAGINA,pedido_almacenar_swap_serializado->tamano,pedido_almacenar_swap_serializado->pedido_serializado,socketswap);
	log_info(logUMC,"Se le mando el pedido de escritura al swap");
	log_debug(logUMC,"Pagina a escribir\n%.*s",config_umc->marco_size,pedido_almacenar_swap.buffer);

	//Recibo la respuesta del swap
	t_paquete *paqueteEscritura=recibir_paquete(socketswap);
	log_info(logUMC,"Recibi una respuesta del swap");

	//Desbloqueo la conexión con el swap
	pthread_mutex_unlock(&mutex);

	//Si la respuesta del swap es que se pudo escribir la página devuelvo el paquete
	if(paqueteEscritura->cod_op==OK){

		log_info(logUMC,"Se pudo escribir la pagina");

	//Si la respuesta del swap es que no pudo escribir la página
	}else if(paqueteEscritura->cod_op==NO_OK){

		log_info(logUMC,"No se pudo escribir en la pagina");
		exit(EXIT_FAILURE);

	//Si la respuesta del swap es que se desconecto el socket
	}else if(paqueteEscritura->cod_op==ERROR_COD_OP){

		log_warning(logUMC,"Hubo un error al escribir la página");
		log_warning(logUMC,"Se cerro la conexión con el swap");
		exit(EXIT_FAILURE);

	}

	destruir_paquete(paqueteEscritura);
	free(pedido_almacenar_swap_serializado);
}


char* leerDeSwap(int pid,int pagina){

	char* datos_pagina;

	//Le pido la página al swap
	t_pedido_leer_swap pedido_leer;
	pedido_leer.nroPagina = pagina;
	pedido_leer.pid = pid;
	log_info(logUMC,"Se pidio leer la pagina %d del proceso %d",pagina);

	//Bloqueo la conexión con el swap
	pthread_mutex_lock(&mutex);

	//Le pido la página requerida al swap
	enviar(LECTURA_PAGINA,sizeof(pedido_leer),&pedido_leer,socketswap);
	t_paquete *paquete_lectura = recibir_paquete(socketswap);
	log_info(logUMC,"Recibi el contenido de la pagina del swap");

	//Desbloqueo la conexión con el swap
	pthread_mutex_unlock(&mutex);

	//Si el swap tiene la pagina me la pasa y le paso los bytes pedidos a la cpu
	if(paquete_lectura->cod_op==BUFFER_LEIDO){

		datos_pagina=malloc(paquete_lectura->tamano_datos);
		memcpy(datos_pagina,paquete_lectura->datos,paquete_lectura->tamano_datos);

	//Si el swap no tiene la pagina le aviso a la cpu que hubo un error
	}else if(paquete_lectura->cod_op==NO_OK){

		log_info(logUMC,"No se pudo leer la página");
		exit(EXIT_FAILURE);

	//Si se desconecto el socket
	}else if(paquete_lectura->cod_op==ERROR_COD_OP){

		log_warning(logUMC,"Se desconecto el swap");
		exit(EXIT_FAILURE);
	}

	destruir_paquete(paquete_lectura);

	return datos_pagina;
}


void destruir_lista(void *tablaDePaginas){
	list_destroy_and_destroy_elements(tablaDePaginas,free);
}

void destruir_estructuras(){
	free(memoria_principal);
	dictionary_destroy_and_destroy_elements(tablasDePagina,destruir_lista);

}

