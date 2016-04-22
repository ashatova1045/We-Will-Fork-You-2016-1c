#include <stdio.h>
#include <stdlib.h>
#include "../../sockets/Sockets.h"
#include <pthread.h>
#include <unistd.h>
#include <commons/config.h>
#include <commons/log.h>
#include <errno.h>

t_log* logconsola;

FILE* abrir_programa(char* path) {
	FILE* programa = fopen(path, "r");
	if (!programa) {
		perror("Error al abrir el archivo");
		log_error(logconsola,"Error al abrir el archivo de programa %s",path);
		exit(EXIT_FAILURE);
	}
	log_debug(logconsola,"Abrio el archivo de programa %s",path);
	return programa;
}

char* leer_programa(FILE* programa_fd,long int size){
	char* programabuf = malloc(size);
	if (!programabuf){
		perror("Error al reservar memoria para un programa");
		log_error(logconsola,"Error al reservar %d bytes para el contenido del programa!",size);
		exit(EXIT_FAILURE);
	}

	fseek(programa_fd, 0L, SEEK_SET);
	fread(programabuf, sizeof(char), size, programa_fd);
	programabuf[size-1] = '\0'; //fread() no lo pone
	log_debug(logconsola,"El programa es:\n%s",programabuf);
	return programabuf;
}

int conectar_kernel(){
	t_config *conf = config_create("../consola/consola.cfg"); //esto se corre desde la carpeta /test
	if (!conf){
		log_error(logconsola,"Error al abrir la configuracion");
		exit(EXIT_FAILURE);
	}

	int sock = conectar(config_get_string_value(conf,"IP_KERNEL"),config_get_int_value(conf,"PUERTO_KERNEL"));
	if(sock == -1){
		log_error(logconsola,"Error al conectarse al server! %s\n IP: %s PUERTO: %d",strerror(errno),config_get_string_value(conf,"IP_KERNEL"),config_get_int_value(conf,"PUERTO_KERNEL"));
		exit(EXIT_FAILURE);
	}
	log_info(logconsola,"Conectado al kernel");

	config_destroy(conf);
	return sock;
}

long int calcular_tamano_prog(FILE* programa){
	fseek(programa, 0L, SEEK_END);
	long int pos = ftell(programa) + 1; //sumo lugar para \0
	log_debug(logconsola,"El tamano del archivo es de %d",pos);
	return pos;
}

int main(int argc, char **argv) {
	/*no es necesario usar signals para el ctrl+c.
	 * si matan a este proceso, el select del otro lado se da cuenta solo
	 */

	logconsola = log_create("consola.log","Consola",false,LOG_LEVEL_DEBUG);

	//leer el codigo del programa
	FILE* programa = abrir_programa(argv[1]);
	long int size = calcular_tamano_prog(programa);
	char* programabuf = leer_programa(programa,size);
	fclose(programa);

	int socket_kernel = conectar_kernel();

	//todo: handshake con kernel? o lo que sigue cuenta como handshake?
	enviar(NUEVO_PROGRAMA,size,programabuf,socket_kernel);
	log_info(logconsola,"Programa enviado al kernel");
	free(programabuf);

	t_paquete* actualizacion;
	do {
		log_info(logconsola,"Eperando mensajes del kernel");
		puts("El programa esta corriendo correctamente. Espere un momento por favor.");
		actualizacion = recibir_paquete(socket_kernel);
		log_info(logconsola,"Nuevo mensaje del kernel!\nTamano: %d\nCopOp: %d",
					actualizacion->tamano_datos,
					actualizacion->cod_op);
		switch (actualizacion->cod_op) {
			case IMPRIMIR_TEXTO:
				log_info(logconsola,"Mostrando por pantalla el texto: %s",actualizacion->datos);
				puts("Impresion de texto por pantalla:");
				puts(actualizacion->datos);
				break;
			case IMPRIMIR_VARIABLE:
				log_info(logconsola,"Mostrando por pantalla la variable: %s",actualizacion->datos); //fixme ver que mostrar aca
				puts("Impresion de variable por pantalla:");
				//todo imprimir variable
				break;
			case TERMINO_BIEN_PROGRAMA:
				log_info(logconsola,"Termino correctamente!");
				puts("El programa ha finalizado correctamente!");
				break;
			case TERMINO_MAL_PROGRAMA:
				log_warning(logconsola,"Programa finalizado inesperadamente por el sistema");
				puts("El programa ha finalizado inesperadamente!");
				break;
			case ERROR_COD_OP:
				log_error(logconsola,"Error en la conexion: %s",strerror(errno));
				puts("Error en la conexion!");
				break;
			default:
				break;
		}
	} while (	actualizacion->cod_op != TERMINO_BIEN_PROGRAMA &&
				actualizacion->cod_op != TERMINO_MAL_PROGRAMA &&
				actualizacion->cod_op != ERROR_COD_OP);

	log_info(logconsola,"Salio del loop y ya no recibira mensajes.");
	close(socket_kernel);

	return EXIT_SUCCESS;
}
