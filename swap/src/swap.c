#include <stdio.h>
#include <stdlib.h>
#include "../../sockets/Sockets.h"
#include <commons/string.h>

typedef struct{
	int puerto_escucha;
	char* nombre_swap;
	int cantidad_paginas;
	int tamanio_pagina;
	int retardo_compactacion;
}t_confFile;

int crearArchivoConfiguracion(){
	FILE * confFile;
	confFile = fopen("swap.data", "w");
	t_confFile confData;
	confData.puerto_escucha = 4100;
	confData.nombre_swap = "swap.data";
	confData.cantidad_paginas = 512;
	confData.tamanio_pagina = 256;
	confData.retardo_compactacion = 6000;
	fseek(confFile,0,SEEK_SET);
	if(fwrite(&confData,sizeof(t_confFile),1,confFile) == 1)
		return EXIT_SUCCESS;

	return EXIT_FAILURE;

}

int contestar(){
	int socket = conectar("192.168.43.57",4200);
	if (socket == -1)
		exit(EXIT_FAILURE);

	printf("Se conecto. Se creo el socket %d \n",socket);
	char* mensaje = malloc(100*sizeof(char));
	for(;;){
		//puts("Escriba su mensaje y pulse enter para enviarlo.");
		//if(fgets(mensaje, 100, stdin)==NULL){
		//	perror("Error al leer datos.");
		//	return EXIT_FAILURE;
		//}

		mensaje = "puertademierda";

		int msj_len = strlen(mensaje);
		//mensaje[msj_len-1] = '\0'; //reemplazo el salto de linea por el caracter nulo

		//si la persona escribio "hola", realoco a 5 bytes ("hola" + el de escape).
		//asi puedo enviar menos bytes

		if (mensaje==NULL){
			perror("Error al alocar memoria.");
			return EXIT_FAILURE;
		}

		enviar(1,msj_len,mensaje,socket); //como solo estoy mandando un string lo mando con el caracter de escape incluido
		puts("Mensaje enviado");

	}
	free(mensaje);

	close(socket);
	return EXIT_SUCCESS;
}

int main(int argc, char** argv) {

	//if(crearArchivoConfiguracion() != 1){
	//	return EXIT_FAILURE;
	//}

	int socketserver = crear_socket_escucha(4100);
	if (socketserver == -1)
		exit(EXIT_FAILURE);

	//printf("Se creo un socket. Su fd es: %d \n",socketserver);
	puts("Escuchando conexiones");

	int socket_conectado =  aceptar_cliente(socketserver);

	if(socket_conectado == -1)
		exit(EXIT_FAILURE);

	printf("Se conecto el UMC con socket: %d \n",socket_conectado);

	t_paquete* paquete;
	paquete = recibir_paquete(socket_conectado);
	if(paquete->cod_op == ERROR_COD_OP)
		exit(EXIT_FAILURE);

	puts("Paquete recibido");
	printf("Codigo de operacion: %d\n",paquete->cod_op);
	printf("Tamano de los datos: %d\n",paquete->tamano_datos);
	printf("Datos: %s\n",(char*)paquete->datos);

//	if(paquete->tamano_datos > 0){
//		contestar();
//	}


	destruir_paquete(paquete);

	close(socketserver);
	close (socket_conectado);
	return EXIT_SUCCESS;
}
