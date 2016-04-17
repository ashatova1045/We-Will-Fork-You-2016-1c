#include <stdio.h>
#include <stdlib.h>
#include "../../sockets/Sockets.h"
#include <pthread.h>
#include <parser/metadata_program.h>

int main(int argc, char **argv) {
	int socket_kernel = conectar("192.168.43.246",4000);
	if(socket_kernel == -1)
		exit(EXIT_FAILURE);
	getchar();
	enviar(1,5,"hola",socket_kernel);
	return EXIT_SUCCESS;
}
