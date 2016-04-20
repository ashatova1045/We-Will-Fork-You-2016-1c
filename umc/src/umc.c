#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>//Incluyo commons
#include <commons/string.h>
#include <parser/metadata_program.h>//Incluyo el parser
#include "../../sockets/Sockets.h"
#include <stdbool.h>
#include <unistd.h>
#include <commons/config.h>

//Defino la funcion para crear el log
t_log* crearLog(){
	t_log* logUMC = log_create("Log.txt","umc.c",false,LOG_LEVEL_INFO);
	return logUMC;
}

//Creo la estructura de configuración
typedef struct{
	int puerto;
	char* ip_swap;
	int puerto_swap;
	int cant_marcos;
	int marco_size;
	int marco_x_proc;
	int entradas_tlb;
	int retardo;
}t_umcConfig;

t_umcConfig* leerConfiguracion(){
	//Asigno memoria para estructura de configuración
	t_umcConfig* datosUmc = (t_umcConfig*)malloc(sizeof(t_umcConfig));

	//Creo el archivo de configuración
	t_config* config = config_create("../umc/umc.cfg");

	//Completo los datos de la umc con los valores del archivo de configuración
	datosUmc->puerto = config_get_int_value(config,"PUERTO");
	datosUmc->ip_swap = config_get_string_value(config,"IP_SWAP");
	datosUmc->puerto_swap = config_get_int_value(config,"PUERTO_SWAP");
	datosUmc->cant_marcos = config_get_int_value(config,"CANT_MARCOS");
	datosUmc->marco_size = config_get_int_value(config,"MARCO_SIZE");
	datosUmc->marco_x_proc = config_get_int_value(config,"MARCO_X_PROC");
	datosUmc->entradas_tlb = config_get_int_value(config,"ENTRADAS_TLB");
	datosUmc->retardo = config_get_int_value(config,"RETARDO");

	//Devuelvo la estructura de datos cargados
	return datosUmc;
}

//Creo el entero para referenciar al socket del swap
int socketswap;

//Función para manejar los mensajes
void manejar_paquete(int socket,t_paquete paq){
	printf("Llego un pedido de conexion de %d\n",socket);
	printf("El socket %d dice:\n",socket);
	puts(paq.datos);
	//Envío datos recibidos al área de swap
	enviar(1,paq.tamano_datos,paq.datos,socketswap);
	//destruir_paquete(paq);
}

//Cerrar puerto de socket conectado
void cerrar_conexion(int socket){
	printf("Se cerro %d\n",socket);
}

//Recibe nuevas conexiones
void nueva_conexion(int socket){
	printf("Se conecto %d\n",socket);
}
//Creo el socket al que le voy a mandar el mensaje

int main(int argc, char **argv){
	//Creo el archivo log
	t_log* logUMC = crearLog();
	log_info(logUMC,"Ejecución del proceso UMC");

	//Leo la configuración de la memoria
	t_umcConfig* config_umc = leerConfiguracion();
	printf("Puerto de conexion %d\n",config_umc->puerto);

	//TODO Leer archivo de configuración y solicitar un bloque de memoria contigua

	//TODO Crear estructura caché TLB

	//TODO Crear estructuras administrativas(Páginas y frames)

	//TODO Consola

	//TODO Conectar con proceso Núcleo

	//TODO Manejar_pedidos de conexiones (multihilo para cpu)

	//TODO mensajes faltantes a Swap

	//TODO Operaciones

	//Me conecto al área de swap
	socketswap = conectar("192.168.43.188",4100);

	//Creo el server multiconexión
	fd_set set_de_fds;
	int* fdmax = malloc(sizeof(int));
	if (fdmax==NULL){
		perror("Error al alocar memoria.");
		return EXIT_FAILURE;
	}
	int puerto=4200;
	int socketServer = crear_server_multiconexion(&set_de_fds,puerto,fdmax);

	//Mensajes de conexión exitosa
	printf("Se creo un socket multiconexion. Su fd es: %d \n",socketServer);
	puts("Escuchando conexiones y corriendo!");

	//correr_server_multiconexion(fdmax,&set_de_fds,socketServer,manejar_paquete,cerrar_conexion,nueva_conexion);
	correr_server_multiconexion(fdmax,&set_de_fds,socketServer,manejar_paquete,cerrar_conexion,nueva_conexion);

	//Cierro el puerto y libero la memoria del socket
	close(socketServer);
	free(fdmax);
	return EXIT_SUCCESS;
}




