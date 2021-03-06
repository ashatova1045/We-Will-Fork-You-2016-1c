#ifndef FUNCIONES_SWAP_H_
#define FUNCIONES_SWAP_H_

#include "estructuras_swap.h"

t_log* crearLog();
void levantarConfiguracion(t_config*);
void manejar_socket_umc(t_paquete*);
int inicializaSwapFile();
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

#endif /* FUNCIONES_SWAP_H_ */
