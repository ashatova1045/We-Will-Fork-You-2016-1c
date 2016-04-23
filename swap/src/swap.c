#include <commons/config.h>
#include <commons/log.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "estructuras_swap.h"
#include "funciones_swap.h"

t_log* logSwap;
t_swapcfg* config_swap;

int main(int argc, char** argv) {

	// Crea archivo de log
	logSwap = crearLog();
	log_info(logSwap, "Ejecución del proceso SWAP");

	// Levanta la configuración del Swap
	config_swap = levantarConfiguracion();
	printf("Se conecta al puerto %d \n",config_swap->puerto_escucha);

	// Inicializa la conexión y queda a espera de procesos UMC
	int socketserver = crear_socket_escucha(config_swap->puerto_escucha);
	//int socketserver = crear_socket_escucha(4100);
	if (socketserver == -1)
		exit(EXIT_FAILURE);

	puts("Esperando procesos UMC...");

	// Acepta la conexón de un proceso UMC
	int socket_memoria =  aceptar_cliente(socketserver);

	if(socket_memoria == -1)
		exit(EXIT_FAILURE);

	printf("Se conectó el UMC con socket: %d \n",socket_memoria);

	t_paquete* paquete;

	// Recibe la petición
	paquete = recibir_paquete(socket_memoria);

	if(paquete->cod_op == ERROR_COD_OP)
		exit(EXIT_FAILURE);

	puts("Proceso recibido");
	printf("Codigo de operacion: %d\n",paquete->cod_op);
	printf("Tamano de los datos: %d\n",paquete->tamano_datos);
	printf("Datos: %s\n",(char*)paquete->datos);

	//todo: Asignar tamaño necesario para el proceso en caso de solicitarse

	//todo: Compactar partición en caso de fragmentación

	//todo: Devolver página / Sobreescribir página

	//todo: Administrar espacio libre - Control de Bitmap

	//todo: Liberar espacio en caso que se finalice el proceso

	//responderUMC(&socket_memoria);

	//destruir_paquete(paquete);

	close(socketserver);
	close (socket_memoria);
	return EXIT_SUCCESS;
}
