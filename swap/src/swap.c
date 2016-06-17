#include "funciones_swap.h"
#include "estructuras_swap.h"



int main(int argc, char** argv) {

	// Crea archivo de log
	logSwap = crearLog();
	log_info(logSwap, "Ejecución del proceso SWAP");

	// Levanta la configuración del Swap
	t_config* config = config_create("../swap/swap.cfg");
	levantarConfiguracion(config);

	printf("Se conecta al puerto %d \n",datosSwap->puerto_escucha);
	log_info(logSwap, "Se conecta al puerto");

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
	log_info(logSwap,"Socket de UMC cerrado");
	free(datosSwap);
	config_destroy(config);
	return EXIT_SUCCESS;
}
