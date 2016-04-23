#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <commons/log.h>//Incluyo commons
#include <commons/string.h>//Incluyo funciones de strings
#include <parser/metadata_program.h>//Incluyo el parser
#include "../../sockets/Sockets.h"
#include <stdbool.h>
#include <unistd.h>
#include <commons/config.h>
#include <errno.h>
#include <commons/collections/list.h>

//------------------------------------------------------------------------------------------------------
//Log
//------------------------------------------------------------------------------------------------------

//Declaro el log
t_log* logUMC;

//Defino la funcion para crear el log
t_log* crearLog(){
	t_log* logUMC = log_create("Log.log","umc.c",false,LOG_LEVEL_INFO);
	return logUMC;
}

//------------------------------------------------------------------------------------------------------
//Configuración
//------------------------------------------------------------------------------------------------------

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

//Declaro la estructura de configuración
t_umcConfig* config_umc;

//Defino la función para leer la configuración
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

	//Elimino la configuración que ya no necesito
	config_destroy(config);

	//Devuelvo la estructura de datos cargados
	return datosUmc;
}

//Función para liberar la memoria pedida para la estructura de configuración
void eliminarConfigUmc(t_umcConfig* datosUmcAEliminar){
	//Si no hay datos de configuración no hace nada
	if(datosUmcAEliminar==NULL){
		return;
	//Si hay datos de configuración libero primero el puntero y luego la estructura
	}else{
		free(datosUmcAEliminar->ip_swap);
		free(datosUmcAEliminar);
	}
}

//------------------------------------------------------------------------------------------------------
//Sockets
//------------------------------------------------------------------------------------------------------
//Creo el entero para referenciar al socket del swap
int socketswap;

//Función para manejar los mensajes
void manejar_paquete(int socket,t_paquete paq){
	printf("Llego un pedido de conexion de %d\n",socket);
	printf("El socket %d dice:\n",socket);
	puts(paq.datos);
	//Envío datos recibidos al área de swap
	enviar(1,paq.tamano_datos,paq.datos,socketswap);
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

//------------------------------------------------------------------------------------------------------
//Consola
//------------------------------------------------------------------------------------------------------
//Defino la función para mostrar el menú de la consola
void imprimirMenu(){
	printf("\n--------------------------------------\n");
	printf("Bienvenido a la consola de la UMC\n");
	printf("\nIngrese el comando a ejecutar\n");
	printf("\nPara ver la lista de comandos y como usarlos ingrese help\n");
	printf("\n? ");
}

//Defino funcion de comando para mostrar las opciones
void mostrarOpciones(){
	printf("\nretardo\n");
	printf("Permite modificar la cantidad de milisegundos que debe esperar el proceso UMC\n");
	printf("antes de responder una solicitud\n");
	printf("dump\n");
	printf("Genera un reporte en pantalla y en un archivo en disco\n");
	printf("flush tlb\n");
	printf("Limpiar por completo el contenido de la TLB\n");
	printf("flush memory\n");
	printf("Marca todas las páginas de un proceso como modificadas\n");
}

//Defino funcion de comando retardo
void ingresarRetardo(){
	int milisegundos=0;

	printf("\nIngrese la cantidad de milisegundos que debe esperar el proceso UMC\n");
	printf("antes de responder a una solicitud de servicio\n");
	printf("\nCantidad milisegundos: ");

	//Limpio el buffer de entrada
	fflush(stdin);

	//Se ingresa por teclado el retardo
	scanf("%d",&milisegundos);

	//Actualizo los milisegundos en la estructura de la umc
	config_umc->retardo=milisegundos;

	printf("\nSe actualizo el retardo\n");
	getchar();
}

//Defino funcion de comando dump-Estructura memoria-Todos los procesos
void rEstructurasDeMemoriaTodas(){
	printf("\nTablas de paginas de todos los procesos\n");
	getchar();
	//TODO generar reporte de tabla de paginas de todos los procesos
}

//Defino funcion de comando dump-Estructura memoria-Un proceso
void rEstructuraDeMemoriaProceso(){
	int idProceso=0;

	printf("\nIngrese el identificador del proceso para generar el reporte\n");
	printf("\nId de Proceso: ");

	//Limpio el buffer de entrada
	fflush(stdin);

	//Se ingresa el id del proceso por teclado
	scanf("%d",&idProceso);

	printf("\nTabla de paginas del proceso: %d\n",idProceso);
	getchar();

	//TODO Generar reporte de tabla de paginas del proceso ingresado
}

//Defino funcion de comando dump-Datos memoria-Todos los procesos
void rDatosMemoriaTodos(){
	printf("\nDatos en memoria de todos los procesos\n");
	getchar();

	//TODO Generar reporte de datos en memoria de todos los procesos
}

//Defino funcion de comando dump-Datos memoria-Un proceso
void rDatosMemoriaProceso(){
	int idProceso=0;

	printf("\nIngrese el identificador del proceso para generar el reporte\n");
	printf("\nId del Proceso: ");

	//Limpio buffer de entrada
	fflush(stdin);

	//Se ingresa el id del proceso por teclado
	scanf("%d",&idProceso);

	printf("\nDatos en memoria del proceso: %d\n",idProceso);
	getchar();

	//TODO Generar reporte de datos en memoria del proceso ingresado
}

//Defino funcion de menu de comando dump
void reportes(){
	int opcion=0;

	//Ciclo para mostrar el menu
	while(1){
	printf("\nIngrese la opcion del reporte que desea ver\n");
	printf("\nReportes de estructuras de memoria\n");
	printf("	1.Tabla de paginas de todos los procesos\n");
	printf("	2.Tabla de paginas de un proceso en particular\n");
	printf("Reportes de contenido de memoria\n");
	printf("	3.Datos en memoria de todos los procesos\n");
	printf("	4.Datos en memoria de un proceso en particular\n");
	printf("5.Volver al menu principal\n");
	printf("\n? ");

	//Limpio el buffer de entrada
	fflush(stdin);

	//Se ingresa la opcion por teclado
	scanf("%d",&opcion);
	if(opcion==1)rEstructurasDeMemoriaTodas();
	else if(opcion==2)rEstructuraDeMemoriaProceso();
	else if(opcion==3)rDatosMemoriaTodos();
	else if(opcion==4)rDatosMemoriaProceso();
	else if(opcion==5)break;
	else printf("\nLa opcion ingresada no es valida\n");

	printf("\nPara volver a la lista de reportes presione Enter\n");

	getchar();

	}
}

//Defino funcion de comando flush tlb
void limpiarTLB(){
	printf("\nSe limpio la tabla de paginas\n");
	//TODO limpiar completamente el contenido de la tabla de paginas
}

//Defino funcion de comando flush memory
void marcarPaginas(){
	int idProceso=0;

	printf("\nIngrese el identificador del proceso para marcar sus paginas\n");
	printf("como modificadas\n");
	printf("\nId del Proceso: ");

	//Limpio el buffer de entrada
	fflush(stdin);

	//Se ingresa el id del proceso por teclado
	scanf("%d",&idProceso);

	printf("\nSe marcaron las paginas del proceso %d como modificadas",idProceso);
	printf("\n");

	getchar();
	//TODO Marcar las paginas del proceso ingresado como leidas
}

//Defino la función para ejecutar la consola
void ejecutoConsola(){

	char* comando=NULL;
	size_t len=0;

	//Mientras no se ingrese el comando para salir de la consola
	while(1){

		//Limpio el valor del comando ingresado
		comando=NULL;

		imprimirMenu();

		//Limpio el buffer de entrada y espero que ingresen comando a la consola
		fflush(stdin);

		//Se ingresa el comando por teclado
		getline (&comando, &len, stdin);

		//Evaluacion de comando para realizar la accion que corresponda
		if(string_equals_ignore_case(comando,"help\n"))mostrarOpciones();
		else if(string_equals_ignore_case(comando,"retardo\n"))ingresarRetardo();
		else if(string_equals_ignore_case(comando,"dump\n"))reportes();
		else if(string_equals_ignore_case(comando,"flush tlb\n"))limpiarTLB();
		else if(string_equals_ignore_case(comando,"flush memory\n"))marcarPaginas();
		else printf("\nEl comando no es valido\n");

		printf("\nPara continuar presione Enter\n");

		getchar();

	}
}

//Defino el hilo de la consola
//pthread_t consolaThread;

//------------------------------------------------------------------------------------------------------

int main(int argc, char **argv){

	//Creo el archivo log
	logUMC = crearLog();
	log_info(logUMC,"Ejecución del proceso UMC");

	//Leo la configuración de la memoria
	config_umc = leerConfiguracion();
	printf("Puerto de conexion %d\n",config_umc->puerto);

	//TODO poner todos los sockets aquí!!!!!!!!

	//Creo el hilo de la consola
	/*if(pthread_create(&consolaThread,NULL,ejecutoConsola,NULL)){
		perror("Error al crear el hilo de la consola");
		exit(EXIT_FAILURE);
	}*/

	ejecutoConsola();

	//Se escribe el log de creacion exitosa de hilo de consola
	log_info(logUMC,"Se creo el hilo de la consola");



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

	//Terminacion del hilo de consola
	//pthread_join(consolaThread,NULL);

	//Cierro el puerto y libero la memoria del socket
	close(socketServer);
	free(fdmax);

	//Libero la memoria de la estructura
	eliminarConfigUmc(config_umc);

	return EXIT_SUCCESS;
}




