#include <stdbool.h>
#include "../../sockets/Sockets.h"
//#include "../../general/general.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "estructuras_swap.h"
#include "funciones_swap.h"
#include <commons/config.h>
#include <commons/log.h>

t_log* logSwap;
t_swapcfg* config_swap;
int socket_memoria;

int main(int argc, char** argv) {

	// Crea archivo de log
	logSwap = crearLog();
	log_info(logSwap, "Ejecución del proceso SWAP");

	// Levanta la configuración del Swap
	t_config* config = config_create("../swap/swap.cfg");
	config_swap = levantarConfiguracion(config);
	printf("Se conecta al puerto %s \n",config_swap->ip_umc);
	printf("Se conecta al puerto %d \n",config_swap->puerto_escucha);

	// Inicializa la conexión y queda a espera de procesos UMC
	int socketserver = crear_socket_escucha(config_swap->puerto_escucha);

	if (socketserver == -1)
		exit(EXIT_FAILURE);

	puts("Esperando procesos UMC...");

	// Acepta la conexón de un proceso UMC
	socket_memoria =  aceptar_cliente(socketserver);

	if(socket_memoria == -1)
		exit(EXIT_FAILURE);

	log_info(logSwap,"Conectado a la UMC");

	config_destroy(config);

	//printf("Se conectó el UMC con socket: %d \n",socket_memoria);
	printf("Main - Socket memoria %d \n",socket_memoria);

	while(true){
		// Recibe la petición
		t_paquete* paquete;
		paquete = recibir_paquete(socket_memoria);

		manejar_socket_umc(paquete);

		 puts("Proceso recibido");
		 printf("Codigo de operacion: %d\n",paquete->cod_op);
		 printf("Tamano de los datos: %d\n",paquete->tamano_datos);
		 printf("Datos: %s\n",(char*)paquete->datos);

		destruir_paquete(paquete);
	}

	//todo: Asignar tamaño necesario para el proceso en caso de solicitarse

	//todo: Compactar partición en caso de fragmentación

	//todo: Devolver página / Sobreescribir página

	//todo: Administrar espacio libre - Control de Bitmap

	//todo: Liberar espacio en caso que se finalice el proceso

	//destruir_paquete(paquete);

	close(socketserver);
	close (socket_memoria);
	return EXIT_SUCCESS;
}
