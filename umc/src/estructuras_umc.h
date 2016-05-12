/*
 * estructuras_cpu.h
 *
 *  Created on: 25/4/2016
 *      Author: utnso
 */

#ifndef ESTRUCTURAS_UMC_H_
#define ESTRUCTURAS_UMC_H_
#include <stdbool.h>

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

typedef struct {
	int pid;
	int pagina;
	bool libre;
	bool modificada;
}t_marco;

typedef struct {
	t_marco *marco;
	int posicion;	//posicion de este marco en la memoria_principal
}t_info_marco;

char *memoria_principal;
t_marco *tabla_marcos;
t_info_marco *tlb;

void crear_estructuras();
t_info_marco* obtener_marco(int pid, int pagina);
char* obtener_posicion_real(int pid, int pagina,int offset_en_bytes);
void destruir_estructuras();
char* obtener_posicion_real_desde_marco(t_info_marco* marco_info, int offset_en_bytes);
bool es_el_marco_requerido(t_marco marco, int pagina, int pid);

void destruir_estructura_conexion(int* datosConexion);
#endif /* ESTRUCTURAS_UMC_H_ */
