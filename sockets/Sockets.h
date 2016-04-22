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
	RIP_CONSOLA,
	NUEVO_PROGRAMA,
	TERMINO_BIEN_PROGRAMA,
	TERMINO_MAL_PROGRAMA,
	IMPRIMIR_TEXTO,
	IMPRIMIR_VARIABLE
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

#endif /* SOCKETS_H_ */
