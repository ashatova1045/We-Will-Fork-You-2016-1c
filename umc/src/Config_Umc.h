#ifndef CONFIG_UMC_H_
#define CONFIG_UMC_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>//Incluyo funciones de strings
#include "estructuras_umc.h"
#include <semaphore.h>

//Declaro la estructura de configuración
t_umcConfig* config_umc;

t_umcConfig* leerConfiguracion(t_config* config);

//mutex para el acceso a los pedidos nuevos
//esto es para que solo corra la config O los pedidos, nunca los 2
sem_t programasquepuedencorrer;
int* cant_programas_conectados;

void eliminarConfigUmc(t_umcConfig* datosUmcAEliminar);


#endif /* CONFIG_UMC_H_ */
