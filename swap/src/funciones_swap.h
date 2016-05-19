#ifndef FUNCIONES_SWAP_H_
#define FUNCIONES_SWAP_H_

#include "estructuras_swap.h"

#include <stdint.h>

#include "../../general/Operaciones_umc.h"
#include "../../general/operaciones_swap.h"

#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "../../sockets/Sockets.h"

t_log* crearLog();
void levantarConfiguracion(t_config*);
void manejar_socket_umc(t_paquete*);
void inicializaSwapFile();
void manejarOperaciones(t_paquete* paquete);
void inicializarNuevoPrograma(t_paquete* paquete);
void leerPagina(t_paquete* paquete);
void escribirPagina(t_paquete* paquete);
void finalizarPrograma(t_paquete* paquete);
int verificarPaginasLibres(int cantidadPaginas);
int encontrar_espacio(int cantidadPaginas);
void agregarNuevoProceso(int posicion,int cantidadPaginas,t_pedido_inicializar_swap* pedido);
void compactar();
int encontrarPrimerVacio();
t_control_swap* buscarProcesoACorrer(int primerPosicionVacia);
void moverProcesos(void *proceso);
void actualizarBitMap(int cantPags);
bool ordenarPorPosicion(void *p1, void *p2);
void loggearBitmap();
void limpiarBitmapAuxiliar();

t_bitarray* bitarray;
t_bitarray* bitarray_aux;
FILE* swapFile;
t_log* logSwap;
t_swapcfg* datosSwap;
int tamanioPagina;
int primerPosicionVacia;

#endif /* FUNCIONES_SWAP_H_ */
