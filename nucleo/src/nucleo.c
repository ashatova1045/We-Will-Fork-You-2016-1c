#include <parser/metadata_program.h>
#include "../../sockets/Sockets.h"
#include <commons/string.h>
#include <pthread.h>
#include <commons/log.h>
#include <errno.h>
#include <commons/config.h>
#include "estados.h"
#include "../../general/pcb.h"


typedef struct {
	int puerto;
	void (*manejar_pedido)(int,t_paquete);
	void (*socket_cerrado)(int);
	void (*conexion_nueva_aceptada)(int);
} t_estructura_server;

typedef struct{
	int puerto;
	char* direccion;
}t_estructura_cliente;

typedef struct{
	int puerto_prog;
	int puerto_cpu;
	int puerto_umc;
	char* ip_umc;
	int quantum;
	int quantum_sleep;
	char** semaf_ids;
	char** semaf_init; //int* lo dejo como char** para usar get_array_value
	char** io_ids;
	char** io_retardo; //int* lo dejo como char** para usar get_array_value
	char** shared_vars;
} t_nucleoConfig;



/*semaforo = malloc(sizeof(semaforo_t));
semaforo->valor = atoi(valorSemaforosArray[i]);
semaforo->cola = queue_create();
usando el atoi dentro de un while*/

int cpu;
int consola;
int socket_umc;
int pidActual=0;
t_log *logNucleo;

t_log* crearLog(){
	t_log *logNucleo = log_create("logNucleo.log", "nucleo.c", false, LOG_LEVEL_TRACE);
	return logNucleo;
}

t_nucleoConfig* cargarConfiguracion(t_config* config){
	t_nucleoConfig* datosNucleo = (t_nucleoConfig*)malloc(sizeof(t_nucleoConfig));
	datosNucleo->puerto_prog=config_get_int_value(config,"PUERTO_PROG");
	datosNucleo->puerto_cpu=config_get_int_value(config,"PUERTO_CPU");
	datosNucleo->puerto_umc=config_get_int_value(config,"PUERTO_UMC");
	datosNucleo->ip_umc=config_get_string_value(config,"IP_UMC");
	datosNucleo->quantum=config_get_int_value(config,"QUANTUM");
	datosNucleo->quantum_sleep=config_get_int_value(config,"QUANTUM_SLEEP");
	datosNucleo->semaf_ids=config_get_array_value(config,"SEMAF_IDS");
	datosNucleo->semaf_init=config_get_array_value(config,"SEMAF_INIT");
	datosNucleo->io_ids=config_get_array_value(config,"IO_IDS");
	datosNucleo->io_retardo=config_get_array_value(config,"IO_RETARDO");
	datosNucleo->shared_vars=config_get_array_value(config,"SHARED_VARS");
	return datosNucleo;
}

/*#define DESTRUIR_PP(nucleo_param) \
		for(i=0 ; *(nucleo_param + i) != NULL ; i++ ){  \
			free(*(nucleo_param + i));					\
		}												\
		free(nucleo_param)								\
		*/


void armar_nuevo_pcb (t_paquete paquete){

	log_debug(logNucleo,"Armando pcb para programa:\n%s\ntamano: %d",paquete.datos,paquete.tamano_datos);
	t_metadata_program* metadata;
	metadata = metadata_desde_literal(paquete.datos);

	t_pcb* nvopcb = malloc(sizeof(t_pcb));
	nvopcb->pid=++pidActual;
	nvopcb->pc=0;
	nvopcb->cant_instrucciones=metadata->instrucciones_size;

	int tamano_instrucciones = sizeof(t_intructions)*nvopcb->cant_instrucciones;
	log_debug(logNucleo,"Tamano de las instrucciones %d",tamano_instrucciones);
	nvopcb->indice_codigo=malloc(tamano_instrucciones);
	memcpy(nvopcb->indice_codigo,metadata->instrucciones_serializado,tamano_instrucciones);

	registro_indice_stack * indice_stack= malloc(sizeof(registro_indice_stack));
	indice_stack->posicion=0;
	nvopcb->indice_stack=indice_stack;
	//nvopcb.cant_etiquetas=metadata->cantidad_de_etiquetas;
	log_debug(logNucleo,"PCB armado para programa:\n%s\ntamano: %d",paquete.datos,paquete.tamano_datos);

}


void destruirNucleoConfig(t_nucleoConfig* datosADestruir){
	int i=0;
	if (!datosADestruir)
		return;
/*	DESTRUIR_PP( datosADestruir->io_ids);
	DESTRUIR_PP( datosADestruir->io_retardo);
	DESTRUIR_PP( datosADestruir->semaf_ids);
	DESTRUIR_PP( datosADestruir->semaf_init);
	DESTRUIR_PP( datosADestruir->shared_vars);*/

	for(i=0 ; *(datosADestruir->io_ids + i) != NULL ; i++ ){
			free(*(datosADestruir->io_ids + i));
	}
	free(datosADestruir->io_ids);
	for(i=0 ; *(datosADestruir->io_retardo + i) != NULL ; i++ ){
		free(*(datosADestruir->io_retardo + i));
	}
	free(datosADestruir->io_retardo);
	for(i=0 ; *(datosADestruir->semaf_ids + i) != NULL ; i++ ){
		free(*(datosADestruir->semaf_ids + i));
	}
	free(datosADestruir->semaf_ids);
	for(i=0 ; *(datosADestruir->semaf_init + i) != NULL ; i++ ){
		free(*(datosADestruir->semaf_init + i));
	}
	free(datosADestruir->semaf_init);
	for(i=0 ; *(datosADestruir->shared_vars + i) != NULL ; i++ ){
		free(*(datosADestruir->shared_vars + i));
	}
	free(datosADestruir->shared_vars);
	free(datosADestruir);
}
//#undef DESTRUIR_PP solo lo puedo usar hasta aca, si lo uso en otro lado no existe

void manejar_socket_consola(int socket,t_paquete paquete){
	log_debug(logNucleo,"Llego un mensaje del socketConsola  %d. codop %d",socket,paquete.cod_op);
	switch (paquete.cod_op) {
		case HS_CONSOLA_NUCLEO:
			log_debug(logNucleo,"El socket consola %d pidio handshake",socket);
			enviar(OK_HS_CONSOLA,1,&socket,socket);
			log_debug(logNucleo,"Se respondio hanshake a socket consola %d",socket);
			//TODO agregar a lista de consolas conectadas dupla cosola-procesid
			break;
		case NUEVO_PROGRAMA:
	//		log_debug(logNucleo,"Se envio el nuevo programa a la umc con codop NUEVO_PROGRAMA");
			printf("Llego un nuevo programa del socketConsola  %d\n",socket);
			printf("El socket %d dice:\n",socket);

			armar_nuevo_pcb(paquete);

			//TODO crear PCB
			//TODO pedir espacio a UMC y enviar codigo del programa y paginas, y luego almacenar estructuras.
			//t_metadata_program * metadata;
			//metadata= metadata_desde_literal(paquete.datos);
			//Recibir codigo fuente del programa
			//crear pcb para programa (PID,PC,SP)
			//crear nuevo stack,
			//pedir umc paginas para el codigo del programa y paginas para almacenar stack
			//recibir paginas donde almacenar
			//almacenar estructuras si no puede porque no hay espacio: rechazar acceso, informar al procPrograma

		//	puts(paquete.datos); //no pasa los datos
			enviar(NUEVO_PROGRAMA,paquete.tamano_datos,paquete.datos,socket_umc);
			log_debug(logNucleo,"Se envio el nuevo programa a la umc con codop NUEVO_PROGRAMA");
			break;
		default:
			break;
	}

	//TODO atender pedidos de la consola
	/* yo recibo del programa algo asi enviar(NUEVO_PROGRAMA,size,programabuf,socket_kernel);
	 * y le tengo que devolver paquete por el socket: actualizacion = recibir_paquete(socket_kernel);
	 */
}

void cerrar_socket_consola(int socket){
	printf("Se cerro %d\n",socket);
	log_debug(logNucleo, "Se cerro %d\n",socket);
	//TODO manejar el cierre de socket de la consola
}

void nueva_conexion_consola(int socket){
	printf("Se conecto proceso Programa %d\n",socket);

}

void manejar_socket_cpu(int socket,t_paquete paquete){
	switch (paquete.cod_op) {
			case HS_CPU_NUCLEO:
				enviar(OK_HS_CPU,1,&socket,socket);
				puts("Handshake exitoso");
				break;
			case NUEVO_PROGRAMA:
				printf("Llego un pedido de conexion del socketCPU  %d\n",socket);
				printf("El socket %d dice:\n",socket);
				puts(paquete.datos); //no pasa los datos

				//
				break;
			default:
				break;
		}

	//TODO manejar pedidos del CPU
}

void cerrar_socket_cpu(int socket){
	printf("Se cerro %d\n",socket);
	//TODO manejar desconexion de un cpu
}


void nueva_conexion_cpu(int socket){
	printf("Se conecto cpu %d\n",socket);
	cpu = socket;
}


void funcion_hilo_servidor(t_estructura_server *conf_server){
	fd_set set_de_fds;
	int* fdmax = malloc(sizeof(int));
	log_debug(logNucleo,"Creando hilo servidor");
	int socketserver = crear_server_multiconexion(&set_de_fds,conf_server->puerto,fdmax);
	printf("Se creo un socket multiconexion. Su fd es: %d \n",socketserver);
	log_info(logNucleo,"Se creo un socket multiconexion. Su fd es: %d \n",socketserver);

	puts("Escuchando conexiones y corriendo!");
	correr_server_multiconexion(fdmax,&set_de_fds,socketserver,conf_server->manejar_pedido,conf_server->socket_cerrado,conf_server->conexion_nueva_aceptada);
	close(socketserver);
	free(fdmax);
}

int main(int argc, char **argv){

//Declaracion de variables Locales
	pthread_t thread_consola, thread_cpu;
	t_estructura_server conf_consola, conf_cpu;
	t_estructura_cliente conf_umc;

//Crea archivo de log
	logNucleo = crearLog();
	log_info(logNucleo, "Ejecucion del Proceso NUCLEO");

//Levanta archivo de config del proceso Nucleo
	t_config* config = config_create("../nucleo/nucleo.cfg");
	t_nucleoConfig* config_nucleo = cargarConfiguracion(config);

	log_info(logNucleo, "Configuracion Cargada");

	printf("El puerto para Proceso Consola es: %d \n", config_nucleo->puerto_prog);
	printf("El puerto para Proceso CPU es: %d \n", config_nucleo->puerto_cpu);
	printf("El puerto para Proceso UMC es: %d \n", config_nucleo->puerto_umc);

	conf_consola.conexion_nueva_aceptada = nueva_conexion_consola;
	conf_consola.manejar_pedido = manejar_socket_consola;
	conf_consola.socket_cerrado = cerrar_socket_consola;
	conf_consola.puerto=config_nucleo->puerto_prog;

	conf_cpu.conexion_nueva_aceptada=nueva_conexion_cpu;
	conf_cpu.manejar_pedido=manejar_socket_cpu;
	conf_cpu.socket_cerrado=cerrar_socket_cpu;
	conf_cpu.puerto= config_nucleo->puerto_cpu;

	//TODO conectarme con UMC
	conf_umc.puerto=config_nucleo->puerto_umc;
	conf_umc.direccion=config_nucleo->ip_umc;


//Creacion hilos para atender conexiones desde cpu/consola/ umc?

	if( (socket_umc = conectar(conf_umc.direccion, conf_umc.puerto)) == -1){
		perror("Error al crear socket de conexion con el proceso umc");
		exit(EXIT_FAILURE);
	}
	log_info(logNucleo, "Me pude conectar con proc_umc");
	handshake(socket_umc,HS_NUCLEO_UMC,OK_HS_NUCLEO);


	if (pthread_create(&thread_cpu, NULL, (void*)funcion_hilo_servidor, &conf_cpu)){
	        perror("Error el crear el thread cpu.");
	        exit(EXIT_FAILURE);
	    }
	log_info(logNucleo, "Me pude conectar con proc_cpu");

	if (pthread_create(&thread_consola, NULL, (void*)funcion_hilo_servidor, &conf_consola)){
		        perror("Error el crear el thread consola.");
		        exit(EXIT_FAILURE);
		}
	log_info(logNucleo, "Me pude conectar con proc_consola");



	//TODO planificacion de los procesos

	// TODO ante cualquiera de las fallas en alguno de los hilos, que pueda atender el otro sin esperar, ahora estaria dando la prioridad a cpu
	/* con un semaforo contador? o alguna estructura de datos compartida donde cada thread avisa si esta activo o  no
	Los ayudates dijeron que no hace falta, no van a destruir hilos porque si */

	pthread_join(thread_cpu, NULL); //el padre espera a qe termina este hilo
	pthread_join(thread_consola, NULL); //el padre espera a qe termina este hilo

	destruirNucleoConfig(config_nucleo);
	config_destroy(config);

	log_destroy(logNucleo);

	return EXIT_SUCCESS;

}
