/*
 * mensajes.c
 *
 *  Created on: 16/6/2015
 *      Author: utnso
 */
#include "mensajesMarta.h"

int recibirMensajeDe(int socketQueEscribe){
	char idProceso = recibirChar(socketQueEscribe);
	char idOperacion = recibirChar(socketQueEscribe);

	switch(idProceso){
		case 'J':
			switch(idOperacion){
			case 'P': //Peticion de nodos en donde se encuentran los archivos
				printf("Se recibio el pedido del job para gestionarlo  \n "); //mensaje dammy
				gestionarPedidoDeJob(socketQueEscribe);
			break;

			case 'E': //Exito o no de la operacion de Mapper

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
	//if (!resultado) return 0;

	return caracter;
}

void pedirleAlFSQueGuardeElArchivoResultante(uint32_t idNodo, char*nombre, char* ruta){
	char idProceso = 'M';
	char idOperacion = 'F';
	uint32_t tamanioNombre = strlen(nombre)+1;
	uint32_t tamanioRuta = strlen(ruta)+1;

	enviar(socketFS,&idProceso,sizeof(char));
	enviar(socketFS,&idOperacion,sizeof(char));
	enviar(socketFS,&idNodo,sizeof(uint32_t));
	enviar(socketFS,&tamanioRuta,sizeof(uint32_t));
	enviar(socketFS,ruta,tamanioRuta);
	enviar(socketFS,&tamanioNombre,sizeof(uint32_t));
	enviar(socketFS,nombre,tamanioNombre);

}

bool escribio(void* elemento){  //se fija que un elemento de la lista de jobs conectados este en la lista de sockets.
	int* unSocket = (int*)elemento;
	return (FD_ISSET(*unSocket,&listaSockets));
}


void gestionarPedidoDeJob(socketQueEscribe){
	t_Package package;
	int seDebeReplanificar =1;
	recibirYdeserializarBloques(&package,socketQueEscribe); //validar status...
	do{
		pthread_mutex_lock(&lockEnvioSolicitudFS);
	t_list* estructuraDeTrabajo =solicitarArchivosAlFileSystem(package.rutasDeArchivos);  //YA FUNCIONA HASTA ACA. ESTA TESTEADO.
		pthread_mutex_unlock(&lockEnvioSolicitudFS);
		imprimirLista(estructuraDeTrabajo);
	if(list_size(estructuraDeTrabajo)==0){
		printf("Finaliza el Job debido a que FS no posee ninguno de los archivos solicitados\n");
		char idProceso = 'F';
		send(socketQueEscribe,&idProceso,sizeof(char),0);
		return;
	}
	t_list* estructuraDeTrabajoPlanificada = planificarNodos(estructuraDeTrabajo,package.combiner);
	imprimirLista(estructuraDeTrabajoPlanificada);
	seDebeReplanificar = mandarAEjecutarJob(estructuraDeTrabajoPlanificada,package.combiner,socketQueEscribe,package.rutaDestinoDeResultado,estructuraDeTrabajo,package.rutasDeArchivos);
//	eliminarListaEstructuraDeTrabajo(estructuraDeTrabajo);
	}while(0);//seDebeReplanificar);
}


void recibirYdeserializarBloques(t_Package* package,int socketQueEscribe){
	int status;
	int cantidadRutasArchivos;
	uint32_t longCadaRuta;

	package->rutasDeArchivos = list_create();

	recv(socketQueEscribe,&package->combiner,sizeof(int),0);
	recv(socketQueEscribe,&package->long_RutaDestino,sizeof(package->long_RutaDestino),0);
	package->rutaDestinoDeResultado = malloc(package->long_RutaDestino);
	recv(socketQueEscribe,package->rutaDestinoDeResultado,package->long_RutaDestino,0);
	recv(socketQueEscribe,&cantidadRutasArchivos,sizeof(int),0);

/*	printf("combiner: %d\n", package->combiner);
	printf("longRuta: %d\n", package->long_RutaDestino);
	printf("ruta: %s\n", package->rutaDestinoDeResultado);
	printf("cantidadRutasArchivos: %d\n", cantidadRutasArchivos);
*/
	int i;
	for(i=0;i<cantidadRutasArchivos;i++){
		recv(socketQueEscribe,&longCadaRuta,sizeof(longCadaRuta),0);
//		printf("longCadaRuta: %d\n", longCadaRuta);
		char* unaRuta = malloc(longCadaRuta);
		recv(socketQueEscribe,unaRuta,longCadaRuta,0);
	//	printf("ruta: %s \n", unaRuta);
		list_add(package->rutasDeArchivos, unaRuta); //si no funca habria que probar &package->rutaDeArchivos
	}

}

t_list* solicitarArchivosAlFileSystem(t_list* listaDeUrls){  //lista de char*

	//enviar peticion
	int tamanioTotal = 0;
	char* mensajeSerializado = serializarPedidoDeArchivosAlFileSystem(listaDeUrls, &tamanioTotal);
	enviar(socketFS,mensajeSerializado, tamanioTotal);

	//recibir respuesta
	char idProceso, idOperacion;

	recibir(socketFS, &idProceso, sizeof(char));
//	printf("IDPROCESO: %c", idProceso);
	recibir(socketFS, &idOperacion, sizeof(char));
//	printf("IDOPERACION: %c", idOperacion);

	//OJO CON LA SINCRONIZACION,  PENSAR!.

	if(idOperacion!='M'){

	t_list* estructuraTrabajo = deserializarYarmarEstructura();
	return estructuraTrabajo;

	}

	t_list* listaVacia = list_create(); //por si falla el pedido
	return listaVacia;
}

//SERIALIZACION PARA PEDIDO DE ARCHIVOS AL FILESYSTEM

char* serializarPedidoDeArchivosAlFileSystem(t_list* listaDeUrl, int* tamanioTotal){
	 int cantidadUrl = list_size(listaDeUrl);
	 int i=0;
	 while(i<cantidadUrl){
	 	char* unaUrl = list_get(listaDeUrl,i);
	 	*tamanioTotal+= sizeof(char)*(strlen(unaUrl)+1);
	 	i++;
	 }

	 *tamanioTotal+= sizeof(char)*2 + cantidadUrl*sizeof(uint32_t) + sizeof(uint32_t)*2 + 1;
	 int tamanioMensaje = *tamanioTotal - sizeof(char)*2 - sizeof(int);
	 char* serializedPackage = malloc(*tamanioTotal);

	 int offset = 0;
	 int size_to_send;


	 char idProceso='M';
	 char idOperacion='P';

	 size_to_send = sizeof(char);
	 memcpy(serializedPackage + offset,&idProceso,size_to_send );
	 offset+=size_to_send;

	 size_to_send = sizeof(char);
 	 memcpy(serializedPackage + offset,&idOperacion,size_to_send );
 	 offset+=size_to_send;

	 size_to_send = sizeof(tamanioMensaje);
	 memcpy(serializedPackage + offset,&tamanioMensaje,size_to_send );
	 offset+=size_to_send;

	 size_to_send = sizeof(cantidadUrl);
	 memcpy(serializedPackage + offset,&cantidadUrl,size_to_send );
	 offset+=size_to_send;

	 i=0;
	 int tamanioUrl;
	 while(i<cantidadUrl){
	 		char* unaUrl = (char *) list_get(listaDeUrl,i);
	 		tamanioUrl = (strlen(unaUrl)+1)*sizeof(char);

	 		size_to_send = sizeof(tamanioUrl);
	 		memcpy(serializedPackage + offset, &(tamanioUrl), size_to_send);
	 		offset+=size_to_send;

	 		size_to_send = tamanioUrl;
	 		memcpy(serializedPackage + offset, unaUrl, size_to_send);
	 		offset+=size_to_send;

 			i++;
	}

	return serializedPackage;
}


//DESERIALIZACION DE LA RESPUESTA DEL FS AL PEDIDO DE ARCHIVOS

t_list* deserializarYarmarEstructura(void){
	t_list* estructuraDeTrabajo = list_create();

	uint32_t longitud;
	int cantidadEstructuras;
	int cantidadBloques;
	int cantidadCopias;

	int inicial = 1;
	/*int longitudPaquete;
	recibir(socketFS,&longitudPaquete,sizeof(int));*/
	recibir(socketFS,&cantidadEstructuras,sizeof(int));
//	printf("cantEstruct: %d\n", cantidadEstructuras);

	//COMIENZO A ARMAR ESTRUCTURA PARA MARTA

	/*memcpy(&cantidadEstructuras,mensajeSerializado + offset,sizeof(int));
	offset+=sizeof(cantidadEstructuras);

	printf("cantEstruct: %d\n", cantidadEstructuras);
*/
	int i;
	for(i=0; i<cantidadEstructuras; i++){

		int offset = 0;
		int tamanioDeEstruc;
		recibir(socketFS,&tamanioDeEstruc,sizeof(int));


	//	printf("tamanio De Estructura: %d\n", tamanioDeEstruc);

		char* mensajeSerializado;

		if (varChanta == 1){
		mensajeSerializado = malloc(tamanioDeEstruc);
		varChanta = 0;
		}else{
			mensajeSerializado = malloc(tamanioDeEstruc);
			//mensajeSerializado = realloc(mensajeSerializado,tamanioDeEstruc);
		}
		recibir(socketFS, mensajeSerializado, tamanioDeEstruc);

		memcpy(&cantidadBloques,mensajeSerializado + offset,sizeof(cantidadBloques));
		offset+=sizeof(cantidadBloques);

	//	printf("cantBloques: %d\n", cantidadBloques);

		int j;
		for(j=0; j<cantidadBloques; j++){

			memcpy(&cantidadCopias,mensajeSerializado + offset,sizeof(cantidadCopias));
			offset+=sizeof(cantidadCopias);


	//		printf("cantCopias: %d\n", cantidadCopias);

			int k;
			for(k=0; k<cantidadCopias; k++){

				int* idNodo = malloc(sizeof(int));
				memcpy(idNodo,mensajeSerializado + offset,sizeof(int));
				offset+=sizeof(int);


//				printf("idNodo: %d\n", *idNodo);

				int ubicacionNodo = ubicacionDelNodoEnEstructuras(*idNodo, estructuraDeTrabajo);

				int tamanioIp;
				memcpy(&tamanioIp, mensajeSerializado + offset, sizeof(tamanioIp));
				offset+=sizeof(tamanioIp);

				char* ipNodo = malloc(tamanioIp);

				memcpy(ipNodo, mensajeSerializado + offset,tamanioIp);
				offset+= tamanioIp;


				int tamanioPuerto;
				memcpy(&tamanioPuerto, mensajeSerializado + offset, sizeof(tamanioPuerto));
				offset+=sizeof(tamanioPuerto);

				char* puertoNodo = malloc(tamanioPuerto);

				memcpy(puertoNodo, mensajeSerializado + offset,tamanioPuerto);
				offset+= tamanioPuerto;

				if(ubicacionNodo==-1){ //si el nodo no se agrego nunca, lo agrego.

					//creo estructura Nodo
					modelon* unNodo = malloc(sizeof(modelon));

					unNodo->idNodo = *idNodo;
					unNodo->ipNodo = malloc(tamanioIp);
					unNodo->puertoNodo = malloc(tamanioPuerto);
					strcpy(unNodo->ipNodo,ipNodo);
					strcpy(unNodo->puertoNodo,puertoNodo);

					//creo un elemento de mi estructura de trabajo
					modeloe* nodoC = malloc(sizeof(modeloe));

					nodoC->nodo = *unNodo;
					t_list* bloques = list_create();

						//armo la lista de bloques y agrego el primer bloque
						modelob* bloque = malloc(sizeof(modelob));

						memcpy(&(bloque->idBloqueNodo),mensajeSerializado + offset, sizeof(int));
						offset+= sizeof(int);

	//					printf("nroBloqueNodo: %d\n", bloque->idBloqueNodo);

						bloque->idBloqueArchivo = inicial + j;

						//armo estructura archivo para el bloque correspondiente
						archivo* unArchivo = malloc(sizeof(archivo));

							int tamanioRuta;
							memcpy(&tamanioRuta, mensajeSerializado + offset, sizeof(tamanioRuta));
							offset+=sizeof(tamanioRuta);

							unArchivo->ruta = malloc(tamanioRuta);

							memcpy(unArchivo->ruta, mensajeSerializado + offset,tamanioRuta);
							offset+= tamanioRuta;

							memcpy(&(unArchivo->tamanio), mensajeSerializado + offset, sizeof(int));
							offset+=sizeof(int);

						bloque->idArchivo = *unArchivo;

					//la agrego a la lista de bloques de dicho nodo.
					list_add(bloques, bloque);
					nodoC->bloques = bloques;

		//			printf("Datos agregados, idNodo: %d, posEnNodo: %d", unNodo->idNodo, bloque->idBloqueNodo);

					list_add(estructuraDeTrabajo,nodoC);

		//			printf("Se creo un nuevo NODOC!\n");


				}else{
					modeloe* nodoC = (modeloe*) list_get(estructuraDeTrabajo,ubicacionNodo);

					modelob* bloque = malloc(sizeof(modelob));

					memcpy(&(bloque->idBloqueNodo),mensajeSerializado + offset, sizeof(uint32_t));
					offset+= sizeof(uint32_t);


		//			printf("nroBloqueNodo: %d\n", bloque->idBloqueNodo);

					bloque->idBloqueArchivo = inicial + j;

					archivo* unArchivo = malloc(sizeof(archivo));

						int tamanioRuta;
						memcpy(&tamanioRuta, mensajeSerializado + offset, sizeof(tamanioRuta));
						offset+=sizeof(tamanioRuta);

						unArchivo->ruta = malloc(tamanioRuta);

						memcpy(unArchivo->ruta, mensajeSerializado + offset,tamanioRuta);
						offset+= tamanioRuta;

						memcpy(&(unArchivo->tamanio), mensajeSerializado + offset, sizeof(int));
						offset+=sizeof(int);

					bloque->idArchivo = *unArchivo;

					list_add(nodoC->bloques, bloque);

	//				printf("Se agrego un bloque a un NODOC ya existente\n");

				}

			}


		}
		inicial+=cantidadBloques;
		free(mensajeSerializado);
	}

	return estructuraDeTrabajo;
}



int ubicacionDelNodoEnEstructuras(int idNodo, t_list* unaLista){
	int ubicacion = -1;
	int cantElementos = list_size(unaLista);
	int i=0;
	while(i<cantElementos && ubicacion==-1){
		modeloe* nodoC = (modeloe*) list_get(unaLista,i);

		if(idNodo == nodoC->nodo.idNodo){
			ubicacion=i;
		}

		i++;
	}

	return ubicacion;
}


// ENVIA Y RECIBE TODO

int enviar(int s, char *buf, int len)
   {
       int total = 0;        // cuántos bytes hemos enviado
       int bytesleft = len; // cuántos se han quedado pendientes
       int n;
       while(total < len) {
           n = send(s, buf+total, bytesleft, 0);
   //        printf("Envio: %d\n",n);
           if (n == -1) { break; }
           total += n;
           bytesleft -= n;
       }
 //      printf("Total enviado: %d\n",total);
       return n==-1?-1:0; // devuelve -1 si hay fallo, 0 en otro caso
   }

int recibir(int s, char *buf, int len)
   {
       int total = 0;        // cuántos bytes hemos enviado
       int bytesleft = len; // cuántos se han quedado pendientes
       int n;
       while(total < len) {
           n = recv(s, buf+total, bytesleft, 0);
           if (n == -1) { break; }
           total += n;
           bytesleft -= n;
       }
//       printf("Total recibido: %d\n",total);
       return n==-1?-1:0; // devuelve -1 si hay fallo, 0 en otro caso
   }

int rdtsc()
{
	__asm__ __volatile__("rdtsc");
}


/*void* manejoConexionesConJob(void* parametro){
	t_list* socketsConectados;
	struct addrinfo hints,*serverInfo;
	int socketMarta,resultadoSelect,maximoSocket;
	socketsConectados = list_create();


	//configuracion del servidor
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(NULL, puertoMarta, &hints, &serverInfo);


	socketMarta = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol); //te crea un socket que devuelve el numero de socket

	bind(socketMarta,serverInfo->ai_addr, serverInfo->ai_addrlen); //enlaza el socket a un lugar que va a escuchar

	freeaddrinfo(serverInfo);

	FD_ZERO(&listaSockets);//inicializa la lista
	FD_SET(socketMarta,&listaSockets);//agrega el nuevo socket a la lista de sockets

	listen(socketMarta, colaEspera);// cantidad maxima de escuchas
	maximoSocket = socketMarta;
	while((resultadoSelect = select(maximoSocket+1,&listaSockets,NULL,NULL,NULL))){ //el select devuelve el socket a ejecutar
		if(FD_ISSET(socketMarta,&listaSockets)){
			struct sockaddr_in addr;
			socklen_t addrlen = sizeof(addr);
			int nuevoSocket;
			char mensajeBienvenida;
			nuevoSocket = accept(socketMarta, (struct sockaddr *) &addr, &addrlen);//falta verificar !-1
			if(recv(nuevoSocket,&mensajeBienvenida,sizeof(char),0)!=-1){
				if(mensajeBienvenida=='J'){
					FD_SET(nuevoSocket,&listaSockets);
					maximoSocket = nuevoSocket;
					list_add(socketsConectados,&maximoSocket);

					printf("conectado    \n");
					recibirMensajeDe(nuevoSocket);
					//break;//este lo agregue yo para q no sea infinito el bucle
					//cargarNuevoJob(maximoSocket);
				}else{
					printf("Error al recibir mensaje de bienvenida\n");
				}
			}else{
				int socketQueEscribe = *(int*)list_find(socketsConectados, escribio);
				recibirMensajeDe(socketQueEscribe);
			}
		}


		if(resultadoSelect<0){
			printf("error");
		}else{
			printf("desconectados \n ");
			break;
		}



	}

}
*/


//-------------------------------Archivo de Configuracion------------------------------

void cargarArchivoDeConfiguracion() {

	t_dictionary* diccionario;
	diccionario = dictionary_create();
	char* dato;
	char* clave;

	char textoLeido[200];
	FILE* archivo;


	char* config = "configMartus";
	archivo = fopen(config,"r");

	while ((feof(archivo)) == 0) {
		fgets(textoLeido, 200, archivo);

		clave = string_split(textoLeido, ",")[0];
		dato = string_split(textoLeido, ",")[1];
		dictionary_put(diccionario, clave, dato);

	}

	puertoFileSystem = obtenerDatoLimpioDelDiccionario(diccionario, "puertoFS");
	ipFileSystem = obtenerDatoLimpioDelDiccionario(diccionario, "ipFS");

	}

char* obtenerDatoLimpioDelDiccionario(t_dictionary* diccionario, char* dato) {

	char* datoProvisorio;
	char* datoLimpio;
	datoProvisorio = dictionary_get(diccionario, dato);
	datoLimpio = string_substring_until(datoProvisorio,
			(string_length(datoProvisorio) - 1));
	return datoLimpio;
}
