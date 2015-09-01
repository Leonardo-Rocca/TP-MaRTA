/*
 ============================================================================
 Name        : FileSystem.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "mensajes.h"


int main() {
	pthread_t hiloConexionesNodos;
	pthread_t consola;

	cargarArchivoDeConfiguracion();
	formatearArchivoDeLog();

	ptrArchivo = (struct archivos *) malloc (sizeof(struct archivos));
	aux_ptrArchivo = (struct archivos *) malloc (sizeof(struct archivos));

	pthread_create(&hiloConexionesNodos,NULL,manejoConexiones,NULL);
	pthread_create(&consola,NULL,manejoConsola,NULL);

	//formatearMDFS();
	//copiarArchivoLocalAlMDFS("/home/utnso/Escritorio/201302hourly.txt");
	//renombrarUnArchivo("/home/utnso/workspace/tp-2015-1c-sinergia/FileSystem/src/archivo", "juan");

	pthread_join(hiloConexionesNodos,NULL);
	pthread_join(manejoConsola,NULL);



	return 0;

}

//MANEJA LAS CONEXIONES CON LOS NODOS.  ------------------------

fd_set listaSockets;
int fileSystemSocket;
int colaEspera = 5;
int nroNodoGlobal = 0;

void* manejoConexiones(void* parametro){

	int estadoOperativo = atoi(cantidadDeNodos); //PONER EN ARCHIVO DE CONFIG

	int socket_FS , socketCliente , c , *new_sock;
	struct sockaddr_in server , client;

	socketsConectados = list_create();

	//Crea Socket
	socket_FS = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_FS == -1)
	{
		printf("Could not create socket");
	}

	//Prepara la estructura sockaddr_in
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( atoi(puertoFS) );

	    //Bind
	    if( bind(socket_FS,(struct sockaddr *)&server , sizeof(server)) < 0)
	    {
	        //print the error message
	        perror("bind failed. Error");
	    }

	    //Listen
	    listen(socket_FS , colaEspera);

	    printf("%d", ntohs(server.sin_port));

	    //Accept and incoming connection
	    c = sizeof(struct sockaddr_in);
	    while( (socketCliente = accept(socket_FS, (struct sockaddr *)&client, (socklen_t*)&c)) )
	    {
	    	char mensajeBienvenida;
	    	recv(socketCliente,&mensajeBienvenida,sizeof(char),0);
	    	if(mensajeBienvenida=='N'){
	    		aceptacionDeProceso("Nodo", socketCliente);
	    		puts("Nuevo proceso. Nodo aceptado\n");
	    		pthread_t sniffer_thread;
	    		//new_sock = malloc(1);
	    		//*new_sock = socketCliente;
	    		creacionNodo* unNodo = malloc(sizeof(creacionNodo));
	    		unNodo->nroSocketNodo = socketCliente;

	    		int tamanioIp;
	    		recv(socketCliente, &(tamanioIp), sizeof(int), 0);
	    		unNodo->ipNodo = malloc(tamanioIp);
	    		recv(socketCliente, unNodo->ipNodo, tamanioIp, 0);

	    		int tamanioPuerto;
	    		recv(socketCliente, &(tamanioPuerto), sizeof(int), 0);
	    		unNodo->puertoNodo = malloc(tamanioPuerto);
	    		recv(socketCliente, unNodo->puertoNodo, tamanioPuerto, 0);

	    		printf("ip: %s, puerto: %s\n", unNodo->ipNodo, unNodo->puertoNodo);

	    		if( pthread_create( &sniffer_thread , NULL ,  manejoHiloNodo , (void*) unNodo) < 0)
	    		{
	    			perror("could not create thread");
	    		}
	    	}

	    	if(mensajeBienvenida=='M'){
	    		if(list_size(socketsConectados)>=estadoOperativo){ //estadoOperativo es la cantidad de nodos (conectados) necesarios para que el FS este operativo
	    			aceptacionDeProceso("Marta", socketCliente);
	    			puts("Marta Conectado");

	    			pthread_t sniffer_thread;
	    			new_sock = malloc(1);
	    			*new_sock = socketCliente;
	    			if( pthread_create( &sniffer_thread , NULL ,  manejoHiloMarta , (void*) new_sock) < 0)
	    			{
	    				perror("could not create thread");
	    			}
	    		}else{
	    			puts("FS no esta en estado operativo");
	    			loguearComentarios("FS no esta en estado operativo");
	    		}
	    	}

	    }

	    if (socketCliente < 0)
	    {
	        perror("accept failed");
	    }

	    return 0;
}

void *manejoHiloNodo(void *estructNodo)
{
    creacionNodo* nodo = (creacionNodo*)estructNodo;
    int socketNodo = nodo->nroSocketNodo;
    int status = 1;
	unsigned int tamanioEspacioAlmacenamiento;
    recv(socketNodo,&tamanioEspacioAlmacenamiento,sizeof(unsigned int),0);

    int ubicacionNodo = seReconectoNodo(nodo->puertoNodo);
    if(ubicacionNodo==-1){
    	struct nodos* unNodo = malloc(sizeof(struct nodos));
    	unNodo->idNodo = nroNodoGlobal;
    	unNodo->nroSocket = socketNodo;
    	unNodo->estado = 1;
    	unNodo->ipNodo = malloc(15);
    	unNodo->puerto = malloc(10);
    	strcpy(unNodo->ipNodo,nodo->ipNodo);
    	strcpy(unNodo->puerto,nodo->puertoNodo);
    	cargarDatosDeBloques(unNodo,tamanioEspacioAlmacenamiento);
    	list_add(socketsConectados,unNodo);
    	nroNodoGlobal++;
    }else{
    	struct nodos* unNodo = list_get(socketsConectados,ubicacionNodo);
    	unNodo->estado = 1;
    	unNodo->nroSocket = socketNodo;
    }

	while (status){
		status = recibirMensajeDe(socketNodo);
	}

    loguearDesconexionDeProceso("Nodo");
    printf("Nodo Desconectado.\n");

    ubicacionNodo = seReconectoNodo(nodo->puertoNodo);
    struct nodos* unNodo = list_get(socketsConectados,ubicacionNodo);
    unNodo->estado = 0; //pasa a estado inactivo

    if(status == 0)
    {
        fflush(stdout);
    }
    else if(status == -1)
    {
        perror("recv failed");
    }


    return 0;
}


void *manejoHiloMarta(void *socketCli)
{
    int nroSocket = *(int*)socketCli;
    int status = 1;

    while (status){
    	status = recibirMensajeDe(nroSocket);
    }
    loguearDesconexionDeProceso("Marta");
    printf("Marta Desconectado.\n");

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
		case 'M': //Marta
			switch(idOperacion){
				case 'P':
					martaPideBloquesDeArchivos(socketQueEscribe);
					return 1;
				break;
				case 'N':
					//martaPideNombre(socketQueEscribe);
				break;
				case 'F':
					martaPideQueGuardeArchivoFinal(socketQueEscribe);
					return 1;
				break;
			}
		break;
		case 'N': //Nodos
			switch(idOperacion){
				case 'A':
					recibirArchivoDelNodo(socketQueEscribe);
					return 1;
				break;
				case 'T':
					recibirArchivoFinal(socketQueEscribe);
					return 1;
				break;
				case 'B':
					recibirBloqueDelNodo(socketQueEscribe);

					return 1;
				break;
			}
		break;
	}

	return 0;
}

char recibirChar(int unSocket){
	int resultado;
	char caracter;

	resultado = recv(unSocket, &caracter, sizeof(caracter), 0);
	if (resultado==-1) caracter='X';

	return caracter;
}


//----------------CONSOLA-----------------------


void* manejoConsola(){
	char *p;
	printf("\n1: formatearMDFS \n2: copiarArchivoAlMDFS \n3:despersistirArchivos\n ");
	while(1){
		char *cadena = (char *)malloc(sizeof(char)*25);

			printf("Ingrese un comando: ");
			scanf("%s",cadena);
			p = cadena;

		do {
		if (!strcmp(p,"despersistirArchivos") || !strcmp(p,"3")) {
			despersistirArchivos();
			break;
			}

		if (!strcmp(p,"formatearMDFS") || !strcmp(p,"1")) {  //PROBADA Y FUNCIONA
				formatearMDFS();
				break;
		}

		if (!strcmp(p,"eliminarArchivo")) {
				char* path = (char *) malloc(sizeof(char)*200);
				printf("Ingrese la ruta del archivo a eliminar:\n");
				scanf("%s", path);
				eliminarUnArchivo(path);
				printf("Se cambio el estado a: %d\n",ptrArchivo->estado); //PARA PROBAR
				free(path);
				break;
			}

		if (!strcmp(p,"renombrarArchivo")) {
				char* path = (char *) malloc(sizeof(char)*200);
				printf("Ingrese la ruta del archivo a renombrar:\n");
				scanf("%s", path);
				cadena = realloc(cadena, sizeof(char)*50);
				printf("Ingrese el nuevo nombre del archivo:\n");
				scanf("%s",cadena);
				renombrarUnArchivo(path,cadena);
				printf("Ahora el nombre es : %s\n",ptrArchivo->nombre); //PARA PROBAR
				free(path);
				break;
			}

		if (!strcmp(p,"moverArchivo")) {
				char* path = (char *) malloc(sizeof(char)*200);
				printf("Ingrese la ruta del archivo a mover:\n");
				scanf("%s", path);
				cadena = realloc(cadena, sizeof(char)*200);
				printf("Ingrese el nuevo directorio del archivo:\n");
				scanf("%s",cadena);
				moverUnArchivo(path,cadena);
				printf("Ahora su directorio padre es: %d\n",ptrArchivo->dirPadre); //PARA PROBAR
				free(path);
				break;
			}


		if (!strcmp(p,"crearDirectorio")) { //PROBADA Y FUNCIONA
				cadena = realloc(cadena, sizeof(char)*50);
				printf("Ingrese nombre del directorio:\n");
				scanf("%s",cadena);
				char* path = (char *) malloc(sizeof(char)*200);
				printf("Ingrese la ruta del directorio a crear (raiz en el caso de no poseer):\n");
				scanf("%s", path);
				crearDirectorio(cadena,path);
				free(path);
				break;
			}

		if (!strcmp(p,"eliminarDirectorio")) {  //PROBADA Y FUNCIONA
				char* path = (char *) malloc(sizeof(char)*200);
				printf("Ingrese la ruta del directorio a eliminar:\n");
				scanf("%s", path);
				eliminarDirectorio(path);
				free(path);
				break;
			}

		if (!strcmp(p,"renombrarDirectorio")) { //PROBADA Y FUNCIONA
				cadena = realloc(cadena, sizeof(char)*50);
				printf("Ingrese nuevo nombre del directorio a renombrar:\n");
				scanf("%s",cadena);
				char* path = (char *) malloc(sizeof(char)*200);
				printf("Ingrese la ruta del directorio a modificar el nombre:\n");
				scanf("%s", path);
				renombrarDirectorio(cadena,path);
				free(path);
				break;
			}

		if (!strcmp(p,"moverDirectorio")) {  //PROBADA Y FUNCIONA
				cadena = realloc(cadena, sizeof(char)*200);
				printf("Ingrese la ruta del directorio a mover:\n");
				scanf("%s",cadena);
				char* path = (char *) malloc(sizeof(char)*200);
				printf("Ingrese la nueva ruta del directorio:\n");
				scanf("%s", path);
				moverDirectorioA(cadena,path);
				free(path);
				break;
			}

		if (!strcmp(p,"copiarArchivoAlMDFS") || !strcmp(p,"2")) {
				cadena = realloc(cadena, sizeof(char)*400);
				printf("Ingrese la ruta del archivo a copiar:\n");
				scanf("%s",cadena);
				copiarArchivoLocalAlMDFS(cadena);
				break;
			}

		if (!strcmp(p,"copiarArchivoDelMDFS")) {
			char* rutaLinux = malloc(200);
			char* rutaMDFS = malloc(200);
			printf("Ingrese la ruta del archivo a copiar:\n");
			scanf("%s",rutaMDFS);
			printf("Ingrese la ruta donde guardarlo:\n");
			scanf("%s",rutaLinux);
			copiarArchivoDelMDFSAlFSLocal(rutaMDFS, rutaLinux);
			free(rutaLinux);
			free(rutaMDFS);
			break;
			}

		if (!strcmp(p,"solicitarMD5ArchivoDelMDFS")) { //FALTA
				solicitarMD5();
				break;
			}

		if (!strcmp(p,"solicitarMD5ArchivoLocal") || !strcmp(p,"5")) {
				char* rutaArchivo = malloc(100);
				char* resultado;
				printf("Ingrese la ruta del archivo:\n");
				scanf("%s",rutaArchivo);
				calcularMD5ArchivoLocal(rutaArchivo, &resultado);
				int i = 0;
				for(;i<16;i++){
					printf("%02x",resultado[i] & 0xff);
				}
				printf("\n");
				free(rutaArchivo);
				free(resultado);
				break;
			}




		if (!strcmp(p,"verBloques")) {
				verBloques();
				break;
			}

		if (!strcmp(p,"borrarBloques")) {
				borrarBloques();
				break;
			}

		if (!strcmp(p,"copiarBloques")) {
				copiarBloques();
				break;
			}

		if (!strcmp(p,"agregarNodo")) { //FALTA
				agregarNodo();
				break;
			}

		if (!strcmp(p,"eliminarNodo")) { //FALTA
				eliminarNodo();
				break;
			}


		if (!strcmp(p,"exit")){
			return EXIT_SUCCESS;
			}

		if (1) {
				printf("El comando no es valido\n");
				break;
			}
		} while(1);

		free(cadena);
	}

}



void copiarArchivoDelMDFS(){

}

void solicitarMD5(){

}


void agregarNodo(){

}

void eliminarNodo(){

}

