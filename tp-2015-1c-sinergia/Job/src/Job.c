/*
 ============================================================================
 Name        : Job.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include "mensajesJOB.h"

char* puertoMarta;
char* ipMarta;
int combiner;
char* rutaDestinoDeResultado;
char* rutaMapper;
char* rutaReducer;

pthread_mutex_t lockEnvio;
pthread_mutex_t variableGlobal;
pthread_mutex_t lockEscucharOrdenes;
pthread_mutex_t lockListaGlobal;

t_list* archivosAUtilizar;
char idProceso = 'J';
int martaSocket;
int variableGlobalJobEnEjecucion = 1;
int variableGlobalNodoCaido1=-1;
int variableGlobalNodoCaido2=-1;
int variableGlobalNodoCaido3=-1;
t_list* listaGlobalNodosCaidos;

int main(void) {
	puts("!!!Hello World!!!I am a Job"); /* prints !!!Hello World!!! */
	formatearArchivoDeLog();
	cargarArchivoDeConfiguracion(); //se cargan los parametros de configuracion en su archivo por teclado

//	printf("\n%s\n", ipMarta);
//	printf("\n%s\n", puertoMarta);
//	printf("\n%d\n", combiner);

	conectarAMarta();
	listaGlobalNodosCaidos = list_create();

	if (pthread_mutex_init(&lockEscucharOrdenes, NULL) != 0)
		{
		        printf("\n Fallo mutex Escuchar a Marta\n");
		}
	if (pthread_mutex_init(&lockListaGlobal, NULL) != 0)
		{
		        printf("\n Fallo mutex Escuchar a Marta\n");
		}

	pedirAMartaInfoParaTrabajar();
	t_list* listaConNodosAMapear;
//	do{
		variableGlobalJobEnEjecucion=1;

	char operacion;
	recv(martaSocket, &operacion, sizeof(char), 0);
	if(operacion=='F'){
		printf("Fin del JOB.\n Motivo: El/Los archivos solicitados no se encuentran disponibles\n");
		return 0;
		}

	listaConNodosAMapear = escucharOrdenesDeMarta(operacion);
	mandarALaburarALosNodos(listaConNodosAMapear);


	while(variableGlobalJobEnEjecucion){ //--ver si esto lo metemos en un hilo--


//		pthread_mutex_lock(&lockEscucharOrdenes);

		recv(martaSocket, &operacion, sizeof(char), 0);
	//	printf("Operacion Recibida: %c \n", operacion);

		if(operacion=='F'){
			printf("Fin del JOB\n");
			return 0;
			}
		if(operacion=='R'){
		t_list* listaConNodosAReducir = escucharOrdenesDeMarta(operacion); //ahora son a reducir
	//	pthread_mutex_unlock(&lockEscucharOrdenes);

		pthread_t hiloOrdenesReduce;
		pthread_create(&hiloOrdenesReduce,NULL,(void*)operacionReduce,(void*)listaConNodosAReducir);
		//int nroSocket = mandarAReducirALosNodos(listaConNodosAMapear);
		//enviarAMartaResultadoOperacionReduce(listaConNodosAMapear,nroSocket);
		}

		if(operacion=='M'){
			listaConNodosAMapear = escucharOrdenesDeMarta(operacion);
			mandarALaburarALosNodos(listaConNodosAMapear);
		}

	}

//	list_destroy_and_destroy_elements(listaConNodosAMapear, (void*) nodos_destroy);

//	}while(variableGlobalJobEnEjecucion==0);
	return 0;
}

void* operacionReduce(t_list* listaConNodosAReducir){
	int nroSocket = mandarAReducirALosNodos(listaConNodosAReducir);
	char resultadoDeOperacionDelNodo = recibirResultadoDelNodo(nroSocket);
loguearCreacionDeHiloReduce(((estructura*)(list_get(listaConNodosAReducir,0)))->nodo.ipNodo );

	pthread_mutex_lock(&lockEnvio);
	enviarAMartaResultadoOperacionReduce(listaConNodosAReducir,nroSocket,resultadoDeOperacionDelNodo);
	pthread_mutex_unlock(&lockEnvio);

	list_destroy_and_destroy_elements(listaConNodosAReducir, (void*) nodos_destroy);
	loguearFinalizacionDeHiloScript("Reduce");
	return 0;
}
int conectarAMarta() {
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ipMarta, puertoMarta, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion

	martaSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);
	int seLogroConectar = 0;
	if (connect(martaSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)
			!= -1) {
		printf("Conectado con Marta\n");
		loguearConexionConMarta(seLogroConectar,ipMarta,puertoMarta);
	}else{
		seLogroConectar = -1;
		loguearConexionConMarta(seLogroConectar,ipMarta,puertoMarta);
	}
	freeaddrinfo(serverInfo);

	char saludo = 'J';
	send(martaSocket, &saludo, sizeof(char), 0); //Enviar Saludo

	return martaSocket;
}


void pedirAMartaInfoParaTrabajar() {
	char idOperacion = 'P';
	send(martaSocket, &idProceso, sizeof(char), 0);	 //Enviar identificacion
	send(martaSocket, &idOperacion, sizeof(char), 0);

	send(martaSocket, &combiner, sizeof(int), 0);
	uint32_t long_RutaDestino = strlen(rutaDestinoDeResultado)+1;
	send(martaSocket, &long_RutaDestino, sizeof(long_RutaDestino), 0);
	send(martaSocket, rutaDestinoDeResultado, long_RutaDestino, 0);

	int cantidadRutasArchivos = list_size(archivosAUtilizar);
	send(martaSocket, &cantidadRutasArchivos, sizeof(int), 0);
	int i;
	for (i = 0; i < cantidadRutasArchivos; i++) {
		char* cadena = list_get(archivosAUtilizar, i);
		mandarCadena(martaSocket, cadena);
	}


}
//-- escucha tanto ordenes de map como de reduce----
t_list* escucharOrdenesDeMarta(char operacion) {

	printf("Operacion a realizar: ");
	if(operacion=='M'){printf(" %capp\n", operacion);}else{printf("%ceduce\n", operacion);}
	loguearCabecerasDeMensajes(operacion);
	int cantidadDeNodos;
	recv(martaSocket, &cantidadDeNodos, sizeof(int), 0);
	int i = 0;
	t_list* listaDeNodos = list_create();

	for (i; i < cantidadDeNodos; i++) {
		estructura* estructuraDeTrabajo = malloc(sizeof(estructura));
		nodo nodoAMappear;

		recv(martaSocket, &(nodoAMappear.idNodo), sizeof(int), 0);

		int tamanioIp;
		recv(martaSocket, &(tamanioIp), sizeof(int), 0);
		nodoAMappear.ipNodo = malloc(tamanioIp);
		recv(martaSocket, nodoAMappear.ipNodo, tamanioIp, 0);

		int tamanioPuerto;
		recv(martaSocket, &(tamanioPuerto), sizeof(int), 0);
		nodoAMappear.puertoNodo = malloc(tamanioPuerto);
		recv(martaSocket, nodoAMappear.puertoNodo, tamanioPuerto, 0);

		(*estructuraDeTrabajo).nodo = nodoAMappear;
		(*estructuraDeTrabajo).bloques = list_create();
//		printf(" voy a mapear nodo : %d\n", nodoAMappear.idNodo);
		escucharBloques(operacion,estructuraDeTrabajo);

		list_add(listaDeNodos, estructuraDeTrabajo);
	}
	if(operacion == 'R'){
		char* nombre = recibirCadena(martaSocket);
		estructura* unaEstructura = list_get(listaDeNodos,0);
		unaEstructura->nombreDelReduce=nombre;
	}
	return listaDeNodos;
}

void escucharBloques(char id,estructura* estructuraDeTrabajo){
	int cantidadDeBloques;
	recv(martaSocket, &cantidadDeBloques, sizeof(int), 0);
	int j = 0;
	char* nombre;

	if(id=='M'){
		for (j; j < cantidadDeBloques; j++) {

		int idBloque;
		recv(martaSocket, &idBloque, sizeof(int), 0);
//		printf("  bloque : %d\n", idBloque);

		bloqueJob* bloqueJobAux = malloc(sizeof(bloqueJob));
		(*bloqueJobAux).idBloqueNodo = idBloque;
		nombre= recibirCadena(martaSocket); //nombre con el q el nodo lo guarda
		(*bloqueJobAux).nombreEnDondeGuardarlo = nombre;
//		printf("nombre: %s\n\n", nombre);
		list_add(estructuraDeTrabajo->bloques, bloqueJobAux);
		}
	}else{ //'R'
		for(j; j < cantidadDeBloques; j++) {
			 nombre = recibirCadena(martaSocket); //nombre de donde sacarlo
			 bloqueJob* bloqueJobAux = malloc(sizeof(bloqueJob));
			 (*bloqueJobAux).nombreEnDondeGuardarlo = nombre; //nombre de donde lo saca
			 list_add(estructuraDeTrabajo->bloques,bloqueJobAux);
		}

		if(cantidadDeBloques==0){
			nombre = recibirCadena(martaSocket); //nombre de donde sacarlo
			bloqueJob* bloqueJobAux = malloc(sizeof(bloqueJob));
			(*bloqueJobAux).nombreEnDondeGuardarlo = nombre; //nombre de donde lo saca
			list_add(estructuraDeTrabajo->bloques,bloqueJobAux);
		}


	 }
}
//--hago la del map
void enviarAMartaResultadoOperacion(envioMap* envio, char resultadoDeOperacionDelNodo){
	//validar resultado exitoso o no            TO DO!!    aca tmb se va a loguear dependiendo de si fue exitoso o no el mapeo!!

	/* VIEJA replanificacion
	if(resultadoDeOperacionDelNodo=='X' && variableGlobalJobEnEjecucion==1){
		pthread_mutex_lock(&variableGlobal);
		variableGlobalJobEnEjecucion=0; //se debe rep-lanificar
		pthread_mutex_unlock(&variableGlobal);
	}else{ //var globlar esta en 0 o en -1, no debo informar nada
		if(resultadoDeOperacionDelNodo=='X' ){ // si ya fallo un map y falla otro, no envio nada.
			return;
		}

		if(resultadoDeOperacionDelNodo=='E' && variableGlobalJobEnEjecucion==0){
			return;
		}
		//no se debe informar nada ya que lo que se cayó es un reduce
	}*/
//	printf("\n Le contesta a marta \n");
	//Nueva re-planificacion
	estructura* nodo = envio->unaEstructura;
	int id = (nodo->nodo).idNodo;

//	printf("id: %d\n", id);
//	printf("resultadoOp: %c\n", resultadoDeOperacionDelNodo);

	if(resultadoDeOperacionDelNodo=='X' && elem(listaGlobalNodosCaidos,id) /*(variableGlobalNodoCaido1==id || variableGlobalNodoCaido2==id  ||  variableGlobalNodoCaido3==id)*/){
		printf("\n vino un hilo y no contesto\n",id);
	return;
	}

	//char resultado = 'X'; printf("\n%cxito\n", resultado);
	send(martaSocket, &resultadoDeOperacionDelNodo, sizeof(char), 0);
	char operacionQueRealizo ='M'; 					//la M Y E las tiene q sacar de algun lado
	send(martaSocket, &operacionQueRealizo, sizeof(char), 0);

	if(resultadoDeOperacionDelNodo=='E' /*&& variableGlobalJobEnEjecucion==1*/){

		send(martaSocket, &id, sizeof(int), 0);
		printf("\n Finalize una operacion en nodo cuya ip es : %s\n", (nodo->nodo).ipNodo);

		bloqueJob* bloque = list_get(nodo->bloques,0);

	//	printf("y lo guarde en :%s", bloque->nombreEnDondeGuardarlo);
		mandarCadena(martaSocket,bloque->nombreEnDondeGuardarlo);
	}

	if(resultadoDeOperacionDelNodo=='X' ){
	//	int id = (nodo->nodo).idNodo;
		printf("\n Fallo nodo con ip: %s   \n",(nodo->nodo).ipNodo);
		send(martaSocket, &id, sizeof(int), 0);
		pthread_mutex_lock(&lockListaGlobal);
		agregarAListaGlobal(id);
		pthread_mutex_unlock(&lockListaGlobal);
//		cambiarVariableGlobal(id);
	//	manejarReplanificacionDeMap();
	}

	return;
}
int elem(t_list* lista, int elemento){
	int i =0;
	int* elementoDeLaLista;
	while(i<list_size(lista)){
		elementoDeLaLista = list_get(lista,i);
		if(elemento==(*elementoDeLaLista) )return 1;
		i++;
	}
	return 0;
}
int elem2(t_list* lista, int elemento){ //no se usa
	int i =0;
	int elementoDeLaLista;
	while(i<list_size(lista)){
		elementoDeLaLista = list_get(lista,i);
		if(elemento==(elementoDeLaLista) )return 1;
		i++;
	}
	return 0;
}
void agregarAListaGlobal(int id){
	int* nro = malloc(sizeof(int));
	*nro = id;
	list_add(listaGlobalNodosCaidos,nro);
}
//poco extensible, no funciona si se caen mas de 2 nodos
int cambiarVariableGlobal(int id){
	if(variableGlobalNodoCaido1==-1){
		variableGlobalNodoCaido1 = id;}
	else{
		if(variableGlobalNodoCaido2==-1){
			variableGlobalNodoCaido2= id;
		}else{
			variableGlobalNodoCaido3= id;
		}
	}
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

////////CONFIGURACION///////////

 void cargarArchivoDeConfiguracion() {

	t_dictionary* diccionario;
	diccionario = dictionary_create();
	char* dato;
	char* clave;

	char textoLeido[200];
	FILE* archivo;
	//archivo = fopen("/home/utnso/workspace/tp-2015-1c-sinergia/Job/src/configJob","r");
	//archivo = fopen("/home/utnso/git/tp-2015-1c-sinergia/Job/src/configJob","r");

	char* config = "configJob";
	archivo = fopen(config,"r");

	while ((feof(archivo)) == 0) {
		fgets(textoLeido, 200, archivo);

		clave = string_split(textoLeido, ",")[0];
		dato = string_split(textoLeido, ",")[1];
		dictionary_put(diccionario, clave, dato);

	}
	//fclose(archivo);

	conseguiRutaArchivos(diccionario);

	/*printf(dictionary_get(diccionario, "ip"));//cada dato tiene que ser cargado en una variable en ves de imprimirse, ej: ipMarta = dictionary_get(diccionario,"ip");
	printf(dictionary_get(diccionario, "puerto"));
	printf(dictionary_get(diccionario, "mapper"));
	printf(dictionary_get(diccionario, "reducer"));
	printf(dictionary_get(diccionario, "combiner"));
	printf(dictionary_get(diccionario, "resultado"));*/

	//IMPORTANTE hay que hacer esto que viene abajo pq el dato obtenido del diccionario viene con un caracter
	// de mas entonces tenemos que "limpiarlo" (se lo sacamos)


	puertoMarta = obtenerDatoLimpioDelDiccionario(diccionario, "puerto");
	ipMarta = obtenerDatoLimpioDelDiccionario(diccionario, "ip");
	rutaMapper = obtenerDatoLimpioDelDiccionario(diccionario, "mapper");
	rutaReducer = obtenerDatoLimpioDelDiccionario(diccionario, "reducer");
	char* combinerComoString = obtenerDatoLimpioDelDiccionario(diccionario, "combiner");
	rutaDestinoDeResultado = obtenerDatoLimpioDelDiccionario(diccionario, "resultado");
	if(!strcmp(combinerComoString,"S")){combiner =1;}else{combiner=0;}
}

void conseguiRutaArchivos(t_dictionary* diccionario) {
	int x = 0;
	int i = 1;
	bool existe;
	archivosAUtilizar = list_create();

	while (x == 0) {

		char *unaPalabra = string_new();
		string_append(&unaPalabra, "archivo");
		string_append(&unaPalabra, string_itoa(i));

		existe = dictionary_has_key(diccionario, unaPalabra);

		if (existe == 1) {
	//		printf(dictionary_get(diccionario, unaPalabra)); //cada ruta de archivo tiene que ser cargada en una lista en ves de imprimirse.
			list_add(archivosAUtilizar,
					obtenerDatoLimpioDelDiccionario(diccionario, unaPalabra));
			i++;
		} else {
			x = 1;
		}
	}

}

char* obtenerDatoLimpioDelDiccionario(t_dictionary* diccionario, char* dato) {

	char* datoProvisorio;
	char* datoLimpio;
	datoProvisorio = dictionary_get(diccionario, dato);
	datoLimpio = string_substring_until(datoProvisorio,
			(string_length(datoProvisorio) - 1));
	return datoLimpio;
}



// TODO MAP!!!



void mandarALaburarALosNodos(t_list* listaDeNodosAMapear){
	if (pthread_mutex_init(&lockEnvio, NULL) != 0)
	{
	        printf("\n Fallo mutex Envio a Marta\n");
	}

	if (pthread_mutex_init(&variableGlobal, NULL) != 0)
	{
		    printf("\n Fallo mutex de variable global\n");
	}

	int i;
	int cantidadDeNodos;
	cantidadDeNodos = list_size(listaDeNodosAMapear);
	printf("\n Mando a mappear nodos\n");
	for (i=0;i<cantidadDeNodos;i++){
		estructura* unNodoConSusBloques;
		unNodoConSusBloques = list_get(listaDeNodosAMapear,i);
		hacerLaburarAlNodo(unNodoConSusBloques); //prueba sin hilos
		loguearCreacionDeHilosMapper(unNodoConSusBloques->nodo.ipNodo);
	}

}


static void nodos_destroy(estructura *self) {
    free(self->nodo.ipNodo);
    free(self->nodo.puertoNodo);
    list_destroy_and_destroy_elements(self->bloques, (void*) bloquejob_destroy);
    free(self);
}

static void bloquejob_destroy(bloqueJob *self) {
	free(self);
}



void* hacerLaburarAlNodo(estructura* unNodoConSusBloques){
	int cantidadBloques = list_size((unNodoConSusBloques->bloques));
	int i=0;


	for(i;i<cantidadBloques;i++){
			bloqueJob* blok = list_get((unNodoConSusBloques->bloques),i);
			estructura* unaEstructura=malloc(sizeof(estructura));
			unaEstructura->nodo=unNodoConSusBloques->nodo;
			unaEstructura->bloques=list_create();
			list_add(unaEstructura->bloques,blok);
			unaEstructura->nombreDelReduce = unNodoConSusBloques->nombreDelReduce;
			envioMap* unEnvio = malloc(sizeof(envioMap));
			//unEnvio->nroSocket = socketNodo;

			unEnvio->unaEstructura = unaEstructura;

			pthread_t hiloConexionNodoPorBloque;
			pthread_create(&hiloConexionNodoPorBloque,NULL,(void*)hacerLaburarAlNodoPorBloque,(void*)unEnvio);
			//hacerLaburarAlNodoPorBloque(unEnvio); //prueba sin hilos
	}
//borrar estructura

return 0;
}

void* hacerLaburarAlNodoPorBloque(envioMap* unEnvio){

	char* puertoNodo = unEnvio->unaEstructura->nodo.puertoNodo;
	char* ipNodo = unEnvio->unaEstructura->nodo.ipNodo;
	int socketNodo = conectarConNodo(puertoNodo,ipNodo);

	if(socketNodo!=-1){
//	printf("puerto: %s, ip: %s, socket:%d , idNodo: %d",puertoNodo, ipNodo,socketNodo,unEnvio->unaEstructura->nodo.idNodo);
	envioDeScriptMap(socketNodo);

	enviarOperacionMapANodos(socketNodo, unEnvio->unaEstructura); //previamente se le envio el script.
	}

	char resultadoOperacionDelNodo = recibirResultadoDelNodo(socketNodo);

	pthread_mutex_lock(&lockEnvio);
	enviarAMartaResultadoOperacion(unEnvio,resultadoOperacionDelNodo);
	pthread_mutex_unlock(&lockEnvio);

	free(unEnvio->unaEstructura);
	list_destroy_and_destroy_elements(unEnvio->unaEstructura->bloques, (void*) bloquejob_destroy);
	free(unEnvio);

	loguearFinalizacionDeHiloScript("Mapp");

	return 0;
}
char recibirResultadoDelNodo(int socketNodo){
	char resultado;
	int status = recv(socketNodo,&resultado,sizeof(char),0);
	if(status == -1){
	//	pthread_mutex_lock(&variableGlobal);
	//	variableGlobalJobEnEjecucion = 0;
	//	pthread_mutex_unlock(&variableGlobal);
		return 'X';
	}
	return  resultado;//'E';
}

void enviarOperacionMapANodos(int socketNodo, estructura* unNodoConSusBloques){
	int longitudMensaje = 0 ;
	char* mensajeSerializado = serializarEnvioDeOperacionMap(unNodoConSusBloques,&longitudMensaje);
	enviar(socketNodo,mensajeSerializado,longitudMensaje);
	free(mensajeSerializado);
}


char* serializarEnvioDeOperacionMap(estructura* unNodoConSusBloques, int* longitudMensaje){

	bloqueJob* unBloque = list_get(unNodoConSusBloques->bloques,0);
	int tamanioNroBloqueEnNodo = sizeof(uint32_t);
	int tamanioNombreEnDondeGuardarlo = strlen(unBloque->nombreEnDondeGuardarlo)+1;
	*longitudMensaje = 2*sizeof(char)+tamanioNroBloqueEnNodo+sizeof(tamanioNombreEnDondeGuardarlo)+tamanioNombreEnDondeGuardarlo + 1;

	char *serializedPackage = malloc(*longitudMensaje);

	int offset = 0;
	int size_to_send;

	char idProceso='J';
	char idOperacion='M'; //M de MAP

	size_to_send = sizeof(char);
	memcpy(serializedPackage + offset,&idProceso,size_to_send );
	offset+=size_to_send;

	size_to_send = sizeof(char);
	memcpy(serializedPackage + offset,&idOperacion,size_to_send );
	offset+=size_to_send;

	size_to_send = tamanioNroBloqueEnNodo;
	memcpy(serializedPackage + offset, &(unBloque->idBloqueNodo), size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(tamanioNombreEnDondeGuardarlo);
	memcpy(serializedPackage + offset, &tamanioNombreEnDondeGuardarlo, size_to_send);
	offset += size_to_send;

	size_to_send = tamanioNombreEnDondeGuardarlo;
	memcpy(serializedPackage + offset, unBloque->nombreEnDondeGuardarlo,size_to_send);

	return serializedPackage;

}

void envioDeScriptMap(int socketNodo){

	int fd_archivo = open(rutaMapper,O_RDONLY); //agregar al archivo de config!!
	char* mapeo;
	int tamanioArchivo = tamanio_archivo(fd_archivo);

	if( (mapeo = mmap(NULL, tamanioArchivo, PROT_READ, MAP_SHARED, fd_archivo, 0 )) == MAP_FAILED){
		//Si no se pudo ejecutar el MMAP, imprimir el error y aborta;
		printf("Error con MMAP");
		abort();
	}

	char idProceso = 'J';
	char idOperacion = 'S';
	char tipoScript = 'm';
	int tamanioEnvio = tamanioArchivo + 1;

	enviar(socketNodo,&idProceso,sizeof(char));
	enviar(socketNodo,&idOperacion,sizeof(char));
	enviar(socketNodo,&tipoScript,sizeof(char));
	enviar(socketNodo,&tamanioEnvio,sizeof(uint32_t));
	enviar(socketNodo,mapeo,tamanioEnvio);

	munmap(mapeo,tamanioArchivo);
}

int tamanio_archivo(int fd){
	struct stat buf; //se crea una estructura buf de tipo stat(estructura que tiene un fd).
	fstat(fd, &buf);  //un poco de magia, "arma" la estructura que llamamos buf con el fd pasado por parametro.
	return buf.st_size; //retorna el tamanio del archivo.
}


// TODO REDUCE!!!


int mandarAReducirALosNodos(t_list* listaDeNodosAReducir){
	estructura* nodoMacho = list_get(listaDeNodosAReducir,0);

	printf("Mando a reducir \n");
	char* puertoNodo = (nodoMacho->nodo).puertoNodo;
	char* ipNodo = (nodoMacho->nodo).ipNodo;
	int socketNodo = conectarConNodo(puertoNodo,ipNodo);
	envioDeScriptReduce(socketNodo);

	char* nombreEnDondeGuardarlo = nodoMacho->nombreDelReduce;
	uint32_t longNombreEnDondeGuardarlo = strlen(nombreEnDondeGuardarlo) + 1;

	char idProceso='J';
	char idOperacion='R'; //R DE REDUCE

	enviar(socketNodo,&idProceso,sizeof(char));
	enviar(socketNodo,&idOperacion,sizeof(char));
	enviar(socketNodo,&longNombreEnDondeGuardarlo,sizeof(uint32_t));
	enviar(socketNodo,nombreEnDondeGuardarlo,longNombreEnDondeGuardarlo);


	envioDeReduceAlNodo(listaDeNodosAReducir,socketNodo);

	return socketNodo;
}
void enviarAMartaResultadoOperacionReduce(t_list* listaConNodosAMapear, int nroSocket,char resultadoDeOperacionDelNodo){

	 //	char resultadoDeOperacionDelNodo = recibirResultadoDelNodo(nroSocket);  //lo mande afuera   //capaz deberia estar afuera de la funcion por el mutex

		estructura* nodo = list_get(listaConNodosAMapear,0);
		printf("\n%cxito de reduce\n", resultadoDeOperacionDelNodo);
		send(martaSocket, &resultadoDeOperacionDelNodo, sizeof(char), 0);
		char operacionQueRealizo ='R';
		send(martaSocket, &operacionQueRealizo, sizeof(char), 0);

	if(resultadoDeOperacionDelNodo=='E'){
		int id = (nodo->nodo).idNodo;
		send(martaSocket, &id, sizeof(int), 0);
	//	printf("\n Finalize reduce de nodo: %d\n", id);
		printf(" Finalize reduce del nodo cuya ip es: %s\n", (nodo->nodo).ipNodo);
	}
}

void envioDeReduceAlNodo(t_list* listaDeNodosAReducir, int socketNodo){

	uint32_t cantidadEstructuras = list_size(listaDeNodosAReducir);

	send(socketNodo,&cantidadEstructuras,sizeof(int),0);

	int i=0;
	while(i<cantidadEstructuras){
		estructura* unaEstructura = list_get(listaDeNodosAReducir,i);

		send(socketNodo,&((unaEstructura->nodo).idNodo),sizeof(int),0);
		mandarCadena(socketNodo,(unaEstructura->nodo).ipNodo);
		mandarCadena(socketNodo,(unaEstructura->nodo).puertoNodo);

		int j=0;
		int cantidadBloques = list_size(unaEstructura->bloques);

		send(socketNodo,&cantidadBloques,sizeof(int),0);

		while(j<cantidadBloques){
			bloqueJob* unBloque = list_get(unaEstructura->bloques,j);
			mandarCadena(socketNodo,(unBloque->nombreEnDondeGuardarlo));

			j++;
		}
		i++;
	}

	return ;
}



void envioDeScriptReduce(int socketNodo){

	int fd_archivo = open(rutaReducer,O_RDONLY); //agregar al archivo de config!!
	char* mapeo;
	int tamanioArchivo = tamanio_archivo(fd_archivo);

	if( (mapeo = mmap(NULL, tamanioArchivo, PROT_READ, MAP_SHARED, fd_archivo, 0 )) == MAP_FAILED){
		//Si no se pudo ejecutar el MMAP, imprimir el error y aborta;
		printf("Error con MMAP");
		abort();
	}

	char idProceso = 'J';
	char idOperacion = 'S';
	char tipoScript = 'r';
	int tamanioEnvio = tamanioArchivo + 1;

	enviar(socketNodo,&idProceso,sizeof(char));
	enviar(socketNodo,&idOperacion,sizeof(char));
	enviar(socketNodo,&tipoScript,sizeof(char));
	enviar(socketNodo,&tamanioEnvio,sizeof(uint32_t));
	enviar(socketNodo,mapeo,tamanioEnvio);

	munmap(mapeo,tamanioArchivo);

}



int conectarConNodo(char* puertoNodo,char* ipNodo){
	int nodoSocket;
	struct addrinfo hints;
		struct addrinfo *serverInfo;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		getaddrinfo(ipNodo, puertoNodo, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion

		nodoSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,serverInfo->ai_protocol);

		if(nodoSocket == -1){
				printf("No se pudo conectar al Nodo.\n");
		}

		if (connect(nodoSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)!= -1) {
		//	printf("Conectado con Nodo\n");

			freeaddrinfo(serverInfo);

			char saludo = 'J';
			send(nodoSocket, &saludo, sizeof(char), 0); //Enviar Saludo
		}

	return nodoSocket;
}


// ENVIA Y RECIBE TODO

int enviar(int s, char *buf, int len)
   {
       int total = 0;        // cuántos bytes hemos enviado
       int bytesleft = len; // cuántos se han quedado pendientes
       int n;
       while(total < len) {
           n = send(s, buf+total, bytesleft, 0);
    //       printf("Envio: %d\n",n);
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
  //     printf("Total recibido: %d\n",total);
       return n==-1?-1:0; // devuelve -1 si hay fallo, 0 en otro caso
        }
void loguearFinalizacionDeHilos(){









}


//------------------Formatear Archivo de Log---------------------


void formatearArchivoDeLog(){

	remove("LogJob");


}


// ------------Archivo de Log-------------------------


void loguearCreacionDeHiloReduce(char* unaIP){

	t_log_level nivel;
		t_log* archivoDeLog;
		nivel = LOG_LEVEL_INFO;
		archivoDeLog = log_create("LogJob", "Job", 0, nivel);



		 char *unaPalabra = string_new();
			 string_append(&unaPalabra, "Hilo creado para reducir los bloques del archivo solicitado del nodo cuya ip es: ");
			 string_append(&unaPalabra,unaIP);
			 log_info(archivoDeLog,unaPalabra);

	free(unaPalabra);
	log_destroy(archivoDeLog);

}



void loguearConexionConMarta(int seLogroConectar,char* ipMarta,char* puertoMarta){
	t_log_level nivel;
	t_log* archivoDeLog;


	if (seLogroConectar == 0) {

	nivel = LOG_LEVEL_INFO;
	archivoDeLog = log_create("LogJob", "Job", 0, nivel);


	 char *unaPalabra = string_new();
		 string_append(&unaPalabra, "Conexion con Marta exitosa usando ipMarta: ");
		 string_append(&unaPalabra, ipMarta);

		 string_append(&unaPalabra, " puertoMarta: ");
		 string_append(&unaPalabra, puertoMarta);



		 log_info(archivoDeLog,unaPalabra);
		 free(unaPalabra);

	}else{

	nivel = LOG_LEVEL_ERROR;
	archivoDeLog = log_create("LogJob", "Job", 0, nivel); //si el archivo no esta creado se crea solo
	log_error(archivoDeLog,"Fallo conexion con Marta");
	//free(unaPalabra);

		}

	log_destroy(archivoDeLog);





}

void loguearCreacionDeHilosMapper(char* idNodo){
	t_log_level nivel;
	t_log* archivoDeLog;
	nivel = LOG_LEVEL_INFO;
	archivoDeLog = log_create("LogJob", "Job", 0, nivel);



	 char *unaPalabra = string_new();
		 string_append(&unaPalabra, "Hilo creado para mappear los bloques del archivo solicitado del nodo cuya ip es: ");
		 string_append(&unaPalabra,idNodo);




	log_info(archivoDeLog,unaPalabra);
free(unaPalabra);
log_destroy(archivoDeLog);


}
void loguearFinalizacionDeHiloScript(char* script){

	t_log_level nivel;
	t_log* archivoDeLog;
	nivel = LOG_LEVEL_INFO;
	archivoDeLog = log_create("LogJob", "Job", 0, nivel);


	 char *unaPalabra = string_new();
		 string_append(&unaPalabra, "Finalizacion de hilo de  ");
		 string_append(&unaPalabra, script);

			log_info(archivoDeLog,unaPalabra);
		free(unaPalabra);

		log_destroy(archivoDeLog);

}



void loguearCabecerasDeMensajes(char operacion){


	t_log_level nivel;
		t_log* archivoDeLog;
		nivel = LOG_LEVEL_INFO;
		archivoDeLog = log_create("LogJob", "Job", 0, nivel);
if(operacion=='M'){
	log_info(archivoDeLog,"cabecera enviada: M por lo tanto va a mapear");

}else{
	log_info(archivoDeLog,"cabecera enviada: R por lo tanto va a reducir");
}


log_destroy(archivoDeLog);
}










void manejarReplanificacionDeMap(){
printf("hago una replanificacion\n");
	pthread_mutex_lock(&lockEscucharOrdenes);
	char operacion;
	//recv(martaSocket, &operacion, sizeof(char), 0);
	//t_list* listaConNodosAMapear = escucharOrdenesDeMarta(operacion);
	pthread_mutex_unlock(&lockEscucharOrdenes);
	//mandarALaburarALosNodos(listaConNodosAMapear);
}
