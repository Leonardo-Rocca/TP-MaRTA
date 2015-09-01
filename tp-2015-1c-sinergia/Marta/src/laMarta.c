#include "mensajesMarta.h"


char* puertoMarta = "6666";
int colaEspera = 5;
int socketMarta;
char* puertoFileSystem= "8888";
char* ipFileSystem = "127.0.0.1";

pthread_mutex_t lockListaGlobal;
pthread_mutex_t lockVariableGlobalNodo;

int idNodoABuscar = 0; //variable global para usar una commons booleana. (ver si se puede solucionar)

int main(void) {
	printf("Marta\n\n");
	varChanta = 1;
	cargarArchivoDeConfiguracion();
	pthread_t hiloConexiones;
	pthread_t hiloConexionFileSystem;

	if (pthread_mutex_init(&lockEnvioSolicitudFS, NULL) != 0)
		{
		        printf("\n Fallo mutex Escuchar a Marta\n");
		}
	if (pthread_mutex_init(&lockListaGlobal, NULL) != 0)
			{
			        printf("\n Fallo mutex Escuchar a Marta\n");
			}
	if (pthread_mutex_init(&lockVariableGlobalNodo, NULL) != 0)
				{
				        printf("\n Fallo mutex Escuchar a Marta\n");
				}
	pthread_create(&hiloConexionFileSystem,NULL,manejoConexionConFS,NULL);
	pthread_create(&hiloConexiones,NULL,manejoConexionesConJob,NULL);

	//PARA PROBAR
	//t_list* unaLista = list_create();
	//list_add(unaLista, "/home/utnso/workspace/tp-2015-1c-sinergia/FileSystem/src/archivo");
	//solicitarArchivosAlFileSystem(unaLista);

	pthread_join(hiloConexionFileSystem,NULL);
	pthread_join(hiloConexiones,NULL);

	return 0;
}

//MANEJO DE CONEXIONES

//MANEJA LAS CONEXIONES CON LOS NODOS.  ------------------------


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

void *manejoHiloJob(void *socketCli)
{
    int nroSocket = *(int*)socketCli;

    recibirMensajeDe(nroSocket);

    printf("Job Desconectado.\n");

    free(socketCli);

    return 0;
}

void* manejoConexionConFS(void* parametro){

	socketFS = conectarAFileSystem();
	return 0;
}

//CONECTA AL FILESYSTEM-----------------------------------------------

int fileSystemSocket;

int conectarAFileSystem(){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ipFileSystem, puertoFileSystem, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion


	fileSystemSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	if(fileSystemSocket == -1){
		printf("No se pudo conectar al FS.\n");
	}


	if(connect(fileSystemSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)!= -1){
		printf("Conectado con File System\n");

	freeaddrinfo(serverInfo);

	char saludo ='M';
	send(fileSystemSocket,&saludo,sizeof(char),0); //Enviar Saludo

	}else{
		printf("no se pudo conectar al FS.\n");
	}

	return fileSystemSocket;
}


//Enviar ordenes de mapps y reduces----------------

int mandarAEjecutarJob(t_list* estructuraDeTrabajo,int combiner,int socketQueEscribe,char* rutaDeResultadoDondeLoQuiere,t_list* estructuraDeTrabajoCompleta,t_list* rutasArchivos){

	int agrupaciones = generarNumeroDeAgrupaciones(estructuraDeTrabajo,combiner);
	t_list* tabla = list_create();
	t_list* eslabonDeTabla = list_create();
	list_add(tabla,eslabonDeTabla);
	int resultadoOperacion=1;
	int noEsFinDeOperacion=1;

	tabla = mandarAEjecutarUnMappACadaNodo(estructuraDeTrabajo,tabla,socketQueEscribe);//llena la tabla
	imprimirTabla(tabla);
//	eliminarListaEstructuraDeTrabajo(estructuraDeTrabajo);
	while((resultadoOperacion!=-1)&&noEsFinDeOperacion){
		noEsFinDeOperacion = verificarQueNoTerminoLaOperacion(tabla,agrupaciones, combiner);
		if(noEsFinDeOperacion){

			resultadoOperacion=verificarQueSeEjecutoBien(socketQueEscribe); //recibe un char
	//		printf("no es fin de operacion \n");

			if(resultadoOperacion!=-1){
				escucharYMandarAEjecutarReduce(tabla,combiner,socketQueEscribe,agrupaciones);

				imprimirTabla(tabla);
			}else{
				printf("error al ejecutar el job \n");
				/*return*/
				resultadoOperacion =  manejarElFalloDeEjecucucion(socketQueEscribe,tabla,estructuraDeTrabajoCompleta,combiner,rutasArchivos);
			}
		}else{ //es fin de operacion
	//		printf("es fin de operacion \n");
			struct eslabonDeSubTabla* elQueTieneElResultado = list_get(list_get(tabla,list_size(tabla)-1),0);
			int idQueTieneLasCosas = destransformarIdNodo(elQueTieneElResultado->nodo.idNodo);
			pedirleAlFSQueGuardeElArchivoResultante(idQueTieneLasCosas,elQueTieneElResultado->nombreEnElArchivoTemporal,rutaDeResultadoDondeLoQuiere);
			printf("operacion finalizada \n");
			printf("Podes buscar tu archivo en :%s\n",rutaDeResultadoDondeLoQuiere);
			char idProceso = 'F';
			send(socketQueEscribe,&idProceso,sizeof(char),0);
			eliminarTabla(tabla);
			return 0;
		}
	}
}
//-- se fija que solo quede un elemento en la ultima fila, en la anterior n(la cantidad de agrupaciones) elementos, y si hay otra fila mas, que haya n + (los que no completaron una agrupacion)--
//----devuelve 1 si la operacion de map/reduce no termino, y 0 si termino
int  verificarQueNoTerminoLaOperacion(t_list* tabla,int agrupaciones,int combiner){
	int posicionUltimaFila = list_size(tabla);
//--si solo tiene un elemento, entonces no termino la operacion. solo tiene la fila de maps----
	if(posicionUltimaFila==1)return 1;

	int tamanioUltimaFila = list_size(list_get(tabla,posicionUltimaFila-1));
	int tamanioDeLaAnterior = list_size(list_get(tabla,posicionUltimaFila-2));;

	//--obtenemos el resto, es decir los q quedan para el final en los reduce(no llegaron a armar una agrupacion)
		//	int losQueQuedaronSinAgrupacion; //esto antes lo usaba, AHORA NO
	    //    if(posicionUltimaFila>1)losQueQuedaronSinAgrupacion = (list_size(list_get(tabla,0)))%agrupaciones;

	int tamanioQueDeberiaTener;
	modelon unNodo;
	int i = 0;
	for(i;i<posicionUltimaFila;i++){
		//agarro un nodo de la fila para obtener el tamaño que deberia tener esta
	//	unNodo = ((vagon*)list_get(list_get(tabla,i-1),0))->nodo;
		tamanioQueDeberiaTener = obtenerTamanioQueDeberiaTenerFilaSegunUnIndice(tabla,i,agrupaciones,combiner);

		if(( (!list_all_satisfy(list_get(tabla,i),(void*)sonTodosLosEstadosNodosUno)))
				|| (tamanioUltimaFila!=1)
				|| tamanioQueDeberiaTener!=list_size(list_get(tabla,i))  )return 1;
	}

	return 0;
}

int generarNumeroDeAgrupaciones(t_list* lista,int combiner){
	if(combiner==0)return (list_size(lista));
	int tamanioLista = list_size(lista);
	if(tamanioLista==4)return 2;
	float agrupaciones =tamanioLista/2;     //esto hay q evaluarlo, por ahora es seudo-dammy
	return (int)agrupaciones+1;
	//lo anterior es la q va, para probar usemos este:
	//return list_size(lista);
}

t_list* mandarAEjecutarUnMappACadaNodo(t_list* estructuraDeTrabajo,t_list* tabla,int socketQueEscribe){
	int i = 0;
	int cantidadDeNodos = list_size(estructuraDeTrabajo);
	modeloe* estructura = malloc(sizeof(modeloe));
	char idOperacion = 'M';// map
	send(socketQueEscribe,&idOperacion,sizeof(char),0);
	send(socketQueEscribe,&cantidadDeNodos,sizeof(int),0);


	for(i;i<cantidadDeNodos;i++){
		modeloe* estructuraAux = list_get(estructuraDeTrabajo,i);
		*estructura=*estructuraAux;
		//imprimirTabla(tabla);
		tabla = enviarEstructuraYArmarTabla(estructura,socketQueEscribe,tabla);
	}
	return tabla;
}

t_list* enviarEstructuraYArmarTabla(modeloe* estructuraAuxiliar,int socket,t_list* tabla){
	modeloe estructura = *estructuraAuxiliar;
	modelon nodoAMappear = estructura.nodo;
	int idNodo = nodoAMappear.idNodo;

	char* ipNodo = malloc(15);
	char* puerto = malloc(10);

	ipNodo = nodoAMappear.ipNodo;
	puerto = nodoAMappear.puertoNodo;
	int tamanioIp = strlen(ipNodo) +1;
	int tamanioPuerto = strlen(puerto) +1;

//	printf(" mapeame nodo : %d\n", idNodo);
	send(socket,&idNodo,sizeof(int),0);  //agregar los demas campos del nodo
	send(socket,&tamanioIp,sizeof(int),0);
	send(socket,ipNodo,tamanioIp,0);
	send(socket,&tamanioPuerto,sizeof(int),0);
	send(socket,puerto,tamanioPuerto,0);

	int cantidadDeBloques= list_size(estructura.bloques);
	send(socket,&cantidadDeBloques,sizeof(int),0);

	struct eslabonDeSubTabla eslabon;// = obtenerEslabon(tabla,nodoAMappear);
	eslabon.nodo=nodoAMappear;
	eslabon.estado=0;
	eslabon.nombreDelMapp=list_create();
	eslabon.nombreEnElArchivoTemporal = "eslabon map";

	int i =0;
	modelob* bloqueAux;
	int idBloqueAux;
	char* nombre;
	for(i;i<cantidadDeBloques;i++){
		bloqueAux=list_get(estructura.bloques,i);
		idBloqueAux=bloqueAux->idBloqueNodo;
//	printf(" mapeame bloque : %d\n", idBloqueAux);
		send(socket,&idBloqueAux,sizeof(int),0);
		 nombre= generarNombreRandom();

		mandarCadena(socket,nombre);
	//	printf(" y guardalo en : %s\n", nombre);
		archivoMap* archivo = malloc(sizeof(archivoMap));
		(*archivo).estado=0;
		(*archivo).nombreEnElArchivoTemporal=nombre;
		(*archivo).idBloqueArchivo = bloqueAux->idBloqueArchivo; //esto es para la re-planificacion
		list_add(eslabon.nombreDelMapp,archivo);
	}

	tabla = agregarATablaEslabon(tabla,eslabon);

	return tabla;

}

t_list* agregarATablaEslabon(t_list* tabla,struct eslabonDeSubTabla eslabonAux){
	int posicionMaxima = list_size(tabla);
	int posicionABuscar=0;
	struct eslabonDeSubTabla* eslabon = malloc(sizeof(struct eslabonDeSubTabla));
	t_list* fila;
	modelon nodoAMappear=eslabonAux.nodo;
	int flagLaEncontro=-1;

	while(posicionABuscar<posicionMaxima){

	fila = list_get(tabla,posicionABuscar);
	if(seEncuentraNodoEnFila(nodoAMappear,fila)){
		posicionABuscar++;
	 }else{
	*eslabon=eslabonAux;
	list_add(fila,eslabon); //esto funciona??
	flagLaEncontro = 1;
	return tabla;                                            //agregado la madrugada del jueves
	}
	 }
	// vemos q pasa si esta en todas las filas
		if(flagLaEncontro==-1){
			t_list* filaAAgregar = list_create();
			*eslabon=eslabonAux;
			list_add(filaAAgregar,eslabon);
			list_add(tabla,filaAAgregar);          //el unico lugar donde se agregan filas
	     }
	return tabla;
}
void escucharYMandarAEjecutarReduce(t_list* tabla,int combiner,int socketQueEscribe,int agrupaciones){

	int idNodo;
	char* archivoDondeSeGuardo;
	char operacionQueRealizo = recibirChar(socketQueEscribe);
	struct eslabonDeSubTabla* eslabon;


	if(operacionQueRealizo=='M'){ //realizo un map

	recv(socketQueEscribe, &idNodo, sizeof(int), 0);

	archivoDondeSeGuardo = recibirCadena(socketQueEscribe);

	eslabon=obtenerEslabon(idNodo,tabla);
	t_list* listaDeArchivos =(*eslabon).nombreDelMapp;
	informarListaArchivosResultadoExitoso(listaDeArchivos,archivoDondeSeGuardo);
		//--.Verifico que todos los bloques del nodo se hayan mapeado, de ser asi, envio a reducir----
		if(list_all_satisfy(listaDeArchivos,&sonTodosLosEstadosArchivosUno)){

			(*eslabon).estado=1;
			int numeroDEAgrupacionesCapazUnitario = agrupaciones*(!combiner)+combiner;
		//--si es combiner hace una agrupacion unitaria para que se envie un reduce solo a ese nodo,sino se tiene q hacer un reduce en la totalidad
			enviarAEjecutarReduce(eslabon,tabla,socketQueEscribe,numeroDEAgrupacionesCapazUnitario, combiner);
	}

	}else{    //escucho 'R'
		recv(socketQueEscribe, &idNodo, sizeof(int), 0);  //falta hacer que lo recibe todo
		eslabon=obtenerEslabon(idNodo,tabla);
		(*eslabon).estado=1;
		//pthread_t hiloAnalizarSiReducir;


//		printf("idNodo:%d \n", idNodo);

		//pthread_create(&hiloAnalizarSiReducir,NULL,enviarAEjecutarReduce,NULL);
		enviarAEjecutarReduce(eslabon,tabla,socketQueEscribe,agrupaciones, combiner);
	}

}
int verificarQueSeEjecutoBien(socketQueEscribe){
	char bienOmal = recibirChar(socketQueEscribe);
	if(bienOmal=='E'){//exito
//		printf("una tanda exitosa\n");
		return 1;
	}else if (bienOmal=='X') {
	//	printf("no exitosa\n");
		return -1;
	}
	//printf("nunca deberia llegar aca");
	return -1;
}

bool estaElNodo(void *elemento) {
   struct eslabonDeSubTabla* elem = (struct eslabonDeSubTabla*)elemento;
   return ((elem->nodo).idNodo) == idNodoABuscar;
}
/*
int seEncuentraNodoEnFila(modelon nodoAMappear,t_list* fila){
	idNodoABuscar = nodoAMappear.idNodo; //ACLARACION: idNodoAbuscar es una variable global ya que a la funcion bool no le puedo pasar parametros.
	//idNodoABuscar= 2;
//	if(list_any_satisfy(fila,&estaElNodo)){
		if(list_count_satisfying(fila,&estaElNodo)>0){
		return 1; // esta
	}else{
		return 0; // no esta
	};
}*/
//----a manopla-----
int seEncuentraNodoEnFila(modelon nodoAMappear,t_list* fila){
	int tamanioFila = list_size(fila);
	int i = 0 ;
	vagon* estructura;
	for(i;i<tamanioFila;i++){
		estructura = list_get(fila,i);
		if((estructura->nodo).idNodo == nodoAMappear.idNodo) return 1;//esta
	}
	return 0;//no esta
}
struct eslabonDeSubTabla* obtenerEslabon(int idNodo,t_list* tabla){
	int posicionMaxima = list_size(tabla);
	int posicionABuscar=0;
	struct eslabonDeSubTabla* eslabon ;//= malloc(sizeof(struct eslabonDeSubTabla));//ver si dejar este malloc..
	t_list* fila;
	int flagNoEncontrado=0;
	modelon nodo;
	nodo.idNodo = idNodo;
	while(posicionABuscar<posicionMaxima&&flagNoEncontrado==0){
	fila=list_get(tabla,posicionABuscar);
	if(seEncuentraNodoEnFila(nodo,fila)){
		posicionABuscar++;
	}else{
		fila=list_get(tabla,posicionABuscar-1);
		flagNoEncontrado=1;

		pthread_mutex_lock(&lockVariableGlobalNodo);
		variableGlobalNodo = nodo;
		 eslabon =list_find(fila,&esEslabonQueBusco);

		pthread_mutex_unlock(&lockVariableGlobalNodo);
		}
	}
	//--vemos q pasa si lo encontro en todas--
	if(flagNoEncontrado==0){
		fila=list_get(tabla,posicionABuscar-1);
		pthread_mutex_lock(&lockVariableGlobalNodo);
				variableGlobalNodo = nodo;
				 eslabon =list_find(fila,&esEslabonQueBusco);
		pthread_mutex_unlock(&lockVariableGlobalNodo);

	}

 	return eslabon;
}

int obtenerIndiceDeFila(int idNodo,t_list* tabla){
	int posicionMaxima = list_size(tabla);
	int posicionABuscar=0;
	struct eslabonDeSubTabla* eslabon;
	t_list* fila;
	int flagLaEncontro=-1;
	while(posicionABuscar<posicionMaxima){
	fila=list_get(tabla,posicionABuscar);
	modelon nodo;
	nodo.idNodo = idNodo;
	if(seEncuentraNodoEnFila(nodo,fila)){
		posicionABuscar++;
	}else{
		flagLaEncontro = 1;
		 return posicionABuscar-1;
	}
	}
// vemos q pasa si esta en todas las filas
		if(flagLaEncontro==-1){
			return posicionMaxima-1;
		}

}

void informarListaArchivosResultadoExitoso(t_list* listaDeArchivos,char* archivoDondeSeGuardo){
	int cantArchivos = list_size(listaDeArchivos);
	int i=0;
	int encontrado = 0;
	while(i<cantArchivos && encontrado!=1){
		archivoMap* archivo = list_get(listaDeArchivos,i);

		if(!strcmp(archivo->nombreEnElArchivoTemporal,archivoDondeSeGuardo)){
			archivo->estado = 1;
			encontrado = 1;
		}

		i++;
	}
}

char* generarNombreRandom(){
	char* nombreAleatorio;
		nombreAleatorio = string_new();
		 int numero;

		  srand(rdtsc()); //semilla para crear el nro random
		 numero = rand();
		 nombreAleatorio = string_itoa(numero);

		return nombreAleatorio;
}

void enviarAEjecutarReduce(struct eslabonDeSubTabla* eslabon ,t_list* tabla,int socketQueEscribe,int agrupaciones, int combiner){
	struct eslabonDeSubTabla* eslabonAux =eslabon ;
	modelon unNodo = eslabonAux->nodo;
	t_list* rama = obtenerRama(tabla,unNodo,agrupaciones);//to do!!

	int tamanioDeFilaQueDeberiaTener = obtenerTamanioQueDeberiaTenerFilaDeNodo(tabla,unNodo,agrupaciones,combiner);
	int indice = obtenerIndiceDeFila(unNodo.idNodo,tabla);
	int tamanioQueTiene =list_size(list_get(tabla,indice));

	int noEsFinDeOperacion = verificarQueNoTerminoLaOperacion(tabla,agrupaciones, combiner);
	int losDeLaFilaQueSonUno = list_count_satisfying(list_get(tabla,indice),sonTodosLosEstadosNodosUno);
	int condicionEspecial = verSiCorresponde2(tamanioDeFilaQueDeberiaTener,list_size(rama),agrupaciones,tamanioQueTiene);// verSiCorrespondeReducirSegunNroNodosDeLaRama(tamanioDeFilaQueDeberiaTener,list_size(rama),losDeLaFilaQueSonUno,agrupaciones);


	if( list_all_satisfy(rama,&sonTodosLosEstadosNodosUno)  &&  noEsFinDeOperacion && condicionEspecial){  //tamanioDeFilaQueDeberiaTener==tamanioQueTiene
		printf("OJO , ESTA ENTRANDO\n");
		int i = 0;
			int cantidadDeNodos = list_size(rama);
			modelon nodoAReducir;

	//Si solo hay un nodo que reducir, no lo mando a el job------se llama a la funcion ejecutar reduce para que evalue si corresponde reducir dadas las nuevas circunstancias
			if((cantidadDeNodos==1) && (indice != 0)){
				eslabonAux = list_get(rama,0);
				struct eslabonDeSubTabla* eslabonUnitario = malloc(sizeof(vagon));
				eslabonUnitario->nodo=eslabonAux->nodo;
				eslabonUnitario->estado=1;
				eslabonUnitario->nombreEnElArchivoTemporal = eslabonAux->nombreEnElArchivoTemporal;
				eslabonUnitario->nombreDelMapp = list_create();
				agregarATablaEslabon(tabla,*eslabonUnitario);
				enviarAEjecutarReduce(eslabonUnitario,tabla,socketQueEscribe,agrupaciones, combiner);
				return;
			}

			char idOperacion = 'R';// reduce
			send(socketQueEscribe,&idOperacion,sizeof(char),0);
			send(socketQueEscribe,&cantidadDeNodos,sizeof(int),0);

			char* nombre;
			for(i;i<cantidadDeNodos;i++){
				eslabonAux = list_get(rama,i);

			//--enviar nodos -------------
				nodoAReducir = eslabonAux->nodo;

				send(socketQueEscribe,&nodoAReducir.idNodo,sizeof(int),0);  //--falta mandar ip'y puerto

				int tamanioIp = strlen(nodoAReducir.ipNodo)+1;
				send(socketQueEscribe, &(tamanioIp), sizeof(int), 0);
				send(socketQueEscribe, nodoAReducir.ipNodo, tamanioIp, 0);

				int tamanioPuerto = strlen(nodoAReducir.puertoNodo)+1;
				send(socketQueEscribe, &(tamanioPuerto), sizeof(int), 0);
				send(socketQueEscribe, nodoAReducir.puertoNodo, tamanioPuerto, 0);

				int j =0;
				archivoMap* bloqueAux;
				int idBloqueAux;

				t_list* listaArchivos=(*eslabonAux).nombreDelMapp;
				int cantidadDeBloques = list_size(listaArchivos);

				send(socketQueEscribe,&cantidadDeBloques,sizeof(int),0);

				for(j;j<cantidadDeBloques;j++){
					bloqueAux=list_get(listaArchivos,j);
					nombre=bloqueAux->nombreEnElArchivoTemporal;

					mandarCadena(socketQueEscribe,nombre);
				}

			if(cantidadDeBloques==0){

				mandarCadena(socketQueEscribe,eslabonAux->nombreEnElArchivoTemporal);

			}


			}
			nombre=generarNombreRandom();

			mandarCadena(socketQueEscribe,nombre);

			struct eslabonDeSubTabla* eslabonCabecillaAReducir;// = malloc(sizeof(struct eslabonDeSubTabla));

			eslabonCabecillaAReducir = list_get(rama,0); //el cabecilla
			//struct eslabonDeSubTabla eslabonAAgregar = *eslabonCabecillaAReducir;

			//vagon* eslabonCabecillaAReducir;
			//*eslabonCabecillaAReducir = *eslabonCabecillaAReducirptr;

			eslabonCabecillaAReducir->estado=0;
			eslabonCabecillaAReducir->nombreEnElArchivoTemporal = nombre;
			eslabonCabecillaAReducir->nombreDelMapp = list_create();
			agregarATablaEslabon(tabla,*eslabonCabecillaAReducir);
		}
}
int verSiCorresponde2(int tamanioDeFilaQueDeberiaTener,int tamanioRama ,int agrupaciones,int tamanioQueTiene){
	if(tamanioRama==agrupaciones)return 1;
	if(tamanioDeFilaQueDeberiaTener==tamanioQueTiene)return 1;
	return 0;
}
int verSiCorrespondeReducirSegunNroNodosDeLaRama(int tamanioDeFilaQueDeberiaTener,int tamanioRama,int losDeLaFilaQueSonUno,int agrupaciones){
	if(tamanioRama==agrupaciones)return 1; //fail fast
	int losQueQuedanSinAgrupacion = tamanioDeFilaQueDeberiaTener%agrupaciones;
	int losQueFueronAgrupados = ((int)(losDeLaFilaQueSonUno/agrupaciones))*agrupaciones;

	int losQueFaltanReducir = tamanioDeFilaQueDeberiaTener-losQueFueronAgrupados;
	if(losQueFaltanReducir<agrupaciones){
		if(tamanioRama==losQueQuedanSinAgrupacion)return 1;
	}
	return 0;
}

/*
t_list* obtenerRama(t_list* tabla,modelon unNodo,int agrupaciones){
	int indice = obtenerIndiceDeFila(unNodo.idNodo,tabla);
	t_list* fila = list_get(tabla,indice);
	//--creo un clon de la fila x las dudas para no tocar la original---- verificar..
	int tamanioFila = list_size(fila);
	t_list* filaClon = list_create();//list_take(fila,tamanioFila);
	list_add_all(filaClon,fila);
	modelon nodoAux;
	struct eslabonDeSubTabla* eslabon;
	int i=0;
while(1){
		eslabon = list_get(filaClon,0);
		nodoAux = eslabon->nodo;
		if((nodoAux.idNodo)!=(unNodo.idNodo)){
			list_remove(filaClon,0);
		}else{
			if(list_size(filaClon)>agrupaciones)
		 	return list_take(filaClon,agrupaciones);
			return list_take(filaClon,list_size(filaClon));
		}
	}
}*/

int obtenerTamanioQueDeberiaTenerFilaDeNodo(t_list* tabla,modelon unNodo,int agrupaciones,int combiner){
	int indice = obtenerIndiceDeFila(unNodo.idNodo,tabla);
	int i=1 ;
	int tamanios= list_size(list_get(tabla,0));

	if(combiner==0){
		if(indice==0)return tamanios;
		return 1;
	}
	int resto;
	for(i;i<indice;i++){
		resto=tamanios%agrupaciones;
		if(resto!=0){
		tamanios=tamanios/agrupaciones+1;}
		else{tamanios=tamanios/agrupaciones;}
	}
	return tamanios;
}
int obtenerTamanioQueDeberiaTenerFilaSegunUnIndice(t_list* tabla,int indice,int agrupaciones,int combiner){
	int i=1 ;
	int tamanios= list_size(list_get(tabla,0));
	if(combiner==0){
		if(indice==0)return tamanios;
		return 1;
	}
	int resto;
	for(i;i<indice;i++){
		resto=tamanios%agrupaciones;
		if(resto!=0){
		tamanios=tamanios/agrupaciones+1;}
		else{tamanios=tamanios/agrupaciones;}
	}
	return tamanios;
}

	t_list* obtenerRama(t_list* tabla,modelon unNodo,int agrupaciones){
		int indice = obtenerIndiceDeFila(unNodo.idNodo,tabla);
		t_list* fila = list_get(tabla,indice);
		//--creo un clon de la fila x las dudas para no tocar la original---- verificar..
		int tamanioFila = list_size(fila);
		t_list* filaClon = list_create();//list_take(fila,tamanioFila);
		list_add_all(filaClon,fila);
		modelon nodoAux;
		struct eslabonDeSubTabla* eslabon;
		int i=0;int j =0; int contadorDeElementos=0;int contadorDeElementosAuxiliar=0;
		int contadorDeAgrupacionesQueBorre =0;
	t_list* laux= list_create();
		while(j<tamanioFila){
			//--hago esto para agarrar menos elementos si no llegan a completar una agrupacion--
			if((tamanioFila-contadorDeAgrupacionesQueBorre*agrupaciones)<agrupaciones)
				agrupaciones=tamanioFila-contadorDeAgrupacionesQueBorre*agrupaciones;

			contadorDeElementosAuxiliar =0;
	//--el limite inferior de i es el que le dejo el ciclo anterior(0 si es la 1ra vez que entra al ciclo  o N * la cantidad de agrupaciones si no es la 1ra vuelta)
			//--el limite superior es el multiplo de las agrupaciones que le sigue
		//	for(i;i<(agrupaciones*(contadorDeAgrupacionesQueBorre+1));i++){
			for(i;i<(agrupaciones+contadorDeElementos);i++){
				eslabon = list_get(filaClon,i);
				vagon* eslab2=malloc(sizeof(vagon));
				eslab2->nombreDelMapp = eslabon->nombreDelMapp;
				*eslab2=*eslabon;
				nodoAux = eslabon->nodo;
				list_add(laux,eslab2);
				contadorDeElementosAuxiliar++;
			}
			contadorDeElementos=contadorDeElementos+contadorDeElementosAuxiliar;
			if(seEncuentraNodoEnFila(unNodo,laux)){
				return laux;
			}else{
				contadorDeAgrupacionesQueBorre++;
				list_clean(laux);
			}
		j=j+agrupaciones;
		}
		return laux;
}





_Bool esEslabonQueBusco(struct eslabonDeSubTabla* eslabon){
	modelon nodo=eslabon->nodo;
	int idNodo =nodo.idNodo;
	int idNodoQueQuieroComparar = variableGlobalNodo.idNodo;
	return (idNodo==idNodoQueQuieroComparar);
}


_Bool sonTodosLosEstadosArchivosUno (archivoMap* h){
	int estado = (*h).estado;
	return (estado==1);
}
_Bool sonTodosLosEstadosNodosUno (struct eslabonDeSubTabla* h){
	int estado = (*h).estado;
	return (estado==1);
}

char* recibirCadena(int socketQueEscribe){
	char* cadena;
	uint32_t long_cadena;
	recv(socketQueEscribe, &long_cadena, sizeof(long_cadena), 0);
	cadena = malloc(long_cadena);
	recv(socketQueEscribe, cadena,long_cadena, 0);
return cadena;
}


void mandarCadena(int socket,char* cadena){
	uint32_t long_cadena = strlen(cadena)+1;
	send(socket,&long_cadena,sizeof(long_cadena),0);
	send(socket,cadena,long_cadena,0);
}


////////////////////////////RE-PLANIFICAR///////////////////////////////////

int manejarElFalloDeEjecucucion(int socketQueEscribe,t_list* tabla, t_list* estructuraDeTrabajo,int combiner,t_list* rutas){
	char operacionQueRealizo = recibirChar(socketQueEscribe);
	if(operacionQueRealizo=='M'){
		printf("Operacion fallida,hay que re-planificar\n");
		int idNodoCaido; //= recibirChar(socketQueEscribe);
		recv(socketQueEscribe, &idNodoCaido, sizeof(int), 0);
		replanificarMap(socketQueEscribe,idNodoCaido,tabla ,estructuraDeTrabajo, combiner,rutas);
		return 1;//debe replanificar
	}
	if(operacionQueRealizo=='R')printf("Se cayo un nodo mientras se hacia un reduce \n ");
	return -1; //esta todo perdido
}


void replanificarMap(int socketQueEscribe,int idNodoCaido ,t_list* tabla, t_list* estructuraDeTrabajoConTodosLosNodos,int combiner,t_list* rutas){
//	t_list* estructuraDeTrabajo = borrarDeMiLista(estructuraDeTrabajoConTodosLosNodos,idNodoCaido);
//	verificarQueEstanTodasLaspartesDelArchivo(estructuraDeTrabajo);				//TO DO!

	pthread_mutex_lock(&lockEnvioSolicitudFS);
	t_list* estructuraDeTrabajo =solicitarArchivosAlFileSystem(rutas);
	pthread_mutex_unlock(&lockEnvioSolicitudFS);

	if(list_size(estructuraDeTrabajo)==0){
		printf("Finaliza el Job debido a que FS no posee alguno de los archivos solicitados\n");
		char idProceso = 'F';
		send(socketQueEscribe,&idProceso,sizeof(char),0);
		return;
	}
	vagon* eslabonDelCaido = obtenerEslabon(idNodoCaido,tabla);

	//-----(involucra una variable global en la funcion)
	pthread_mutex_lock(&lockListaGlobal);
	estructuraDeTrabajo = filtrarLasPartesDelArchivoQueHayQueRemapear(estructuraDeTrabajo,eslabonDelCaido);
	pthread_mutex_unlock(&lockListaGlobal);


	estructuraDeTrabajo = planificarNodos(estructuraDeTrabajo,combiner);
	sacarDeTabla(tabla,eslabonDelCaido);
//	cambiarIdNodos(estructuraDeTrabajo);
	cambiarIdNodosParaQueNoSeRepitanEnTabla(estructuraDeTrabajo,tabla);
	imprimirTabla(tabla);
	mandarAEjecutarUnMappACadaNodo(estructuraDeTrabajo,tabla,socketQueEscribe);
}

t_list* borrarDeMiLista(t_list* estructuraDeTrabajoConTodosLosNodos,int idNodoCaido){
	pthread_mutex_lock(&lockVariableGlobalNodo);

	variableGlobalNodo.idNodo=idNodoCaido;

	//list_remove_and_destroy_by_condition(estructuraDeTrabajoConTodosLosNodos,esElNodoQueBusco,(void*)destruirEstructuraDeTrabajo);
	list_remove_by_condition(estructuraDeTrabajoConTodosLosNodos,esElNodoQueBusco);
	pthread_mutex_unlock(&lockVariableGlobalNodo);

	return estructuraDeTrabajoConTodosLosNodos;
}

_Bool esElNodoQueBusco(modeloe* estructura){
	modelon nodo=estructura->nodo;
	int idNodo =nodo.idNodo;
	int idNodoQueQuieroComparar = variableGlobalNodo.idNodo;
	return (idNodo==idNodoQueQuieroComparar);
}

void armarListaDeArchivosEnVariableGlobalParaPoderFiltrar(vagon* eslabonDelCaido){
	t_list* listaDeBloques=list_create();
	int i = 0;
	archivoMap* archivo;

	for(i;i<list_size(eslabonDelCaido->nombreDelMapp);i++){
		archivo = list_get(eslabonDelCaido->nombreDelMapp,i);

		modelob* bloqueParaComparar = malloc(sizeof(modelob));
		(*bloqueParaComparar).idBloqueArchivo = archivo->idBloqueArchivo;
		list_add(listaDeBloques,bloqueParaComparar);
	}

	variableGlobalBloques= listaDeBloques;
	return;
}

t_list*  filtrarLasPartesDelArchivoQueHayQueRemapear(t_list* estructuraDeTrabajo,vagon* eslabonDelCaido){
	armarListaDeArchivosEnVariableGlobalParaPoderFiltrar(eslabonDelCaido);

	int i = 0;
	modeloe* estructuraAuxiliar;
	for(i;i<list_size(estructuraDeTrabajo);i++){
		estructuraAuxiliar = list_get(estructuraDeTrabajo,i);
		dejarSoloLosArchivosQueSirven(estructuraAuxiliar);
	}
	t_list* estructuraDeTrabajoConArchivosUtiles = estructuraDeTrabajo;
	//= list_map(estructuraDeTrabajo,(void*)dejarSoloLosArchivosQueSirven);
	//list_remove_by_condition(estructuraDeTrabajoConArchivosUtiles,(void*)listaDeBloquesVacia);       //Tengo dudas de que pasa si lo que remueve esta en el primer elemento, tendria que asignarle el remove a una t_list*??
	estructuraDeTrabajoConArchivosUtiles = list_filter(estructuraDeTrabajoConArchivosUtiles,(void*)listaDeBloquesVacia);

	//vaciamos la variable global
/*	modeloe* estructuraInservible = malloc(sizeof(modeloe));
	estructuraInservible->bloques = list_create();
	list_add_all(estructuraInservible->bloques,variableGlobalBloques);
	destruirEstructuraDeTrabajo(estructuraInservible);
*/
	return estructuraDeTrabajoConArchivosUtiles;
}

//remueve de la lista los bloques que no esten caidos
modeloe* dejarSoloLosArchivosQueSirven(modeloe* estructura){
	estructura->bloques = list_filter(estructura->bloques,(void*)bloqueDistintoAlQueBusco);
return estructura;
}

//devuelve 1 si el bloque pertenece a la lista que busco
_Bool bloqueDistintoAlQueBusco(modelob* bloque){ //seria bloqueIgualAlQueBusco
	int i =0;
	modelob* bloqueAux;
	for(i;i<list_size(variableGlobalBloques);i++){
		bloqueAux=list_get(variableGlobalBloques,i);
		if(bloqueAux->idBloqueArchivo==bloque->idBloqueArchivo)
			return 1;
	}
	return 0;
}

_Bool listaDeBloquesVacia(modeloe* estructura){
	return !list_is_empty(estructura->bloques);
}

void sacarDeTabla(t_list* tabla,vagon* eslabonDelCaido){
	//MUTEX (ya existente)
	pthread_mutex_lock(&lockVariableGlobalNodo);

	variableGlobalNodo = eslabonDelCaido->nodo;
	list_remove_and_destroy_by_condition(list_get(tabla,0),(void*)esEslabonQueBusco,(void*)destruirVagon);

	pthread_mutex_unlock(&lockVariableGlobalNodo);

}

//le aumenta 100 a los id de los nodos para distinguirlos de los ya existentes en tabla.
void cambiarIdNodos(t_list* estructuraDeTrabajo){
	modeloe* estructura;
	int i = 0;
	for(i;i<list_size(estructuraDeTrabajo);i++){
		estructura = list_get(estructuraDeTrabajo,i);
		estructura->nodo.idNodo = estructura->nodo.idNodo+100;
	}
}
int destransformarIdNodo(int id){
	//if(id>199)return id-200;//me atajo ante una posible re-re-planificacion
	//if(id>99)return id-100;
	while(id>100){
		id=id-100;
	}return id;
}
void cambiarIdNodosParaQueNoSeRepitanEnTabla(t_list* estructuraDeTrabajo,t_list* tabla){
	t_list* fila = list_get(tabla,0);
	t_list* idNodos = list_map(fila,(void*)obtenerIdNodos);
	modeloe* estructura;
	int i = 0;
	int flag;

	for(i;i<list_size(estructuraDeTrabajo);i++){
			estructura = list_get(estructuraDeTrabajo,i);
		do{
			estructura->nodo.idNodo = estructura->nodo.idNodo+100;
		flag = elem(idNodos,(estructura->nodo).idNodo);

		}while(flag);
	}
}

void* obtenerIdNodos(void* eslabon){
	return ((vagon*)eslabon)->nodo.idNodo;
}

int elem(t_list* lista, int elemento){
	int i =0;
	int elementoDeLaLista;
	while(i<list_size(lista)){
		elementoDeLaLista = list_get(lista,i);
		if(elemento==(elementoDeLaLista) )return 1;
		i++;
	}
	return 0;
}









//funciones dammys no utilizadas --------------------- (ver si borrarlas en un futuro)


/*
t_list* agregarNodoATabla(t_list* tabla,struct nodo nodoAMappear){
	int posicionMaxima = list_size(tabla);
	int posicionABuscar=0;
	struct eslabonDeSubTabla* eslabon = malloc(sizeof(struct eslabonDeSubTabla));
	struct eslabonDeSubTabla eslabonAux;
	t_list* fila;

	while(posicionABuscar<posicionMaxima){

	fila = list_get(tabla,posicionABuscar);
	if(seEncuentraNodoEnFila(nodoAMappear,fila)){
		posicionABuscar++;
	 }else{
	eslabonAux.estado=0;
	eslabonAux.nodo=nodoAMappear;
	//char* nombre= generarNombreRandom();            TO DO
	*eslabon=eslabonAux;
	list_add(fila,eslabon);
	}

	//falta ver q pasa si esta en todas las filas
	}
	return tabla;
}
*/
void ejecutarJob(){
	int combiner= 1;
	char* listaArchivos = "c:/desktop";
	char* resultado = "c:/desktop/clima.txt";	//recibir cosas del job y asignarlas a: int combiner;(char* rutaArchrivos)* listaArchivos;*char resultado;

	t_list* listaNodoConArchivos = list_create();
	//listaNodoConArchivos = solicitarArchivosAlFileSystem(listaArchivos ); //devuelve estructura de trabajo

	t_list* nodosConBloquesAEnviar = list_create();
	nodosConBloquesAEnviar = planificarNodos(listaNodoConArchivos, combiner);


	/*//aplicarmap, reduce armando arbol (capaz)*/


	struct nodoDeDondeSacarloYArchivoTemporal nodoDeDondeSacarloYArchivoTemporalA = AplicarJob(nodosConBloquesAEnviar);
	solicitarFyleSystemCopiarResultado(resultado, nodoDeDondeSacarloYArchivoTemporalA);

	printf("JOB");
	avisarJobTareaFInalizada();
}



t_list* planificarNodosDammy(t_list* listaNodoConArchivos,int combiner){
	//funcion DAMMY
	return listaNodoConArchivos;
}

void solicitarFyleSystemCopiarResultado(char* resultado, struct nodoDeDondeSacarloYArchivoTemporal nodoDeDondeSacarloYArchivoTemporalA ){
	//funcion Dammy
//	struct nodo unNodo = nodoDeDondeSacarloYArchivoTemporalA.nodoDeDondeSacarlo;
	char* unArchivo = nodoDeDondeSacarloYArchivoTemporalA.archivoTemporal;

	}

struct nodoDeDondeSacarloYArchivoTemporal AplicarJob(t_list* nodoConBloquesAEnviar){
	//Funcion Dammy
	struct estructuraDeTrabajo* x = list_get(nodoConBloquesAEnviar,0);
	struct nodoDeDondeSacarloYArchivoTemporal nodo;
//	nodo.nodoDeDondeSacarlo = x->nodo;

	return nodo;
}

void avisarJobTareaFInalizada(){
	//dammy
	return;
}


/*
void aplicarMapp(){
	while(1){
		//recivir NodoConResultado
		if(nodoConResultado.resultado == -1)
			avisarError(NodoConResultado);
		//else el resultado es 1
		if(todasLasRamasSonUno(NodoConResultado)){
			if(validarFin())
				break;
		}
		crearHilo(aplicarRedue(Rama(NodoConResultado)));
	}
}
void aplicarReduce(rama(NodoConResultado)){
	NodoConResultado* rama;
	int tamaño = tamaño(rama);
	//"serializar"
	send(serializar(rama,tamaño));
	if(listaSig(NodoConResultado)==NULL){
		ultimoPuntero=agregarElementolista(puntero*)}
	agregarElemento(cabecilla(rama),ultimoPuntero);
	}
}*/
/*verificar si termino:
	if((tamanioUltimaFila==1)
		&&(tamanioDeLaAnterior==agrupaciones)
		&&(list_all_satisfy((list_get(tabla,posicionUltimaFila-1),&sonTodosLosEstadosNodosUno)))
		&&(list_all_satisfy((list_get(tabla,posicionUltimaFila-2),&sonTodosLosEstadosNodosUno)))){
 //--Si solo hay 2 filas, verifico que la de maps y el de reduce, esten todos en uno, es decir que se hayan mapeado todos.
		if(posicionUltimaFila==2){
			return 0;}
	//-- si hay mas de 2 filas, pregunto que la anteultima ,
		else if ((list_size(list_get(tabla,posicionUltimaFila-3))==(agrupaciones+losQueQuedaronSinAgrupacion))
				&&(list_all_satisfy((list_get(tabla,posicionUltimaFila-3),&sonTodosLosEstadosNodosUno)))) {
			return 0;
		}

	}*/
