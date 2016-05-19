#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <stdlib.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>

typedef struct
{
	uint16_t cod_op;
	uint32_t tamano_datos;
	void* datos;

} __attribute__((__packed__)) t_paquete ;
#define SIZE_HEADER sizeof(uint32_t) + sizeof(uint16_t)

//constantes de error
#define ERROR_COD_OP 9999
#define ERROR_TAMANO 2
#define ERROR_DATOS "E"

enum cod_op{ // Si se persisten en algun lado,agregar los cod_ops al final
	NUEVO_PROGRAMA=0,
	TERMINO_BIEN_PROGRAMA=1,
	TERMINO_MAL_PROGRAMA=2,
	IMPRIMIR_TEXTO=3,
	IMPRIMIR_VARIABLE=4,
	HS_CONSOLA_NUCLEO=5,
	OK_HS_CONSOLA=6,
	HS_CPU_NUCLEO=7,
	HS_CPU_UMC=8,
	OK_HS_CPU=9,
	HS_NUCLEO_UMC=10,
	OK_HS_NUCLEO=11,
	HS_UMC_SWAP=12,
	OK_HS_UMC=13,
	CORRER_PCB=14,
	LECTURA_PAGINA=15,
	ESCRITURA_PAGINA=16,
	CAMBIO_PROCESO_ACTIVO=17,
	FINALIZA_PROGRAMA=18,
	TAMANIO_PAGINA=19,
	NO_OK=20,
	OK=21,
	BUFFER_LEIDO=22,
	QUANTUM=23,
	FIN_QUANTUM=24,
	OBTENER_VALOR=25,
	GRABAR_VALOR=26,
	WAIT=27,
	SIGNAL=28,
	ENTRADA_SALIDA=29


};

int enviar(uint16_t cod_op, int tamano_datos_en_bytes, void* datos, int destino_fd);
int conectar(char* direccion, int puerto);
int crear_socket_escucha(int puerto);
t_paquete* recibir_paquete(int socket_receptor_fd);
void destruir_paquete(t_paquete* paquete);
int aceptar_cliente(int socket_server_fd);

/**
* @NAME: correr_server_multiconexion
* @DESC: Corre indefinidamente un server multiconexion. Se recomienda siempre usarlo con crear_server_multiconexion()
* @PARAMS:
* 		fdmax - fd de mayor valor relacionado al fds que nos interese. Se recomienda pasarle exactamente lo mismo que a crear_server_multiconexion()
* 		fd_set - fds que nos interese. Se recomienda pasarle exactamente lo mismo que a crear_server_multiconexion()
* 		socket_escucha - socket que escucha conexiones. Se recomienda pasarle exactamente lo que devolvio crear_server_multiconexion()
* 		manejar_pedido - callback para cuando un socket cliente recibe un pedido. Recibe el fd del socket
* 		socket_cerrado - callback para cuando un socket cliente se cierra. Recibe el fd del socket
* 		conexion_nueva_aceptada - callback para cuando se conecta un nuevo socket cliente. Recibe el fd del socket
*/
int correr_server_multiconexion(int* fdmax,fd_set* read_fds, int socket_escucha,void (*manejar_pedido)(int,t_paquete),void (*socket_cerrado)(int),void (*conexion_nueva_aceptada)(int));

/**
* @NAME: crear_server_multiconexion
* @DESC: Crea un server multiconexion y empieza a escuchar conexiones
* @PARAMS:
* 		fdmax - fd de mayor valor relacionado al fds que nos interese. Se recomienda pasarle exactamente lo mismo que a crear_server_multiconexion()
* 		fds - fd_set* nuevo. No es necesario hacerle FD_ZERO() antes de llamar a esta funcion
* 		puerto - puerto para el socket
*/
int crear_server_multiconexion(fd_set* fds, int puerto, int* fdmax);

bool recibio_datos(int socket);
int handshake(int socket, uint16_t cop_op_cliente, uint16_t cod_op_esperado);

#endif /* SOCKETS_H_ */
