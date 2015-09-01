/*
 ============================================================================
 Name        : c.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

int main(void) {
	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
	pthread_t hiloConexiones;

	pthread_create(&hiloConexiones,NULL,manejoConexionesConCPUs,NULL);


	pthread_join(hiloConexiones,NULL);
	return EXIT_SUCCESS;
}

void* manejoConexionesConJob(void* parametro){
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
	server.sin_port = htons( 6666 );

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
	    	if(mensajeBienvenida=='J'){
	    		puts("Nuevo proceso. Job aceptado");

	    //		pthread_t sniffer_thread;
	    		new_sock = malloc(1);
	    		*new_sock = socketCliente;
	    		if( pthread_create( &sniffer_thread , NULL ,  manejoHiloJob , (void*) new_sock) < 0)
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
