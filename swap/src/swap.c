#include <commons/config.h>
#include <commons/log.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../../sockets/Sockets.h"
#include "estructuras_swap.h"
#include "funciones_swap.h"
#include <commons/collections/list.h>
#include <commons/bitarray.h>

char* prog;

t_log* logSwap;
t_list* lista_procesos;
int socket_memoria;
t_swapcfg* datosSwap;

int main(int argc, char** argv) {

	// Crea archivo de log
	logSwap = crearLog();
	log_info(logSwap, "Ejecución del proceso SWAP");

	// Levanta la configuración del Swap
	t_config* config = config_create("../swap/swap.cfg");
	levantarConfiguracion(config);

	printf("Se conecta al puerto %d \n",datosSwap->puerto_escucha);
	log_info(logSwap, "Se conecta al puerto");

	// Inicializa archivo Swap
	if(inicializaSwapFile(datosSwap) == -1){
		return 0;
	}

	lista_procesos = list_create();

	// Inicializa la conexión y queda a espera de procesos UMC
	int socketserver = crear_socket_escucha(datosSwap->puerto_escucha);

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

	printf("Main - Socket memoria %d \n",socket_memoria);

	while(true){
		// Recibe la petición
		t_paquete* paquete;
		paquete = recibir_paquete(socket_memoria);

		manejar_socket_umc(paquete);

		destruir_paquete(paquete);
	}

	list_destroy(lista_procesos);
	fclose(swapFile);
	close(socketserver);
	close (socket_memoria);
	return EXIT_SUCCESS;
}
