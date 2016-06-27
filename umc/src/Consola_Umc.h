#ifndef CONSOLA_UMC_H_
#define CONSOLA_UMC_H_
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>//Incluyo commons
#include <commons/string.h>//Incluyo funciones de strings

void imprimirMenu();

void mostrarOpciones();

void ingresarRetardo();

void rEstructurasDeMemoriaTodas();

void rEstructuraDeMemoriaProceso();

void rDatosMemoriaTodos();

void rDatosMemoriaProceso();

void reportes();

void limpiarTLB();

void marcarPaginas();

void ejecutoConsola();

void generarReporteDeProceso(int idProceso);

void generarReporteDeDatosProceso(int idProceso);

void generarReporteDeProceso(int idProceso);

//mutex para el acceso a la variable de retardo
pthread_mutex_t mutex_retardo;

#endif /* CONSOLA_UMC_H_ */
