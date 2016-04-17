#include <stdio.h>
#include <stdlib.h>
#include <parser/metadata_program.h>
#include "/home/utnso/workspace/tp-2016-1c-We-Will-Fork-You/sockets/Sockets.h"
#include <commons/string.h>
#include <commons/collections/list.h>

int main(void) {

	//Me conecto al kernel y la umv
	int kernel = conectar("192.168.43.246", 4050);
	int umv = conectar("192.168.43.57", 4200);

	if (kernel == -1 || umv == -1)
	exit (EXIT_FAILURE);

	//Me preparo para recibir los paquetes
	t_paquete *paquete = recibir_paquete(kernel);

	puts(paquete->datos);

	//Reenvío
	enviar(1, paquete->tamano_datos, paquete->datos, umv);

	//Destruyo el paquete enviado
	destruir_paquete(paquete);

	return (EXIT_SUCCESS);
}
