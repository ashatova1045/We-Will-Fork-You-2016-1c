//------------------------------------------------------------------------------------------------------
//Includes
//------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <commons/log.h>//Incluyo commons
#include <commons/string.h>//Incluyo funciones de strings
#include <parser/metadata_program.h>//Incluyo el parser
#include "../../sockets/Sockets.h"
#include <unistd.h>
#include <commons/config.h>
#include <errno.h>

#include "estructuras_umc.h"

//------------------------------------------------------------------------------------------------------
//Log
//------------------------------------------------------------------------------------------------------

//Declaro el log
t_log* logUMC;

//Defino la funcion para crear el log
t_log* crearLog(){
	t_log* logUMC = log_create("Log.log","umc.c",false,LOG_LEVEL_DEBUG);
	return logUMC;
}

//------------------------------------------------------------------------------------------------------
//Configuración
//------------------------------------------------------------------------------------------------------


//Declaro la estructura de configuración
t_umcConfig* config_umc;

//Defino la función para leer la configuración
t_umcConfig* leerConfiguracion(t_config* config){
	//Asigno memoria para estructura de configuración
	t_umcConfig* datosUmc = (t_umcConfig*)malloc(sizeof(t_umcConfig));

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
//Estructuras de memoria
//------------------------------------------------------------------------------------------------------

char *memoria_principal;
t_marco *tabla_marcos;
t_info_marco *tlb;


//------------------------------------------------------------------------------------------------------
//Sockets
//------------------------------------------------------------------------------------------------------
//Creo el entero para referenciar al socket del swap
int socketServerPedido, socketswap;
char* codigo; //fixme
int d; //fixme

//Función para manejar los mensajes
void manejar_paquete(int socket,t_paquete paq){
	log_info(logUMC,"Llego un paquete");
	//Comprueba el codigo de operacion
	switch (paq.cod_op) {
		//Si es handshake de CPU le manda ok
		case HS_CPU_UMC:
			enviar(OK_HS_CPU,1,&socket,socket);
			puts("Handshake CPU correcto");
			log_info(logUMC,"Handshake con CPU exitoso");
			break;
		//Si es handshake de nucleo le manda ok
		case HS_NUCLEO_UMC:
			enviar(OK_HS_NUCLEO,1,&socket,socket);
			puts("Handshake Nucleo correcto");
			log_info(logUMC,"Handshake con nucleo exitoso");
			break;
		//Si es el código de error
		case ERROR_COD_OP:
			log_error(logUMC,"Llego el codigo de error");
			exit(EXIT_FAILURE);
		//Si llega un nuevo programa lo reenvio al swap
		case NUEVO_PROGRAMA:
			puts(paq.datos);
			enviar(paq.cod_op,paq.tamano_datos,paq.datos,socketswap);
			log_info(logUMC,"Paquete mandado a swap");
			break;
		//Si llega el codigo 50 de pedido de paquete de CPU
		case 50:
			puts("Llego 50");
			//Le pido el paquete al swap
			enviar(50,1,&socketswap,socketswap);
			//El swap me manda el paquete pedido
			t_paquete* nuevo_paq=recibir_paquete(socketswap);
			log_info(logUMC,"Pedido de paquete al swap");
			//Reenvio paquete a la CPU
			enviar(50,nuevo_paq->tamano_datos,nuevo_paq->datos,socket);
			log_info(logUMC,"Paquete reenviado a la CPU");
			break;
		default:
			break;
	}

	printf("Llego un pedido de conexion de %d\n",socket);
	//log_info("Llego un pedido de conexion de %d\n",socket);

	printf("El socket %d dice:\n",socket);
	//log_info("El socket %d dice: \n",socket);

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

	//While para ejecutar el menu de la consola
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
//------------------------------------------------------------------------------------------------------
//Hilos sockets
//------------------------------------------------------------------------------------------------------

//Creo la funcion del socket servidor para CPU y Nucleo
void servidor_pedidos(){

	log_debug(logUMC,"Entro a la funcion servidor");

	//Creo el server multiconexión
	fd_set set_de_fds;
	int fdmax;

	int puerto=config_umc->puerto;
	socketServerPedido = crear_server_multiconexion(&set_de_fds,puerto,&fdmax);
	if(socketServerPedido ==-1){
		log_error(logUMC,"Error al crear el server");
		exit(EXIT_FAILURE);
	}

	//Mensajes de conexión exitosa
	printf("Se creo un socket multiconexion. Su fd es: %d \n",socketServerPedido);
	puts("Escuchando conexiones y corriendo!");

	//correr_server_multiconexion(fdmax,&set_de_fds,socketServer,manejar_paquete,cerrar_conexion,nueva_conexion);
	correr_server_multiconexion(&fdmax,&set_de_fds,socketServerPedido,manejar_paquete,cerrar_conexion,nueva_conexion);


	//Cierro el puerto y libero la memoria del socket
	close(socketServerPedido);
}
//------------------------------------------------------------------------------------------------------

int main(int argc, char **argv){

	//Defino el hilo para el socket servidor
	pthread_t  pedidosThread;

	//Creo el archivo log
	logUMC = crearLog();
	log_info(logUMC,"Ejecución del proceso UMC");

	//Creo el archivo de configuración
	t_config* config = config_create("../umc/umc.cfg");

	//Leo la configuración de la memoria
	config_umc = leerConfiguracion(config);
	printf("Puerto de conexion %d\n",config_umc->puerto);

	log_info(logUMC,"Se cargo la configuracion");

	//Creo las estructuras de memoria
	crear_estructuras();

	//Me conecto al área de swap y hago handshake
	socketswap = conectar(config_umc->ip_swap,config_umc->puerto_swap);
	if(socketswap == -1)
		puts("No se pudo conectar\n");

	if(handshake(socketswap,HS_UMC_SWAP,OK_HS_UMC) ==-1 ){
		puts("Swap no respondio al handshake");
	}
	printf("Handshake Swap correcto");


	//Creo el hilo de pedidos
	log_debug(logUMC,"Creando el hilo para recibir pedidos");
	if(pthread_create(&pedidosThread,NULL,(void*)servidor_pedidos,NULL)){
		perror("Error al crear el hilo de la cpu");
		exit(EXIT_FAILURE);
	}

	ejecutoConsola();

	//TODO Leer archivo de configuración y solicitar un bloque de memoria contigua

	//TODO Crear estructura caché TLB

	//TODO Crear estructuras administrativas(Páginas y frames)

	//TODO Manejar_pedidos de conexiones (multihilo para cpu)

	//TODO mensajes faltantes a Swap

	//TODO Operaciones


	//Cierro el puerto y libero la memoria del socket
	close(socketServerPedido);
	log_info(logUMC,"Socket de pedidos cerrado");
	close(socketswap);
	log_info(logUMC,"Socket de swap cerrado");

	//Libero la memoria de la estructura
	eliminarConfigUmc(config_umc);
	config_destroy(config);
	log_debug(logUMC,"Configuracion destruida");

	destruir_estructuras();
	log_info(logUMC,"Estructuras de memoria destruidas");

	log_destroy(logUMC);

	return EXIT_SUCCESS;
}




