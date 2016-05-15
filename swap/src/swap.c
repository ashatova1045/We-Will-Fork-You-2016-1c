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
int tamanio;

t_log* logSwap;
t_swapcfg* config_swap;
t_list* lista_procesos;
int socket_memoria;
t_bitarray* bitarray;

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

	lista_procesos = list_create();

	//// TEST LISTA Y GRABACIÓN ARCHIVO SWAP
	/*
	int pid = 175;
	int cantPags = 20;

	int y;
	int posicion = 0, pagina = 0;
	while(bitarray_test_bit(bitarray,posicion)){
		posicion++;
	}

	for(y=0;y<cantPags;y++){
		//printf("Página %d - Posición %d \n",pagina,posicion);
		t_control_swap* controlSwap = malloc(sizeof(t_control_swap));
		controlSwap->PId = pid;
		controlSwap->cantPaginas = pagina;
		controlSwap->posicion = posicion;

		list_add(lista_procesos,controlSwap);

		pagina++;
		posicion++;
	}

	if(fwrite(lista_procesos,sizeof(t_list),1,swapFile) != 1){
		puts("Error");
	}else{
		puts("Grabación exitosa");
	}
	fseek(swapFile,0,SEEK_SET);

	t_list* listaLeida = malloc(sizeof(t_list));
	t_control_swap* controlSwap2 = malloc(sizeof(t_control_swap));

	fread(listaLeida,sizeof(t_control_swap),1,swapFile);

	int i=0;
	while(!list_is_empty(listaLeida)){
		controlSwap2 = list_remove(listaLeida,i);
		printf("Lee PID %d \n",controlSwap2->PId);
		printf("Lee Página %d \n",controlSwap2->cantPaginas);
		printf("Lee Posición %d \n",controlSwap2->posicion);
	}
	*/
	//// FIN TEST LISTA Y GRABACIÓN ARCHIVO SWAP

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
