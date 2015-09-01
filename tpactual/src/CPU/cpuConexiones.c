/*
 * cpuConexiones.c
 *
 *  Created on: 1/9/2015
 *      Author: utnso
 */
#include "cpuMensajes.h"

char* puertoPlanificador;
char* ipSwap;

int conectarAPlanificador() {
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ipSwap, puertoPlanificador, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion

	planificadorSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);
	int seLogroConectar = 0;
	if (connect(planificadorSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)
			!= -1) {
		printf("Conectado con Planificador\n");
		//	loguearConexionConMarta(seLogroConectar,ipPlanificador,puertoPlanificador);
	}else{
		seLogroConectar = -1;
		//  loguearConexionConMarta(seLogroConectar,ipPlanificador,puertoPlanificador);
	}
	freeaddrinfo(serverInfo);

	char saludo = 'C';
	send(planificadorSocket, &saludo, sizeof(char), 0); //Enviar Saludo

	return planificadorSocket;
}
