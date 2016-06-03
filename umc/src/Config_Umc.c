#include "Config_Umc.h"

//------------------------------------------------------------------------------------------------------
//Configuración
//------------------------------------------------------------------------------------------------------

//Defino la función para leer la configuración
t_umcConfig* leerConfiguracion(t_config* config){
	//Asigno memoria para estructura de configuración
	t_umcConfig* datosUmc = (t_umcConfig*)malloc(sizeof(t_umcConfig));

	//Completo los datos de la umc con los valores del archivo de configuración
	datosUmc->puerto = config_get_int_value(config,"PUERTO");
	datosUmc->ip_swap = config_get_string_value(config,"IP_SWAP");
	datosUmc->puerto_swap = config_get_int_value(config,"PUERTO_SWAP");
	datosUmc->cant_marcos = config_get_int_value(config,"CANT_MARCOS");
	datosUmc->marco_size = config_get_int_value(config,"MARCO_SIZE");
	datosUmc->marco_x_proc = config_get_int_value(config,"MARCO_X_PROC");
	datosUmc->tlb_habilitada = config_get_int_value(config,"TLB_HABILITADA");
	datosUmc->entradas_tlb = config_get_int_value(config,"ENTRADAS_TLB");
	datosUmc->retardo = config_get_int_value(config,"RETARDO");

	//Devuelvo la estructura de datos cargados
	return datosUmc;
}

//Función para liberar la memoria pedida para la estructura de configuración
void eliminarConfigUmc(t_umcConfig* datosUmcAEliminar){
	//Si no hay datos de configuración no hace nada
	if(datosUmcAEliminar==NULL){
		return;
	//Si hay datos de configuración libero primero el puntero y luego la estructura
	}else{
		free(datosUmcAEliminar->ip_swap);
		free(datosUmcAEliminar);
	}
}


