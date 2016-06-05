//------------------------------------------------------------------------------------------------------
//Includes
//------------------------------------------------------------------------------------------------------
#include "Consola_Umc.h"
#include "Config_Umc.h"

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

	config_umc->retardo=milisegundos;

	printf("\nSe actualizo el retardo, el nuevo retardo es de %d milisegundos\n",config_umc->retardo);
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

	//printf("Proceso: %d",idProceso);
	//printf("Tamaño: %d",dictionary_size(tablasDePagina));
	t_entrada_diccionario* entradaD = dictionary_get(tablasDePagina,i_to_s(idProceso));
	t_list* tablaPaginas = entradaD->tablaDePaginas;

	log_info(logUMC,"Cantidad de páginas: %d",list_size(tablaPaginas));
	int i;

	printf("////////////////////////////////////////////////////\n");
	printf("----------> Reporte del proceso %d \n",idProceso);
	printf("////////////////////////////////////////////////////\n");

	for(i=0;i<list_size(tablaPaginas);i++){
		t_entrada_tabla_paginas* entPag = list_get(tablaPaginas,i);
		if((entPag->presencia) == 1){
			printf("La página %d del proceso está en memoria! \n",entPag->nro_marco);
		}
		if((entPag->uso) == 1){
			printf("La página %d del proceso está en uso! \n",entPag->nro_marco);
		}
		if((entPag->modificado) == 1){
			log_info(logUMC,"La página %d del proceso está modificado! \n",entPag->nro_marco);
		}
	}

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
	//limpiar completamente el contenido de la tabla de paginas
	printf("\nSe limpio la tabla de paginas\n");
	int cantEntradas = list_size(tlb);
	while(!list_is_empty(tlb)){
		free(list_remove(tlb,0));
	}
	crearTLB(cantEntradas);
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
	while(true){

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




