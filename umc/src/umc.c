//------------------------------------------------------------------------------------------------------
//Includes
//------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <commons/log.h>//Incluyo commons
#include <commons/string.h>//Incluyo funciones de strings
#include "../../sockets/Sockets.h"
#include <stdbool.h>
#include <unistd.h>
#include <commons/config.h>
#include <errno.h>

#include "estructuras_umc.h"
#include "Log_Umc.h"
#include "Config_Umc.h"
#include "Conexiones_Umc.h"
#include "Consola_Umc.h"

//------------------------------------------------------------------------------------------------------

int main(int argc, char **argv){


	//Creo el archivo log
	logUMC = crearLog();
	if(logUMC==NULL){
		log_error(logUMC,"No se pudo crear el log de la umc");
	}
	log_info(logUMC,"Ejecución del proceso UMC");

	//Creo el archivo de configuración
	t_config* config = config_create("../umc/umc.cfg");
	if(config==NULL){
	log_error(logUMC,"No se pudo crear la config de la umc");
	}else{
		log_info(logUMC,"Se creo el config de la umc");
	}

	//Leo la configuración de la memoria
	config_umc = leerConfiguracion(config);
	log_debug(logUMC,"Puerto de conexion %d\n",config_umc->puerto);

	log_info(logUMC,"Se cargo la configuracion");

	//Creo las estructuras de memoria
	crear_estructuras();

	//Me conecto al área de swap y hago handshake
	socketswap = conectar(config_umc->ip_swap,config_umc->puerto_swap);
	if(socketswap == -1){
		log_error(logUMC,"No se pudo conectar");
		exit(EXIT_FAILURE);
	}else{
		log_info(logUMC,"Conectado al swap");
	}


	if(handshake(socketswap,HS_UMC_SWAP,OK_HS_UMC) ==-1 ){
		log_error(logUMC,"Swap no respondio al handshake");
		exit(EXIT_FAILURE);
	}else{
		log_info(logUMC,"Handshake Swap correcto");
	}

	int32_t frames= config_umc->marco_size;
	enviar(TAMANIO_PAGINA,sizeof(int32_t),&frames,socketswap);
	log_info(logUMC,"Se envió el tamaño de la página al swap");


	mutex_pags = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	nuevos_pedidos = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

	cant_pedidos_corriendo = malloc(sizeof(int));
	*cant_pedidos_corriendo = 0;

	//Defino el hilo para el socket servidor
	pthread_t  pedidosThread;

	//Creo el hilo de pedidos con formato datachable
	log_debug(logUMC,"Creando el hilo para recibir pedidos");

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	if(pthread_create(&pedidosThread,&attr,(void*)servidor_pedidos,NULL)){
		log_error(logUMC,"Error al crear el hilo de los pedidos");
		exit(EXIT_FAILURE);
	}else{
		log_info(logUMC,"Se creo el hilo para recibir pedidos");
	}
	pthread_attr_destroy(&attr);

	ejecutoConsola();


	//Cierro el puerto y libero la memoria del socket
	close(socketServerPedido);
	log_info(logUMC,"Socket de pedidos cerrado");
	close(socketswap);
	log_info(logUMC,"Socket de swap cerrado");

	//Libero la memoria de la estructura
	eliminarConfigUmc(config_umc);
	config_destroy(config);
	log_debug(logUMC,"Configuracion destruida");

	destruir_estructuras();
	log_info(logUMC,"Estructuras de memoria destruidas");

	log_destroy(logUMC);

	return EXIT_SUCCESS;
}




