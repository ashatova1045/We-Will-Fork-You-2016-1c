/*
#include <stdio.h>
#include <string.h>
#include <parser/parser.h>
#include <parser/metadata_program.h>


int main (){



static const int CONTENIDO_VARIABLE = 20;
static const int POSICION_MEMORIA = 0x10;

	t_puntero dummy_definirVariable(t_nombre_variable variable) {
	printf("definir la variable %c\n", variable);
	return POSICION_MEMORIA;
}

t_puntero dummy_obtenerPosicionVariable(t_nombre_variable variable) {
	printf("Obtener posicion de %c\n", variable);
	return POSICION_MEMORIA;
}

t_valor_variable dummy_dereferenciar(t_puntero puntero) {
	printf("Dereferenciar %d y su valor es: %d\n", puntero, CONTENIDO_VARIABLE);
	return CONTENIDO_VARIABLE;
}

void dummy_asignar(t_puntero puntero, t_valor_variable variable) {
	printf("Asignando en %d el valor %d\n", puntero, variable);
}

void dummy_imprimir(t_valor_variable valor) {
	printf("Imprimir %d\n", valor);
}

void dummy_imprimirTexto(char* texto) {
	printf("ImprimirTexto: %s", texto);
}
	AnSISOP_funciones functions = {
			.AnSISOP_definirVariable		= dummy_definirVariable,
			.AnSISOP_obtenerPosicionVariable= dummy_obtenerPosicionVariable,
			.AnSISOP_dereferenciar			= dummy_dereferenciar,
			.AnSISOP_asignar				= dummy_asignar,
			.AnSISOP_imprimir				= dummy_imprimir,
			.AnSISOP_imprimirTexto			= dummy_imprimirTexto,

	};
	AnSISOP_kernel kernel_functions = { };





	char *s=malloc(200);
	strcpy(s,"begin\nvariables a b c\n#comentario\na=2\nend");
	t_metadata_program* md =  metadata_desde_literal(s);
	int i;
	printf("size %d\n",md->instrucciones_size);
	for (i = 0; i < md->instrucciones_size; ++i)
	{
		printf("start %d\n",(md->instrucciones_serializado+i)->start);
		printf("offset %d\n",(md->instrucciones_serializado+i)->offset);
		printf("%.*s\n",(md->instrucciones_serializado+i)->offset,s+(md->instrucciones_serializado+i)->start);
	}


//	analizadorLinea("variables a", &functions, &kernel_functions);
//	puts("hola!");
}*/