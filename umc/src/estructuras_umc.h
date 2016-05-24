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
//t_marco *tabla_marcos;

//Cada proceso tiene su tabla de paginas

void nuevaTablaDePaginas(int pid, int cantPaginas);

void crear_estructuras();
void destruir_estructuras();
void destruir_lista(void *tablaDePaginas);
char* i_to_s(int i);
char* datos_pagina_en_memoria(int marco);
t_entrada_tabla_paginas* buscar_pagina_en_tabla(int pid,int pagina);

void destruir_estructura_conexion(int* datosConexion);
#endif /* ESTRUCTURAS_UMC_H_ */
