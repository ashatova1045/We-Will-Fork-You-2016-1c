#include "Log_Umc.h"
#include <commons/string.h>

//------------------------------------------------------------------------------------------------------
//Log
//------------------------------------------------------------------------------------------------------

//Defino la funcion para crear el log general
t_log* crearLog(){
	t_log* logUMC = log_create("logUMC.log","umc.c",false,LOG_LEVEL_DEBUG);
	return logUMC;
}

//Defino la funcion para crear el log de reemplazo para clock
t_log* crearLogReemplazoClock(int proceso){

	char* nombre = string_new();
	string_append(&nombre,"Reemplazos en Proceso ");
	string_append(&nombre,string_itoa(proceso));
	string_append(&nombre,"(Clock)");
	string_append(&nombre, ".log");

	t_log* logReemplazo = log_create(nombre,"umc.c",false,LOG_LEVEL_DEBUG);
	return logReemplazo;
}

//Defino la funcion para crear el log de reemplazo para clockM
t_log* crearLogReemplazoClockM(int proceso){

	char* nombre = string_new();
	string_append(&nombre,"Reemplazos en Proceso ");
	string_append(&nombre,string_itoa(proceso));
	string_append(&nombre,"(ClockM)");
	string_append(&nombre, ".log");

	t_log* logReemplazo = log_create(nombre,"umc.c",false,LOG_LEVEL_DEBUG);
	return logReemplazo;
}

//Defino la funcion para crear el log del comando dump
t_log* crearLogDump(){
	t_log* logDump = log_create("logDump.log","umc.c",true,0);
	return logDump;
}
