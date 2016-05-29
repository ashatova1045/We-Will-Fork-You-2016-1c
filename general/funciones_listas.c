#include "funciones_listas.h"

int list_get_index(t_list* lista,void* element){

	int i;
	
	for(i=0;i<list_size(lista);i++){ 
		
		if( list_get(lista,i) == element)
			
			return i;
	}
	
	return -1;
}

