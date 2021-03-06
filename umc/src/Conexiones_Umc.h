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
#include "estructuras_umc.h"
#include "../../general/Operaciones_umc.h"
#include "../../general/operaciones_swap.h"
#include "Config_Umc.h"

//Creo el entero para referenciar al socket del swap
int socketServerPedido, socketswap, socket_nucleo;
char* codigo; //fixme
int d; //fixme

fd_set set_de_fds;

void manejar_paquete(int socket,t_paquete paq);

void cerrar_conexion(int socket);

void nueva_conexion(int socket);

void servidor_pedidos();

char* leerDeSwap(int pid,int pagina);

void escribirEnSwap(int pagina,char* datos_pagina,int pid);

void eliminar_pagina_TLB(int proceso, int pagina);

//mutex para el acceso a la tabla de paginas
pthread_mutex_t mutex_pags;

#endif /* CONEXIONES_UMC_H_ */
