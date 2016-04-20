#include <stdio.h>
#include <stdlib.h>
#include "../../sockets/Sockets.h"
#include <pthread.h>
#include <unistd.h>
#include <commons/config.h>

FILE* abrir_programa(char* path) {
	FILE* programa = fopen(path, "r");
	if (!programa) {
		perror("Error al abrir el archivo");
		exit(EXIT_FAILURE);
	}
	return programa;
}

char* leer_programa(FILE* programa_fd,long int size){
	char* programabuf = malloc(size);
	if (!programabuf){
		perror("Error al reservar memoria para un programa");
		exit(EXIT_FAILURE);
	}

	fseek(programa_fd, 0L, SEEK_SET);
	fread(programabuf, sizeof(char), size, programa_fd);
	return programabuf;
}

int conectar_kernel(){
	t_config *conf = config_create("../consola/consola.cfg"); //esto se corre desde la carpeta /test
	if (!conf)
		exit(EXIT_FAILURE);

	int sock = conectar(config_get_string_value(conf,"IP_KERNEL"),config_get_int_value(conf,"PUERTO_KERNEL"));

	config_destroy(conf);

	if(sock == -1)
		exit(EXIT_FAILURE);
	return sock;
}
long int calcular_tamano_prog(FILE* programa){
	fseek(programa, 0L, SEEK_END);
	long int pos = ftell(programa);
	return pos;
}

int main(int argc, char **argv) {
	/*no es necesario usar signals para el ctrl+c.
	 * si matan a este proceso, el select del otro lado se da cuenta solo
	 */

	//leer el codigo del programa
	FILE* programa = abrir_programa(argv[1]);
	long int size = calcular_tamano_prog(programa);
	char* programabuf = leer_programa(programa,size);
	fclose(programa);

	int socket_kernel = conectar_kernel();
	if(socket_kernel == -1)
		exit(EXIT_FAILURE);

	//todo: handshake con kernel? o lo que sigue cuenta como handshake?
	enviar(NUEVO_PROGRAMA,size,programabuf,socket_kernel);
	free(programabuf);

	t_paquete* actualizacion;
	do {
		actualizacion = recibir_paquete(socket_kernel);
		switch (actualizacion->cod_op) {
			case IMPRIMIR_TEXTO:
					puts("Impresion de texto por pantalla:");
					puts(actualizacion->datos);
				break;
			case IMPRIMIR_VARIABLE:
				puts("Impresion de variable por pantalla:");
				//todo imprimir variable
				break;
			case TERMINO_BIEN_PROGRAMA:
				puts("El programa ha finalizado correctamente");
				break;
			case TERMINO_MAL_PROGRAMA:
				puts("El programa ha finalizado inesperadamente");
				break;
			case ERROR_COD_OP:
				puts("Error en la conexion!!");
				break;
			default:
				break;
		}
	} while (	actualizacion->cod_op != TERMINO_BIEN_PROGRAMA &&
				actualizacion->cod_op != TERMINO_MAL_PROGRAMA &&
				actualizacion->cod_op != ERROR_COD_OP);

	close(socket_kernel);

	return EXIT_SUCCESS;
}
