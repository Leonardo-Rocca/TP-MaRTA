/*
 ============================================================================
 Name        : Nodo.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "mensajesNodo.h";
#include <signal.h>

int main(void) {
	signal(SIGPIPE,SIG_IGN);

	pthread_t hiloConexiones;
	pthread_t hiloConFS;

	formatearArchivoDeLog();
	cargarArchivoDeConfiguracion();
	printf("tmp: %s", dirTemp);
	printf("long: %d", strlen(dirTemp));

	tamanioDeEspacioDeDatos = atoi(tamanioNodo);

	cambiarCondicionANodoViejo();

	printf("%s, %s\n", ipNodo, puertoNodo);

	pthread_create(&hiloConFS,NULL,manejoConexionesConFS,NULL);
	pthread_create(&hiloConexiones,NULL,manejoConexionesNodosYJobs,NULL);

	pthread_join(hiloConexiones,NULL);
	pthread_join(hiloConFS,NULL);


	return EXIT_SUCCESS;
}

//CONECTA AL FILESYSTEM-----------------------------------------------

int fileSystemSocket;

/*char* puertoFileSystem= "8888";
char* ipFileSystem = "127.0.0.1";*/

int conectarAFileSystem(){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ipFileSystem, puertoFileSystem, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion


	fileSystemSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	int selogroConectar = 0;
	char* nombreProcesoAConectarse = "FileSystem";

	if(connect(fileSystemSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)!= -1){
		selogroConectar = 0;
		//loguearConexionConProceso(nombreProcesoAConectarse,selogroConectar, fileSystemSocket);

		printf("Conectado con File System\n");
	}else{

		selogroConectar = -1;
		//loguearConexionConProceso(nombreProcesoAConectarse,selogroConectar,0);
	}
	freeaddrinfo(serverInfo);

	char saludo ='N';
	send(fileSystemSocket,&saludo,sizeof(char),0); //Enviar Saludo

	int tamanioIp = strlen(ipNodo) + 1;
	int tamanioPuerto = strlen(puertoNodo) + 1;

	send(fileSystemSocket,&tamanioIp,sizeof(int),0);
	send(fileSystemSocket,ipNodo,tamanioIp,0);
	send(fileSystemSocket,&tamanioPuerto,sizeof(int),0);
	send(fileSystemSocket,puertoNodo,tamanioPuerto,0);

	send(fileSystemSocket,&tamanioDeEspacioDeDatos,sizeof(unsigned int),0); //Enviar tamanioDeAlmacenamiento

	return fileSystemSocket;
}

//RECIBIR Y DESERIALIZAR PEDIDO DEL JOB--------------------------------------------------


int recibirYdeserializarBloques(t_Package* package,int socket_FS){
	int status;
	int buffer_size;
	char *buffer = malloc(buffer_size = sizeof(uint32_t));

	recibir(socket_FS, &(package->nroBloque), sizeof(int));

	uint32_t bloque_long;
	status = recv(socket_FS, buffer, sizeof(package->contenidoBloque_long),0);
	memcpy(&(bloque_long),buffer,buffer_size);
	if(!status) return 0;

	package->contenidoBloque = malloc(bloque_long);

	status = recibir(socket_FS, package->contenidoBloque, bloque_long);
	if(!status) return 0;
	//loguearEscrituraEnBloque(package->nroBloque);
	free(buffer);

	return status;
}

//MANEJA LAS CONEXIONES CON FS. --------------------------

void* manejoConexionesConFS(void* parametro){
	cargarArchivoDeConfiguracion();
	char* espacioTemporal = "espacioTemporal.txt";
	int fd_archivo = open(espacioTemporal,O_RDONLY);
	int tamanioArchivo = tamanio_archivo(fd_archivo);
	close(fd_archivo);
	FILE* archivoAbierto;
	if(nodoNuevo == "N"){
		archivoAbierto = fopen(espacioTemporal,"w+");
	} else {
	archivoAbierto = fopen(espacioTemporal,"r+");
	}
	int indice = 0;

	int socket_FS = conectarAFileSystem();
	char idProceso;
	char idOperacion;
	t_Package* pack = malloc(sizeof(t_Package));
	while(recv(socket_FS,&idProceso,sizeof(char),0)!=-1){
			if(idProceso=='F'){
				recv(socket_FS,&idOperacion,sizeof(char),0);
				printf("%c",idOperacion);
				if(idOperacion=='B'){
					indice++;
					recibirYdeserializarBloques(pack,socket_FS);
					guardarContenidoEnEspacioTemporalDelNodo(archivoAbierto,pack->contenidoBloque,pack->nroBloque,tamanioArchivo);
					//printf("Contenido: %s\n NroBloque: %d \n",pack->contenidoBloque,pack->nroBloque);
					free(pack->contenidoBloque);
				}
				if(idOperacion=='A'){
					conseguirYEnviarLoQuePideElFS(socket_FS); //lo qye le pide marta
				}
				if(idOperacion=='Z'){
					conseguirArchivoFinal(socket_FS);
				}
				if(idOperacion=='G'){
					conseguirBloqueYEnviarAlFS(socket_FS);
				}
				if(idOperacion=='D'){
					deletearBloque(socket_FS,archivoAbierto);
				}


			}
	}


	free(pack);
	fclose(archivoAbierto);
	return 0;
}


//MANEJA LAS CONEXIONES CON LOS NODOS.  ------------------------

int colaEspera = 5;

void* manejoConexionesNodosYJobs(void* parametro){
	int socket_Nodo , socketCliente , c , *new_sock;
	struct sockaddr_in server , client;

	socketsConectados = list_create();

	//Crea Socket
	socket_Nodo = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_Nodo == -1)
	{
		printf("Could not create socket");
	}

	int puerto = atoi(puertoNodo);

	//Prepara la estructura sockaddr_in
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(puerto);


	    //Bind
	    if( bind(socket_Nodo,(struct sockaddr *)&server , sizeof(server)) < 0)
	    {
	        //print the error message
	        perror("bind failed. Error");
	    }


		printf("%d",ntohs(server.sin_port));

	    //Listen
	    listen(socket_Nodo , colaEspera);

	    //Accept and incoming connection
	    c = sizeof(struct sockaddr_in);
	    char* nombreProceso;

	    while( (socketCliente = accept(socket_Nodo, (struct sockaddr *)&client, (socklen_t*)&c)) )
	    {
	    	char mensajeBienvenida;
	    	recv(socketCliente,&mensajeBienvenida,sizeof(char),0);
	    	if(mensajeBienvenida=='N'){
	    		nombreProceso = "Nodo";
	    		aceptacionDeProceso(nombreProceso, socketCliente);
	    		puts("Nuevo proceso. Nodo aceptado");

	    		pthread_t sniffer_thread;
	    		new_sock = malloc(1);
	    		*new_sock = socketCliente;
	    		if( pthread_create( &sniffer_thread , NULL ,  manejoHiloNodo , (void*) new_sock) < 0)
	    		{
	    			perror("could not create thread");
	    		}
	    	}

	    	if(mensajeBienvenida=='J'){
	    		nombreProceso = "Job";
	    		aceptacionDeProceso(nombreProceso, socketCliente);

	    		puts("Nuevo Proceso. Job aceptado");

	    			pthread_t sniffer_thread;
	    			new_sock = malloc(1);
	    			*new_sock = socketCliente;
	    			if( pthread_create( &sniffer_thread , NULL ,  manejoHiloJob, (void*) new_sock) < 0)
	    			{
	    				perror("could not create thread");
	    			}
	    	}

	    }

	    if (socketCliente < 0)
	    {
	    	//loguearConexionConProceso("Proceso",-1, 0);
	        perror("accept failed");
	    }

	    return 0;
}


void *manejoHiloNodo(void *socketCli)
{
    int nroSocket = *(int*)socketCli;
    int status = 1;
    char* nombreProceso = "Nodo";

    while (status){
    	status = recibirMensajeDe(nroSocket);
    }
   // loguearDesconexionDeProceso(nombreProceso,nroSocket);
    printf("Nodo Desconectado.\n");

    if(status == 0)
    {
        fflush(stdout);
    }
    else if(status == -1)
    {
        perror("recv failed");
    }

    free(socketCli);

    return 0;
}

void *manejoHiloJob(void *socketCli)
{
    int nroSocket = *(int*)socketCli;
    int status = 1;
    char* nombreProceso = "Job";


    while (status){
    	status = recibirMensajeDe(nroSocket);
    }
   // loguearDesconexionDeProceso(nombreProceso,nroSocket);
    printf("Job Desconectado.\n");

    if(status == 0)
    {
        fflush(stdout);
    }
    else if(status == -1)
    {
        perror("recv failed");
    }

    free(socketCli);

    return 0;
}


int recibirMensajeDe(int socketQueEscribe){
	char idProceso = recibirChar(socketQueEscribe);
	char idOperacion = recibirChar(socketQueEscribe);

	switch(idProceso){
		case 'J': //para mensajes de JOB
			switch(idOperacion){
			case 'M': //M DE MAP!
				recibirOrdenMap(socketQueEscribe);
				return 1;
			break;

			case 'R': //R DE REDUCE!
				recibirOrdenReduce(socketQueEscribe);
				return 1;
			break;

			case 'S': //SCRIPT
				recibirScript(socketQueEscribe);
				return 1;
			break;
			}
		break;
		case 'N': //para mensajes de NODO
			switch(idOperacion){
			case 'P':
				conseguirBloqueYEnviar(socketQueEscribe);
				return 1;
			break;

			case 'R':

			break;
			}
	}

	return 0;
}

char recibirChar(int unSocket){
	int resultado;
	char caracter;

	resultado = recv(unSocket, &caracter, sizeof(caracter), 0);
	//if (!resultado) return 0;

	return caracter;
}

//CONECTAR A OTRO NODO (NODO PADRE)

int nodoHijoSocket;

int conectarANodo(char* ipNodoHijo, char* puertoNodo){
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(ipNodoHijo, puertoNodo, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion
	nodoHijoSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	if(connect(nodoHijoSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)!= -1){
		printf("Conectado con Nodo\n");
	}
	freeaddrinfo(serverInfo);
	char saludo ='N';
	send(nodoHijoSocket,&saludo,sizeof(char),0); //Enviar Saludo
	return nodoHijoSocket;
}
