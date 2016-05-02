//------------------------------------------------------------------------------------------------------
//Includes
//------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <commons/log.h>//Incluyo commons
#include <commons/string.h>//Incluyo funciones de strings
#include <parser/metadata_program.h>//Incluyo el parser
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


int main(int argc, char **argv){

	//Defino el hilo para el socket servidor
	pthread_t  pedidosThread;

	//Creo el archivo log
	logUMC = crearLog();
	log_info(logUMC,"Ejecución del proceso UMC");

	//Creo el archivo de configuración
	t_config* config = config_create("../umc/umc.cfg");

	//Leo la configuración de la memoria
	config_umc = leerConfiguracion(config);
	printf("Puerto de conexion %d\n",config_umc->puerto);

	log_info(logUMC,"Se cargo la configuracion");

	//Me conecto al área de swap y hago handshake
	socketswap = conectar(config_umc->ip_swap,config_umc->puerto_swap);
	if(socketswap == -1)
		puts("No se pudo conectar\n");

	if(handshake(socketswap,HS_UMC_SWAP,OK_HS_UMC) ==-1 ){
		puts("Swap no respondio al handshake");
	}
	printf("Handshake Swap correcto");


	//Creo el hilo de pedidos
	log_debug(logUMC,"Creando el hilo para recibir pedidos");
	if(pthread_create(&pedidosThread,NULL,(void*)servidor_pedidos,NULL)){
		perror("Error al crear el hilo de la cpu");
		exit(EXIT_FAILURE);
	}

	ejecutoConsola();

	//TODO Solicitar un bloque de memoria contigua de tamaño definido por archivo de configuracion

	//TODO Crear estructura caché TLB

	//TODO Crear estructuras administrativas(Páginas y frames)

	//TODO Manejar_pedidos de conexiones (multihilo para cpu)

	//TODO mensajes faltantes a Swap

	//TODO Operaciones


	//Cierro el puerto y libero la memoria del socket
	close(socketServerPedido);

	//Libero la memoria de la estructura
	eliminarConfigUmc(config_umc);

	config_destroy(config);

	return EXIT_SUCCESS;
}




