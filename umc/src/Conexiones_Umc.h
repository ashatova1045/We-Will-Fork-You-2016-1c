/*
 * Conexiones_Umc.h
 *
 *  Created on: 30/4/2016
 *      Author: utnso
 */

#ifndef CONEXIONES_UMC_H_
#define CONEXIONES_UMC_H_

#include <stdio.h>
#include <stdlib.h>
#include "../../sockets/Sockets.h"
#include "Log_Umc.h"
#include "Config_Umc.h"
#include <errno.h>
#include <pthread.h>
#include "../../general/Operaciones_umc.h"
#include "../../general/operaciones_swap.h"

//Creo el entero para referenciar al socket del swap
int socketServerPedido, socketswap;
char* codigo; //fixme
int d; //fixme

fd_set set_de_fds;
//Declaro Semaforos

void manejar_paquete(int socket,t_paquete paq);

void cerrar_conexion(int socket);

void nueva_conexion(int socket);

void servidor_pedidos();

char* leerDeSwap(int pid,int pagina);

void escribirEnSwap(int pagina,char* datos_pagina,int pid);

#endif /* CONEXIONES_UMC_H_ */
