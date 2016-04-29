#include <commons/config.h>
#include <commons/log.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../../sockets/Sockets.h"
#include "../../general/general.h"
#include "estructuras_swap.h"
#include "funciones_swap.h"
#include <commons/collections/list.h>

char* prog;
int tamanio;

t_log* logSwap;
t_swapcfg* config_swap;
t_list* lista_utilizada;
int socket_memoria;

int main(int argc, char** argv) {


	// Crea archivo de log
	logSwap = crearLog();
	log_info(logSwap, "Ejecución del proceso SWAP");

	// Levanta la configuración del Swap
	t_config* config = config_create("../swap/swap.cfg");
	config_swap = levantarConfiguracion(config);

	printf("Se conecta al puerto %d \n",config_swap->puerto_escucha);
	log_info(logSwap, "Se conecta al puerto");

	// Inicializa archivo Swap
	FILE* swapFile = inicializaSwapFile(config_swap);

	// Inicializa la conexión y queda a espera de procesos UMC
	int socketserver = crear_socket_escucha(config_swap->puerto_escucha);

	if (socketserver == -1)
		exit(EXIT_FAILURE);

	puts("Esperando procesos UMC...");
	log_info(logSwap, "Esperando procesos UMC...");

	// Acepta la conexón de un proceso UMC
	socket_memoria =  aceptar_cliente(socketserver);

	if(socket_memoria == -1)
		exit(EXIT_FAILURE);

	puts("Conectado a la UMC");
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

	fclose(swapFile);
	close(socketserver);
	close (socket_memoria);
	return EXIT_SUCCESS;
}
