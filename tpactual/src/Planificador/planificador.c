/*
 * planificador.c
 *
 *  Created on: 1/9/2015
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include "planificadorMensajes.h"

int main(void) {
	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
	pthread_t hiloConexiones;

	pthread_create(&hiloConexiones,NULL,manejoConexionesConCPUs,NULL);


	pthread_join(hiloConexiones,NULL);
	return EXIT_SUCCESS;
}

