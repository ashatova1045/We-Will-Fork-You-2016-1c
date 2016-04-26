/*
 * estructuras_cpu.h
 *
 *  Created on: 25/4/2016
 *      Author: utnso
 */

#ifndef ESTRUCTURAS_UMC_H_
#define ESTRUCTURAS_UMC_H_


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


#endif /* ESTRUCTURAS_UMC_H_ */
