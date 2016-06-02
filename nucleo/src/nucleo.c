#include <parser/metadata_program.h>
#include "../../sockets/Sockets.h"
#include <commons/string.h>
#include <pthread.h>
#include <commons/log.h>
#include <errno.h>
#include <commons/config.h>
#include <stdbool.h>
#include "estados.h"
#include "../../general/pcb.h"
#include "../../general/Operaciones_umc.h"
#include "nucleo.h"


/*semaforo = malloc(sizeof(semaforo_t));
semaforo->valor = atoi(valorSemaforosArray[i]);
semaforo->cola = queue_create();
usando el atoi dentro de un while*/

int cpu;
int consola;
int socket_umc;
int pidActual=0;
t_log *logNucleo;
int tamano_pag_umc;
t_nucleoConfig* config_nucleo;


int roundup(x, y){
   int a = (x -1)/y +1;

   return a;
}

void cargar_cpu(int32_t socket){
	log_trace(logNucleo,"Cargando nueva CPU con socket %d",socket);
	t_cpu *cpu_nuevo;
	cpu_nuevo=malloc(sizeof(t_cpu));
	cpu_nuevo->socket=socket;
	cpu_nuevo->corriendo=false;
	list_add(lista_cpus_conectadas,cpu_nuevo);
	log_debug(logNucleo,"se agrego a la lista el cpu con socket %d", socket);
}

void cargar_programa(int32_t socket, int pid){
	t_consola *programa_nuevo;
	programa_nuevo=malloc(sizeof(t_consola));
	programa_nuevo->socket=socket;
	programa_nuevo->corriendo=false;
	programa_nuevo->pid=pid;
	list_add(lista_programas_actuales,programa_nuevo);
	log_debug(logNucleo,"se agrego a la lista el programa con socket %d, pid: %d", socket, programa_nuevo->pid);
}

void relacionar_cpu_programa(t_cpu *cpu, t_consola *programa, t_pcb *pcb){
	t_relacion *nueva_relacion;
	nueva_relacion=malloc(sizeof(t_relacion));
	cpu->corriendo=true;
	programa->corriendo=true;
	nueva_relacion->cpu=cpu;
	nueva_relacion->programa=programa;
	list_add(lista_relacion,nueva_relacion);
	log_info(logNucleo,"Se agrego la relacion entre cpu del socket: %d y el programa del socket: %d",cpu->socket, programa->socket);
	moverA_colaExec(pcb);
	log_debug(logNucleo, "Movi a la cola Exec el pcb con pid: %d", pcb->pid);
}

void liberar_una_relacion(t_pcb *pcb_devuelto){

	bool matchPID(void *relacion) {
		return ((t_relacion*)relacion)->programa->pid == pcb_devuelto->pid;
	}

	t_relacion *rel = list_remove_by_condition(lista_relacion,matchPID);
	rel->cpu->corriendo= false;
	rel->programa->corriendo= false;
	free(rel);

}

void liberar_una_relacion_porsocket_cpu(int socketcpu){

	bool matchsocketcpu(void *relacion) {
		return ((t_relacion*)relacion)->cpu->socket == socketcpu;
	}

	t_relacion *rel = list_remove_by_condition(lista_relacion,matchsocketcpu);
	rel->cpu->corriendo= false;
	rel->programa->corriendo= false;
	free(rel);

}

t_consola* matchear_consola_por_pid(int pid){

	bool matchPID_Consola(void *consola) {
						return ((t_consola*)consola)->pid == pid;
					}

	t_consola * programa_terminado=list_find(lista_programas_actuales, matchPID_Consola);
	return programa_terminado;
}

t_relacion* matchear_relacion_por_socketcpu(int socket){
	bool matchsocketcpurelacion(void *relacion) {
						return ((t_relacion*)relacion)->cpu->socket == socket;
					}

	return list_find(lista_relacion, matchsocketcpurelacion);
}

void elminar_consola_por_socket(int socket){
	bool matchconsola(void *consola) {
						return ((t_consola*)consola)->socket == socket;
					}

	free(list_remove_by_condition(lista_programas_actuales,matchconsola));
}

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
	datosNucleo->tamano_stack=config_get_int_value(config,"TAMANO_STACK");
	return datosNucleo;
}

/*#define DESTRUIR_PP(nucleo_param) \
		for(i=0 ; *(nucleo_param + i) != NULL ; i++ ){  \
			free(*(nucleo_param + i));					\
		}												\
		free(nucleo_param)								\
		*/


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


t_pcb* armar_nuevo_pcb (t_paquete paquete,t_metadata_program* metadata){
	int i;
	log_debug(logNucleo,"Armando pcb para programa original:\n%s\nTamano: %d bytes",paquete.datos,paquete.tamano_datos);

	t_pcb* nvopcb = malloc(sizeof(t_pcb));
	nvopcb->pid=++pidActual;
	nvopcb->pc=0;

//>>>>>>>>> Asignacion de cant de instrucciones, paginas totales e indice de codigo
	nvopcb->cant_instrucciones=metadata->instrucciones_size;

	int tamano_instrucciones = sizeof(t_posMemoria)*(nvopcb->cant_instrucciones);

	nvopcb->indice_codigo=malloc(tamano_instrucciones);

	int pagina_actual = 0;
	int offset_actual = 0;
	int tamano_pagina_restante = tamano_pag_umc;
	for(i=0;i<(metadata->instrucciones_size);i++){
		int tamano_instruccion = metadata->instrucciones_serializado[i].offset;

		t_posMemoria posicion_nueva_instruccion;
		posicion_nueva_instruccion.pag = pagina_actual;
		posicion_nueva_instruccion.offset= offset_actual;
		posicion_nueva_instruccion.size = tamano_instruccion;
		nvopcb->indice_codigo[i] = posicion_nueva_instruccion;

//		printf("pag %d offset %d size %d\n",posicion_nueva_instruccion.pag,posicion_nueva_instruccion.offset,posicion_nueva_instruccion.size);
//		printf("%.*s\n",posicion_nueva_instruccion.size,(char*)paquete.datos+metadata->instrucciones_serializado[i].start);

		if(tamano_instruccion<tamano_pagina_restante){
			offset_actual += tamano_instruccion;
		}
		else{
			pagina_actual++;
			offset_actual = tamano_instruccion - tamano_pagina_restante;
		}

		//por si el else del if anterior justo deja el offset al final de la pag
		if(offset_actual == tamano_pag_umc){
			pagina_actual++;
			offset_actual = 0;
		}

		tamano_pagina_restante = tamano_pag_umc-offset_actual;
	}

	int tamano_codigo = 0;
	for(i=0;i<nvopcb->cant_instrucciones;i++)
		tamano_codigo+= nvopcb->indice_codigo[i].size;
	log_trace(logNucleo,"Tamano del codigo parseado: %d",tamano_codigo);

	int result_pag = roundup(tamano_codigo, tamano_pag_umc);
	nvopcb->cant_pags_totales=(result_pag + (config_nucleo->tamano_stack));
	log_debug(logNucleo,"Cant paginas totales: %d",nvopcb->cant_pags_totales);
//<<<<<<<<<<<<<<<fin

//>>>>>>>>>>> Inicializacion de Etiquetas
	nvopcb->tamano_etiquetas=metadata->etiquetas_size;
	nvopcb->indice_etiquetas=malloc(nvopcb->tamano_etiquetas);
	memcpy(nvopcb->indice_etiquetas,metadata->etiquetas,nvopcb->tamano_etiquetas);
//<<<<<<<<<< fin

//>>>>>>> Inicializacion de indice de stack
	nvopcb->cant_entradas_indice_stack=0;
//	nvopcb->fin_stack=0;
//<<<<<<<< fin

	//registro_indice_stack * indice_stack= malloc(sizeof(registro_indice_stack));
	//indice_stack->posicion=0;
	//nvopcb->indice_stack=indice_stack;
	//cant var y cant argumentos??
/*
 * estructura del pcb
 *
typedef struct {
	int32_t pid; ++
	int32_t pc; ++
	int32_t cant_pags_totales;   ++

	t_posMemoria fin_stack;

	u_int32_t cant_instrucciones; ++
	t_posMemoria* indice_codigo; ++

	int32_t cant_etiquetas;	+-
	t_indice_etiq* indice_etiquetas;	+-

	u_int32_t cant_entradas_indice_stack;
	registro_indice_stack* indice_stack;
} t_pcb;


*/
	return nvopcb;
}
	//falta liberar todos los malloc todo los libera en el destruirpcb??

char* armar_codigo(t_pcb* nuevo_pcb,char* codigo,t_metadata_program* metadata){
	int i,offset=0;
	char *codigo_rta = malloc(tamano_pag_umc*(nuevo_pcb->cant_pags_totales-config_nucleo->tamano_stack));
	for(i=0;i < nuevo_pcb->cant_instrucciones;i++){
		memcpy(codigo_rta+offset,codigo+metadata->instrucciones_serializado[i].start,nuevo_pcb->indice_codigo[i].size);

//		log_trace(logNucleo,"Instruccion %d:\n%.*s",i,nuevo_pcb->indice_codigo[i].size,codigo_rta+offset);

		offset+= nuevo_pcb->indice_codigo[i].size;
	}
	//lo hago null-terminated para poder serializarlo y printearlo mas facil
	codigo_rta[offset] = '\0';
	return codigo_rta;
}

bool esta_libre(void * unaCpu){
	return !(((t_cpu*)unaCpu)->corriendo);
}

void enviar_a_cpu(){
	t_cpu *cpu_libre = list_find(lista_cpus_conectadas,esta_libre);
	if(!cpu_libre){
		return;
	}

	t_pcb *pcb_ready;
	pcb_ready=queue_pop(colaReady);
	if(!pcb_ready){
		return;
	}
	log_info(logNucleo, "Se levanto el pcb con id %d", pcb_ready->pid);

	bool matchPID(void *programa) {
		return ((t_consola*)programa)->pid == pcb_ready->pid;
	}

	t_consola *programa_alpedo = list_find(lista_programas_actuales, matchPID);
	if(!programa_alpedo){
		return;
	}

	log_debug(logNucleo,"Corriendo el PCB (PID %d) del programa (socket %d)en el CPU (socket %d)",pcb_ready->pid,programa_alpedo->socket,cpu_libre->socket);
	relacionar_cpu_programa(cpu_libre,programa_alpedo,pcb_ready);
	log_info(logNucleo, "Pude relacionar cpu con programa");

	t_pcb_serializado serializado = serializar(*pcb_ready);
	log_trace(logNucleo,"Se serializo un pcb");
	enviar(CORRER_PCB,serializado.tamanio,serializado.contenido_pcb,cpu_libre->socket);
	log_info(logNucleo,"Se envio un pcb a correr en la cpu %d",cpu_libre->socket);
}

bool inicializar_programa(t_pcb* nuevo_pcb,t_paquete paquete, t_metadata_program* metadata){
	t_pedido_inicializar pedido_inicializar;

	pedido_inicializar.idPrograma = nuevo_pcb->pid;
	pedido_inicializar.pagRequeridas = nuevo_pcb->cant_pags_totales;
	pedido_inicializar.codigo = armar_codigo(nuevo_pcb,paquete.datos,metadata);
	log_trace(logNucleo,"Arme el pedido_inicializar");

	t_pedido_inicializar_serializado *inicializarserializado = serializar_pedido_inicializar(&pedido_inicializar);
	log_trace(logNucleo,"Serialice");

	enviar(NUEVO_PROGRAMA,inicializarserializado->tamano,inicializarserializado->pedido_serializado,socket_umc);
	log_debug(logNucleo,"Pedido inicializar enviado. PID %d,paginas %d",pedido_inicializar.idPrograma,pedido_inicializar.pagRequeridas);

	free(inicializarserializado->pedido_serializado);
	free(inicializarserializado);

	t_paquete *respuesta_umc=recibir_paquete(socket_umc);
	log_info(logNucleo,"Recibi respuesta umc");

	switch (respuesta_umc->cod_op) {
		case OK:
			puts("Inicializacion correcta");
//			log_info(logNucleo,"Inicializacion correcta. UMC envio OK");
			return true;
			break;
		case NO_OK:
			puts("Inicializacion incorrecta");
			log_warning(logNucleo,"Inicializacion incorrecta. UMC envio NO_OK");
			//responder a la consola que no se puede ejecutar el programa
			//destruir lo inicializado
			break;
		case ERROR_COD_OP:
			puts("Se desconecto la umc");
			log_error(logNucleo,"Murio la UMC");
			exit(EXIT_FAILURE);
			//responder a la consola que no se puede ejecutar el programa
			//destruir lo inicializado
			break;
		default:
			break;
	}
return false;
}



void manejar_socket_consola(int socket,t_paquete paquete){
	log_debug(logNucleo,"Llego un mensaje del socketConsola  %d. codop %d",socket,paquete.cod_op);
	switch (paquete.cod_op) {
		case HS_CONSOLA_NUCLEO:
			log_debug(logNucleo,"El socket consola %d pidio handshake",socket);
			enviar(OK_HS_CONSOLA,1,&socket,socket);
			log_debug(logNucleo,"Se respondio hanshake a socket consola %d",socket);

			break;

		case NUEVO_PROGRAMA:
	//		log_debug(logNucleo,"Se envio el nuevo programa a la umc con codop NUEVO_PROGRAMA");
			printf("Llego un nuevo programa del socketConsola  %d\n",socket);

			t_metadata_program* metadata;
			metadata = metadata_desde_literal(paquete.datos);

			//Creo el pcb y lo agrego a la cola new
			t_pcb *nuevo_pcb;
			nuevo_pcb = armar_nuevo_pcb(paquete,metadata);
			log_debug(logNucleo, "Se armo el nuevo pcb, su id es: %d", nuevo_pcb->pid);
			moverA_colaNew(nuevo_pcb);

			//envio el inicializar a umc
			bool inicializo_bien = inicializar_programa(nuevo_pcb,paquete,metadata);

			metadata_destruir(metadata);

			//agrego consola a la lista
			cargar_programa(socket,nuevo_pcb->pid);

			if (inicializo_bien)
				moverA_colaReady(nuevo_pcb);

			enviar_a_cpu();
			log_debug(logNucleo,"Termino la inicializacion del programa");
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

void msleep(int usecs){
	usleep(usecs*1000);
}

void entrada_salida(t_pedido_wait *pedido){
	t_dispositivo_io* disp = dictionary_get(entradasalida,pedido->semaforo);

	pthread_mutex_lock(&disp->mutex);
	msleep(pedido->tiempo*disp->retardo);
	pthread_mutex_unlock(&disp->mutex);

	desbloquear_pcb(pedido->pcb);

	free(pedido->semaforo);
	free(pedido);

	enviar_a_cpu();
}

void manejar_socket_cpu(int socket,t_paquete paquete){
	switch (paquete.cod_op) {
			case HS_CPU_NUCLEO:
				enviar(OK_HS_CPU,1,&socket,socket);
				enviar(QUANTUM,sizeof(int32_t),&config_nucleo->quantum, socket);
				puts("Handshake exitoso");
				log_debug(logNucleo,"Handshake exitoso con cpu de socket %d",socket);
				cargar_cpu(socket);
				enviar_a_cpu();
				break;

			case FIN_QUANTUM:
				log_debug(logNucleo,"Fin quantum, recibi pcb serializado del socket: %d",socket);
				t_pcb *pcb_devuelto = deserializar(paquete.datos);
				log_debug(logNucleo,"PCB deserializado");

				moverA_colaReady(pcb_devuelto);

				liberar_una_relacion(pcb_devuelto);
				enviar_a_cpu();
				break;

			case IMPRIMIR_VARIABLE:
				log_info(logNucleo, "Recibi orden de imprimir variable");
				t_relacion *relacion_imp_var = matchear_relacion_por_socketcpu(socket);
				enviar(IMPRIMIR_VARIABLE,paquete.tamano_datos,paquete.datos,relacion_imp_var->programa->socket);
				log_debug(logNucleo,"Enviando imprimir variable a la consola");
				break;

			case IMPRIMIR_TEXTO:
				log_info(logNucleo, "Recibi orden de imprimir texto");
				t_relacion *relacion_imp_tex = matchear_relacion_por_socketcpu(socket);
				enviar(IMPRIMIR_TEXTO,paquete.tamano_datos,paquete.datos,relacion_imp_tex->programa->socket);
				log_debug(logNucleo,"Enviando imprimir texto a la consola");
				break;
			case OBTENER_VALOR:
				break;
			case GRABAR_VALOR:
				break;
			case FINALIZA_PROGRAMA:
			{
				log_debug(logNucleo,"Fin programa, recibi pcb serializado del socket: %d",socket);

				t_pcb *pcb_devuelto = deserializar(paquete.datos);
				enviar(FINALIZA_PROGRAMA,sizeof(int32_t),&(pcb_devuelto->pid),socket_umc);
				log_debug(logNucleo,"Envie a la umc el codigo de que finalizo el programa con el pid: %d", pcb_devuelto->pid );

				moverA_colaExit(pcb_devuelto);

				liberar_una_relacion(pcb_devuelto);

				t_consola* consola = matchear_consola_por_pid(pcb_devuelto->pid);

				enviar(TERMINO_BIEN_PROGRAMA,1,consola,consola->socket);
				elminar_consola_por_socket(consola->socket);
				enviar_a_cpu();

			}
				break;
			case WAIT:
				{t_pedido_wait *pedido_wait = malloc(sizeof(t_pedido_wait));
				*pedido_wait=  deserializar_wait(paquete.datos);

				log_debug(logNucleo,"WAIT del socket cpu: %d al semaforo %s",socket,pedido_wait->semaforo);

				t_semaforo* semaforo = dictionary_get(semaforos,pedido_wait->semaforo);
				if(!semaforo){
					log_error(logNucleo,"Se pidio wait a un semaforo inexistente");
					exit(EXIT_FAILURE);
				}

				semaforo->valor--;

				int codop;
				if(semaforo->valor >= 0)
					codop = OK;
				else{
					codop= NO_OK;

					queue_push(semaforo->cola,pedido_wait->pcb);

					bloquear_pcb(pedido_wait->pcb);
					liberar_una_relacion_porsocket_cpu(socket);
				}

				enviar(codop,1,&socket,socket);

				if(codop==NO_OK)
					enviar_a_cpu();

				break;
				}
			case SIGNAL:
				log_debug(logNucleo,"SIGNAL del socket cpu: %d",socket);

				t_semaforo* semaforo = dictionary_get(semaforos,paquete.datos);
				if(!semaforo){
					log_error(logNucleo,"Se pidio wait a un semaforo inexistente");
					exit(EXIT_FAILURE);
				}

				semaforo->valor++;

				t_pcb* pcb = queue_pop(semaforo->cola);
				desbloquear_pcb(pcb);
				enviar_a_cpu();

				break;
			case ENTRADA_SALIDA:
				log_debug(logNucleo,"ENTRADA_SALIDA del socket cpu: %d",socket);

				t_pedido_wait *pedido = malloc(sizeof(t_pedido_wait));
				*pedido = deserializar_wait(paquete.datos);

				liberar_una_relacion(pedido->pcb);
				bloquear_pcb(pedido->pcb);
				enviar_a_cpu();

				pthread_t thread;
				pthread_attr_t attr;
				pthread_attr_init(&attr);
				pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

				if(pthread_create(&thread,&attr,(void*)entrada_salida,pedido)){
					log_error(logNucleo,"Error al crear el hilo de ENTRADASALIDA");
					exit(EXIT_FAILURE);
				}
				pthread_attr_destroy(&attr);
				break;
			default:
				break;
		}

	//TODO manejar pedidos del CPU
}

void cerrar_socket_cpu(int socket){
	log_warning(logNucleo,"Se cerro el cpu %d\n",socket);
	printf("Se cerro el cpu %d\n",socket);

	bool matchSocket_Cpu(void *cpu) {
						return ((t_cpu*)cpu)->socket == socket;
					}

	t_cpu* cpu = list_remove_by_condition(lista_cpus_conectadas,matchSocket_Cpu);

	t_relacion* relacion = 	matchear_relacion_por_socketcpu(socket);

	if (relacion){
		log_warning(logNucleo,"El cpu que se cerro estaba corriendo el programa con PID %d",relacion->programa->pid);

		enviar(TERMINO_MAL_PROGRAMA,1,cpu,relacion->programa->socket);

		elminar_consola_por_socket(relacion->programa->socket);
		liberar_una_relacion_porsocket_cpu(cpu->socket);
	}

	free(cpu);
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

void crear_semaforos(){
	semaforos = dictionary_create();
	int i;
	char* sem_id = config_nucleo->semaf_ids[0];

	for(i=0;sem_id;i++){
		int valor_inicial = atoi(config_nucleo->semaf_init[i]);

		t_semaforo *semaforo =malloc(sizeof(t_semaforo));
		semaforo->valor = valor_inicial;
		semaforo->cola = queue_create();

		dictionary_put(semaforos,sem_id,semaforo);
		log_debug(logNucleo,"Se inicializo el semaforo %s con valor %d",sem_id,valor_inicial);

		sem_id = config_nucleo->semaf_ids[i+1];
	}
}

void crear_dispositivos_es(){
	entradasalida = dictionary_create();
	int i;
	char* dispositivo_id = config_nucleo->io_ids[0];

	for(i=0;dispositivo_id;i++){

		t_dispositivo_io *dispositivo =malloc(sizeof(t_dispositivo_io));

		dispositivo->retardo = atoi(config_nucleo->io_retardo[i]);
		dispositivo->mutex =(pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;

		dictionary_put(entradasalida,dispositivo_id,dispositivo);
		log_debug(logNucleo,"Se inicializo el dispositivo %s con retardo %d",dispositivo_id,dispositivo->retardo);

		dispositivo_id = config_nucleo->io_ids[i+1];
	}
}

int main(int argc, char **argv){

//Declaracion de variables Locales
	pthread_t thread_consola, thread_cpu;
	t_estructura_server conf_consola, conf_cpu;
	t_estructura_cliente conf_umc;
	t_paquete * paquete_umc;

//Inicializacion de listas
	crear_colas();

//Crea archivo de log
	logNucleo = crearLog();
	log_info(logNucleo, "Ejecucion del Proceso NUCLEO");

//Levanta archivo de config del proceso Nucleo
	t_config* config = config_create("../nucleo/nucleo.cfg");
	config_nucleo = cargarConfiguracion(config);

	//tamano_stack=config_nucleo->tamano_stack;

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


	conf_umc.puerto=config_nucleo->puerto_umc;
	conf_umc.direccion=config_nucleo->ip_umc;

//Inicializacion de semaforos
	crear_semaforos();
//Inicializacion de dispositivos de es
	crear_dispositivos_es();


//Creacion hilos para atender conexiones desde cpu/consola

	if( (socket_umc = conectar(conf_umc.direccion, conf_umc.puerto)) == -1){
		perror("Error al crear socket de conexion con el proceso umc");
		exit(EXIT_FAILURE);
	}
	log_info(logNucleo, "Me pude conectar con proc_umc");
	handshake(socket_umc,HS_NUCLEO_UMC,OK_HS_NUCLEO);
	paquete_umc = recibir_paquete(socket_umc);
		switch (paquete_umc->cod_op) {
			case TAMANIO_PAGINA:
				log_info(logNucleo,"Llego el tamano pagina de la umc");
				tamano_pag_umc=*((int*)paquete_umc->datos);
				log_info(logNucleo,"Tamano de pagina %d",tamano_pag_umc);
				break;
			default:
				log_error(logNucleo,"Recibi codigo incorrecto de la umc");

				break;
		}
//	tamano_pag_umc=*((int*)paquete_umc->datos);
//	log_info(logNucleo,"Tamano de pagina %d",tamano_pag_umc);

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

	dictionary_destroy_and_destroy_elements(semaforos,free);

	log_destroy(logNucleo);

	return EXIT_SUCCESS;

}
