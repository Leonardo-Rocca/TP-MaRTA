/*
 ============================================================================
 Name        : AdministradorMemoria.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "adminMemoriaMensajes.h"

int main(void) {
	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
	pthread_t hiloConexiones;
	pthread_t hiloConexionesSwap;

	char* puerto = "6668";
	ipSwap = "127.0.0.1";
	pthread_create(&hiloConexionesSwap,NULL,manejoConexionesConSwap,puerto);
	pthread_create(&hiloConexiones,NULL,manejoConexionesConCPUs,NULL);

	pthread_join(hiloConexionesSwap,NULL);
	pthread_join(hiloConexiones,NULL);

	return EXIT_SUCCESS;
}

int recibirMensajeDe(int nroSocket){
	char* cadena = recibirCadena(nroSocket) ;
	printf("recibi: %s \n",cadena);

	mandarCadena(swapSocket,cadena);
return 0;
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
