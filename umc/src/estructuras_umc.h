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
#include <pthread.h>
#include "../../general/funciones_listas.h"

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
	int pid;
	int manecilla;
	t_list* tablaDePaginas;
}t_entrada_diccionario;

typedef struct{
	int32_t nro_marco;
	bool presencia;
	bool modificado;
	bool uso;
}t_entrada_tabla_paginas;

typedef struct{
	int pid;
	int32_t nro_marco;
	bool presencia;
	bool modificado;
	bool uso;
}t_entrada_tlb;

t_dictionary *tablasDePagina;

char *memoria_principal;

t_bitarray *bitmap_frames;
t_list *tlb;
//t_marco *tabla_marcos;

//Cada proceso tiene su tabla de paginas

void nuevaTablaDePaginas(int pid, int cantPaginas);

void crear_estructuras();
void destruir_estructuras();
void destruir_lista(void *tablaDePaginas);
char* i_to_s(int i);
char* datos_pagina_en_memoria(int marco);
t_entrada_tabla_paginas* buscar_pagina_en_tabla(int pid,int pagina);
bool paginaPresente(void* entrada_pag);
//t_entrada_tabla_paginas* elegir_victima(t_list *tablaDePaginas);
//t_entrada_tabla_paginas* reemplazarPagina(int pid,int pagina,t_entrada_tabla_paginas* entrada_pag_pedida,t_list* tablaDePaginas);
t_entrada_tabla_paginas* reemplazarPagina(int pagina,t_entrada_diccionario *entrada_diccionario);
t_entrada_tabla_paginas* elegir_victima_clock(t_entrada_diccionario *entrada_diccionario);
void destruir_estructuras();
void destruir_lista(void *tablaDePaginas);

void destruir_estructura_conexion(int* datosConexion);

//Funciones de bitmap
int verificarFramesLibres(int cantidadFrames);
int cantidadFramesLibres();
int encontrarPrimerVacio();
void usarBitMapDesdePos(int cantFrames, int desdeEstaPosicion);
void limpiarBitMapDesdePos(int cantFrames, int desdeEstaPosicion);
void eliminarPaginas(void *pagina);
void loggearBitmap();

//TLB
void crearTLB(int entradasTLB);
t_entrada_tabla_paginas* buscar_pagina_en_TLB(int32_t proceso, int32_t numeroPagina);
void cargar_en_TLB(int32_t pid, t_entrada_tabla_paginas* pagina);
void eliminar_menos_usado_en_TLB();
#endif /* ESTRUCTURAS_UMC_H_ */
