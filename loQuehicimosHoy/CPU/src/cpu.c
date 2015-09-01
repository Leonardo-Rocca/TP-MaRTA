/*
 * cpu.c

 *
 *  Created on: 1/9/2015
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include "cpuMensajes.h"


int main(void) {
	puts("!!!Hello World!!!I am a Job");
	ipSwap = "127.0.0.1";
	puertoPlanificador = "6666";
	idProceso = 'C';
	int socketPlanificador = conectarAPlanificador(puertoPlanificador);

	puertoAdminMemoria = "6667";

	int socketAdminMemoria = conectarAPlanificador(puertoAdminMemoria);

	 //int longitud;
	char* mensaje;

	mensaje = recibirCadena(socketPlanificador);

	printf("recibi: %s \n",mensaje);

	mandarCadena(socketAdminMemoria,mensaje);
	sleep(5);
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
