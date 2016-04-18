#include <stdio.h>
#include <stdlib.h>
#include "../../sockets/Sockets.h"
#include <pthread.h>
#include <unistd.h>

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
	int sock = conectar("192.168.43.246",4000);
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
	//leer el codigo del programa
	FILE* programa = abrir_programa(argv[1]);
	long int size = calcular_tamano_prog(programa);
	char* programabuf = leer_programa(programa,size);
	fclose(programa);


	int socket_kernel = conectar_kernel();
	//todo: handshake con kernel, enviar el codigo del programa
	enviar(1,size,programabuf,socket_kernel);
	//todo:esperar respuesta/finalizacion en un while

	close(socket_kernel);
	free(programabuf);

	return EXIT_SUCCESS;
}
