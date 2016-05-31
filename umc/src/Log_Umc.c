#include "Log_Umc.h"

//------------------------------------------------------------------------------------------------------
//Log
//------------------------------------------------------------------------------------------------------

//Defino la funcion para crear el log
t_log* crearLog(){
	t_log* logUMC = log_create("logUMC.log","umc.c",false,LOG_LEVEL_DEBUG);
	return logUMC;
}

