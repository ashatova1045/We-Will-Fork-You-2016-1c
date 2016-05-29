/*
 * estructuras_cpu.h
 *
 *  Created on: 25/4/2016
 *      Author: utnso
 */

#ifndef ESTRUCTURAS_UMC_H_
#define ESTRUCTURAS_UMC_H_
#include <stdbool.h>
#include <commons/collections/list.h>
#include "../../sockets/Sockets.h"
#include <stdlib.h>
#include <commons/collections/dictionary.h>
#include "Log_Umc.h"
#include <commons/bitarray.h>
#include "../../general/operaciones_swap.h"
#include "../../general/funciones_listas.h"
#include "Conexiones_Umc.h"
#include <pthread.h>

//Creo la estructura de configuración
typedef struct{
	int puerto;
	char* ip_swap;
	int puerto_swap;
	int cant_marcos;
	int marco_size;
	int marco_x_proc;
	int entradas_tlb;
	int retardo;
}t_umcConfig;


typedef struct{
	int32_t nro_marco;
	bool presencia;
	bool modificado;
}t_entrada_tabla_paginas;

t_dictionary *tablasDePagina;

char *memoria_principal;

t_bitarray *bitmap_frames;

//Declaro Semaforos

//mutex para la comunicación con el swap
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//mutex para el acceso a la tabla de paginas
pthread_mutex_t mutex_pags=PTHREAD_MUTEX_INITIALIZER;

//Cada proceso tiene su tabla de paginas

void nuevaTablaDePaginas(int pid, int cantPaginas);

void crear_estructuras();
void destruir_estructuras();
void destruir_lista(void *tablaDePaginas);
char* i_to_s(int i);
char* datos_pagina_en_memoria(int marco);
t_entrada_tabla_paginas* buscar_pagina_en_tabla(int pid,int pagina);
bool paginaPresente(void* entrada_pag);
t_entrada_tabla_paginas* elegir_victima(t_list *tablaDePaginas);
void escribirEnSwap(int pagina,char* datos_pagina,int pid);
char* leerDeSwap(int pid,int pagina);

void destruir_estructura_conexion(int* datosConexion);
#endif /* ESTRUCTURAS_UMC_H_ */
