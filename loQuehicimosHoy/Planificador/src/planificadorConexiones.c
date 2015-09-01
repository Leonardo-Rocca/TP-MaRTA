/*
 * planificadorConexiones.c
 *
 *  Created on: 1/9/2015
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>

#include "planificadorMensajes.h"

int colaEspera = 5;
char* puerto = "6666";

void* manejoConexionesConCPUs(void* parametro){
	int socket_Marta , socketCliente , c , *new_sock;
	struct sockaddr_in server , client;
	pthread_t sniffer_thread; //leo lo puso aca
	socketsConectados = list_create();

	//Crea Socket
	socket_Marta = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_Marta == -1)
	{
		printf("Could not create socket");
	}

	//Prepara la estructura sockaddr_in
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 6666 );                    //FALTA UN POSIBLE ATOI(PUERTO)

	    //Bind
	    if( bind(socket_Marta,(struct sockaddr *)&server , sizeof(server)) < 0)
	    {
	        //print the error message
	        perror("bind failed. Error");
	    }

	    //Listen
	    listen(socket_Marta , colaEspera);

	    //Accept and incoming connection
	    c = sizeof(struct sockaddr_in);
	    while( (socketCliente = accept(socket_Marta, (struct sockaddr *)&client, (socklen_t*)&c)) )
	    {
	    	char mensajeBienvenida;
	    	recv(socketCliente,&mensajeBienvenida,sizeof(char),0);
	    	if(mensajeBienvenida=='C'){
	    		puts("Nuevo proceso. Job aceptado");

	    //		pthread_t sniffer_thread;
	    		new_sock = malloc(1);
	    		*new_sock = socketCliente;
	    		if( pthread_create( &sniffer_thread , NULL ,  manejoHiloAdminMemoria , (void*) new_sock) < 0)
	    		{
	    			perror("could not create thread");
	    		}
	    	}

	    }

	    if (socketCliente < 0)
	    {
	        perror("accept failed");
	    }
		pthread_join(sniffer_thread,NULL);
	    return 0;
}

void *manejoHiloAdminMemoria(void *socketCli)
{
    int nroSocket = *(int*)socketCli;
    char mensajeBienvenida;
    //   recibirMensajeDe(nroSocket);

    char* mensaje = "corer Programa";
    mandarCadena(nroSocket,mensaje);


    char resultado;
	int status = recv(nroSocket,&resultado,sizeof(char),0);

	if(status == -1){
	    printf("Desconectado.\n");}

    free(socketCli);

    return 0;
}

