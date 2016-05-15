#include "Config_Umc.h"
#include "Log_Umc.h"
#include "estructuras_umc.h"
#include "../../sockets/Sockets.h"
#include <stdlib.h>

void crear_estructuras(){
	//creo la memoria_principal
	int espacio_total = config_umc->cant_marcos*config_umc->marco_size;
	memoria_principal = malloc(espacio_total);
	if(!memoria_principal){
		log_error(logUMC,"Error al reservar memoria para la memoria principal");
		exit(EXIT_FAILURE);
	}
	log_info(logUMC,"Memoria principal creada. %d marcos * %d bytes = %d bytes",config_umc->cant_marcos,config_umc->marco_size,espacio_total);



	//creo la tabla de marcos
	tabla_marcos = malloc(config_umc->cant_marcos*sizeof(t_marco));
	if(!tabla_marcos){
		log_error(logUMC,"Error al reservar memoria para la tabla de marcos");
		exit(EXIT_FAILURE);
	}
	log_info(logUMC,"Tabla de marcos creada con %d marcos",config_umc->cant_marcos);



	//creo la tlb
	if(config_umc->entradas_tlb){
		tlb = malloc(config_umc->entradas_tlb * sizeof(t_info_marco));
		if(!tlb){
			log_error(logUMC,"Error al reservar memoria para la tlb");
			exit(EXIT_FAILURE);
		}
		log_info(logUMC,"TLB creada con %d entradas",config_umc->entradas_tlb);
	}
	else
		log_info(logUMC,"No se creo la TLB porque esta desactivada en el archivo de configuracion");
}

bool es_el_marco_requerido(t_marco marco, int pagina, int pid){
	return marco.pagina == pagina &&
			marco.pid == pid;
}

t_info_marco* obtener_marco(int pid, int pagina){
	int i;

	if(config_umc->entradas_tlb){
		log_debug(logUMC,"Buscando el marco con pid %d y pagina %d en la TLB...",pid,pagina);
		for(i=0;i<config_umc->entradas_tlb;i++){
			if(es_el_marco_requerido(*tlb[i].marco, pagina, pid)){
					log_debug(logUMC,"Marco encontrado: %d en la TLB!", tlb[i].posicion);
					return &tlb[i];
				}
		}
		log_debug(logUMC,"El marco no estaba en TLB");
	}

	log_debug(logUMC,"Buscando el marco con pid %d y pagina %d en la tabla de marcos...",pid,pagina);
	for(i = 0; i < config_umc->cant_marcos; i++){
			if(es_el_marco_requerido(tabla_marcos[i],pagina,pid)){
					t_info_marco *marco_pedido = malloc(sizeof(t_info_marco)); //fixme ponerlo en la tlb. algoritmo a usar?
					marco_pedido->marco = &tabla_marcos[i];
					marco_pedido->posicion = i;
					log_debug(logUMC,"Marco encontrado: %d en la tabla de marcos!", i);
					return marco_pedido;
			}
	}
	log_debug(logUMC,"El marco no estaba en la tabla de marcos");

	//todo pedir a swap. ponerlo en la tabla de marcos. algoritmo a usar?
	return tlb; //fixme
}

char* obtener_posicion_real(int pid, int pagina,int offset_en_bytes){
	t_info_marco * marco = obtener_marco(pid,pagina);
	char* posicion_real = obtener_posicion_real_desde_marco(marco,offset_en_bytes);
	log_debug(logUMC,"Posicion real obtenida.\npid: %d\npagina: %d\noffset: %d\nposicion real:%p",pid,pagina, offset_en_bytes,posicion_real);
	return posicion_real;
}

char* obtener_posicion_real_desde_marco(t_info_marco* marco_info, int offset_en_bytes){
	return memoria_principal + marco_info->posicion * config_umc->marco_size + offset_en_bytes;
}

void destruir_estructuras(){
	free(memoria_principal);
	free(tabla_marcos);
	free(tlb);
}
