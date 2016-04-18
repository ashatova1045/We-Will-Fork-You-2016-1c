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

int responderUMC(){
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

		mensaje = "respuestaSwap";

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

	// Inicializa la conexión y queda a espera de procesos UMC
	int socketserver = crear_socket_escucha(4100);
	if (socketserver == -1)
		exit(EXIT_FAILURE);

	//printf("Se creo un socket. Su fd es: %d \n",socketserver);
	puts("Escuchando conexiones");

	// Acepta la conexón de un proceso UMC
	int socket_conectado =  aceptar_cliente(socketserver);

	if(socket_conectado == -1)
		exit(EXIT_FAILURE);

	printf("Se conecto el UMC con socket: %d \n",socket_conectado);

	t_paquete* paquete;

	// Recibe la petición
	paquete = recibir_paquete(socket_conectado);
	if(paquete->cod_op == ERROR_COD_OP)
		exit(EXIT_FAILURE);

	//uint16_t cod_op = paquete->cod_op;
 	//char *datos = paquete->datos;
	puts("Paquete recibido");
	printf("Codigo de operacion: %d\n",paquete->cod_op);
	printf("Tamano de los datos: %d\n",paquete->tamano_datos);
	printf("Datos: %s\n",(char*)paquete->datos);

	//todo: Asignar tamaño necesario para el proceso en caso de solicitarse
	//todo: Compactar partición en caso de fragmentación

	//todo: Devolver página / Sobreescribir página

	//todo: Administrar espacio libre - Control de Bitmap

	//todo: Liberar espacio en caso que se finalice el proceso

//	responderUMC();

	destruir_paquete(paquete);

	close(socketserver);
	close (socket_conectado);
	return EXIT_SUCCESS;
}
