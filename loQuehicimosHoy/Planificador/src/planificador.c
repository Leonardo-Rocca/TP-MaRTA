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
void mandarCadena(int socket, char* cadena) {
	uint32_t long_cadena = strlen(cadena)+1;
	send(socket, &long_cadena, sizeof(long_cadena), 0);
	send(socket, cadena, long_cadena, 0);
}
char* recibirCadena(int socketQueEscribe) {
	char* cadena;
	uint32_t long_cadena;
	recv(socketQueEscribe, &long_cadena, sizeof(long_cadena), 0);
	cadena = malloc(long_cadena);
	recv(socketQueEscribe, cadena, long_cadena, 0);
	return cadena;
}
