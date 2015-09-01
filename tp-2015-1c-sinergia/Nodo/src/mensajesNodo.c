/*
 * mensajesNodo.c
 *
 *  Created on: 8/6/2015
 *      Author: utnso
 */

#include "mensajesNodo.h";

//DADO UN FD DE UN ARCHIVO, SE OBTIENE SU TAMANIO--------------------------------------------------------------------
int tamanio_archivo(int fd){
	struct stat buf; //se crea una estructura buf de tipo stat(estructura que tiene un fd).
	fstat(fd, &buf);  //un poco de magia, "arma" la estructura que llamamos buf con el fd pasado por parametro.
	return buf.st_size; //retorna el tamanio del archivo.
}

char* mapearAMemoriaElEspacioDeDatos(char* path){

	char* mapeo;
	int fd_archivo = open(path,O_RDONLY);
	int tamanioArchivo = tamanio_archivo(fd_archivo);

	if( (mapeo = mmap(NULL, tamanioArchivo, PROT_READ, MAP_SHARED, fd_archivo, 0 )) == MAP_FAILED){
		//Si no se pudo ejecutar el MMAP, imprimir el error y aborta;
		printf("Error con MMAP");
		abort();
	}

	return mapeo;
}

long int obtenerPosicionInicialDe(int nroBloque){
	return nroBloque * 20971520;
}

void guardarContenidoEnEspacioTemporalDelNodo(FILE* espacioTemporal,char *contenidoBloque, int numeroDeBloque, int tamanioArchivo){
	//if(tamanioArchivo == 20971520*numeroDeBloque){
		fwrite(contenidoBloque,sizeof(char),20971520,espacioTemporal);
	/*}else{
		int cantidadALeer = tamanioArchivo - 20971520*numeroDeBloque - 20971520;
		char* contenido = malloc(cantidadALeer);
		fseek(espacioTemporal,20971520*numeroDeBloque+20971520,SEEK_SET);
		fread(contenido,sizeof(char),cantidadALeer,espacioTemporal);
		fseek(espacioTemporal,20971520*numeroDeBloque,SEEK_SET);
		fwrite(contenidoBloque,sizeof(char),20971520,espacioTemporal);
		fwrite(contenido,sizeof(char),cantidadALeer,espacioTemporal);
		free(contenido);
	}*/
	//loguearEscrituraDeEspacioTemporal();
}

char* conseguirUnBloquePedido(int bloque){
	char* espacioTemporal = "espacioTemporal.txt";
	int indiceBueno = bloque;
	FILE* archivo = fopen(espacioTemporal, "r");
	int offset = 20971520*indiceBueno;
	fseek(archivo,offset,SEEK_SET);
	char* bloqueLeido = malloc(20971530);
	//memset(bloqueLeido,'\0',20971530);
	fread(bloqueLeido,sizeof(char),20971520,archivo);
	//loguearLecturaDeBloques(indiceBueno);
	int finalDeBloque = 20971518;
	while(bloqueLeido[finalDeBloque] == '0'){
		finalDeBloque--;
	}
	free(bloqueLeido);
	char* bloqueLeido2 = malloc(20971530);
	fseek(archivo,offset,SEEK_SET);
	fread(bloqueLeido2,sizeof(char),finalDeBloque,archivo);

	fclose(archivo);
	return bloqueLeido2;
}

//----------------Archivo de Configuracion---------------------------------
void cargarArchivoDeConfiguracion() {

	t_dictionary* diccionario;
	diccionario = dictionary_create();
	char* dato;
	char* clave;

	char textoLeido[200];
	FILE* archivo;
	//archivo = fopen("/home/utnso/workspace/tp-2015-1c-sinergia/Nodo/Debug/configNodo","r");
	char* config = "configNodo";
	archivo = fopen(config,"r");


	while ((feof(archivo)) == 0) {
		fgets(textoLeido, 200, archivo);

		clave = string_split(textoLeido, ",")[0];
		dato = string_split(textoLeido, ",")[1];
		dictionary_put(diccionario, clave, dato);

	}

	// IMPORTANTE hay que hacer esto que viene abajo pq el dato obtenido del diccionario viene con un caracter
	//de mas entonces tenemos que "limpiarlo" (se lo sacamos)




	ipNodo = malloc(15);
	puertoNodo = malloc(10);
	dirTemp = malloc(100);

	puertoFileSystem = obtenerDatoLimpioDelDiccionario(diccionario, "puertoFS");
	ipFileSystem = obtenerDatoLimpioDelDiccionario(diccionario, "ipFS");
	ipNodo = obtenerDatoLimpioDelDiccionario(diccionario, "ipNodo");
	puertoNodo = obtenerDatoLimpioDelDiccionario(diccionario, "puertoNodo");
	tamanioNodo = obtenerDatoLimpioDelDiccionario(diccionario, "tamanioNodo");
	nodoNuevo = obtenerDatoLimpioDelDiccionario(diccionario, "nodoNuevo");
	dirTemp = obtenerDatoLimpioDelDiccionario(diccionario, "dirTemp");
}

char* obtenerDatoLimpioDelDiccionario(t_dictionary* diccionario, char* dato) {

	char* datoProvisorio;
	char* datoLimpio;
	datoProvisorio = dictionary_get(diccionario, dato);
	datoLimpio = string_substring_until(datoProvisorio,(string_length(datoProvisorio) - 1));
	return datoLimpio;
}

//----------------------------------------------------------------------------------------------------------------------------
void armarArchivoParaMapear(int bloque, char* ruta){
	FILE* archivo;
	archivo = fopen(ruta,"w");
	char* informacion = conseguirUnBloquePedido(bloque);
	fwrite(informacion,sizeof(char),strlen(informacion)+1,archivo);
	fclose(archivo);
	free(informacion);
}

char ejecutarMap(char* rutaScriptMap,char* archivoDondeLoGuardo, int bloque, char* rutaEntrada){
	armarArchivoParaMapear(bloque,rutaEntrada);

	printf("pasa esto\n");

	int queFuncionEs = 0;
	int resultadoRedireccionar = llamada_al_programa_redireccionando_stdin_out_ordenando(rutaScriptMap,archivoDondeLoGuardo,(void *) escribirEnArchivo,rutaEntrada,queFuncionEs);

	//remove(rutaEntrada); //borra el archivo que ya no sirve
	if(resultadoRedireccionar==-1)return 'X';
	return 'E';
}

char* generarRuta(){
	int indice = rand()%36;
	char* elementos[]={"0","1","2","3","4","5","6","7","8","9" ,"a", "b","c","d","e","f","g","h","i","j","k","l","m","n ","ñ","o","p","q","r","s","t", "u","v","w","x","y","z"};
	char* finRuta = elementos[indice];
	return finRuta;
}

char ejecutarReduce(char* rutaScriptReduce,char* archivoDondeLoGuardo,t_list* listaNodoNombres){


	//IMPRIME LA LISTA
	int tamanioLista = list_size(listaNodoNombres);
	int p=0;
	while(p<tamanioLista){
		estructuraNN* unaEstructura = list_get(listaNodoNombres,p);
		printf("id del Nodo: %d \n", unaEstructura->nodo.idNodo);
		int cantArchivos = list_size(unaEstructura->listaNombres);

		int l = 0;

		while(l<cantArchivos){
			char* unArchivo = list_get(unaEstructura->listaNombres,l);
			printf("unArchivo: %s\n", unArchivo);
			l++;
		}
		p++;
	}
	//

	char* rutaEntrada = generarNombreRandom();
	int j = 0;
	int cantidadArchivos = 0;
	while(list_size(listaNodoNombres)>j){
		estructuraNN* estructura1;
		t_list* listaNombres1;
		estructura1 = list_get(listaNodoNombres,j);
		listaNombres1 = estructura1->listaNombres;
		int cantidadArchivosEnNodo = list_size(listaNombres1);
		cantidadArchivos = cantidadArchivos + cantidadArchivosEnNodo;
		j++;
	}
	int i = 0;
	int indiceLista = 0;
	char* archivos[cantidadArchivos];
	while(cantidadArchivos>i){
		estructuraNN* estructura2;
		estructura2 = list_get(listaNodoNombres,indiceLista);
		nodo nodo = estructura2->nodo;
		t_list* listaNombres2 = estructura2->listaNombres;
		if(!(strcmp(nodo.ipNodo,ipNodo) || strcmp(nodo.puertoNodo,puertoNodo))){
			int k = 0;
			while(list_size(listaNombres2)>k){

				char* nombre = list_get(listaNombres2,k);
				archivos[i] = nombre;
				i++;
				k++;
			}
		}else{
			int q = 0;
			while(list_size(listaNombres2)>q){
				char* nombre = list_get(listaNombres2,q);
				archivos[i] = pedirBloqueAOtroNodo(nodo,nombre);
				i++;
				q++;
			}
		}
		indiceLista++;
	}
	apareoDeArchivos(cantidadArchivos,archivos,rutaEntrada);
	int queFuncionEs = 1;
	llamada_al_programa_redireccionando_stdin_out_ordenando(rutaScriptReduce,archivoDondeLoGuardo,(void *) escribirEnArchivo,rutaEntrada,queFuncionEs);
	return 'E';
}

void conseguirYEnviarLoQuePideElFS(socket_FS){
	uint32_t tamanio;
	char* nombreArchivo;
	recibir(socket_FS,&tamanio,sizeof(uint32_t));
	recibir(socket_FS,nombreArchivo,tamanio);

	int fd_archivo = open(nombreArchivo,O_RDONLY); //agregar al archivo de config!!
	char* mapeo;
	uint32_t tamanioArchivo = tamanio_archivo(fd_archivo);

	if( (mapeo = mmap(NULL, tamanioArchivo, PROT_READ, MAP_SHARED, fd_archivo, 0 )) == MAP_FAILED){
		//Si no se pudo ejecutar el MMAP, imprimir el error y aborta;
		printf("Error con MMAP");
		abort();
	}

//	char idProceso = 'N';
//	char idOperacion = 'A';

//	enviar(socket_FS,&idProceso,sizeof(char));
//	enviar(socket_FS,&idOperacion,sizeof(char));
	enviar(socket_FS,&tamanioArchivo,sizeof(uint32_t));
	enviar(socket_FS,mapeo,tamanioArchivo);
	munmap(mapeo,tamanioArchivo);
}



void recibirScript(int socketQueEscribe){
	char tipoScript;
	recibir(socketQueEscribe,&tipoScript,sizeof(char));

	char* identificador = malloc(15);
	identificador = string_itoa(socketQueEscribe);
	char* nombre = malloc(20);
	if(tipoScript=='m'){
		strcpy(nombre,"mapper.sh");
	}else{
		strcpy(nombre,"reducer.pl");
	}



	string_append(&identificador,nombre);


	/*FILE* archivo = fopen(identificador,"w");

	if(fchmod(fileno(archivo), 0755)==-1){
			printf("No se genero el permiso\n");
	}*/

	int fd_Archivo = creat(identificador,S_IRWXU);
	FILE* archivo = fdopen(fd_Archivo,"w");

	uint32_t longScript;
	recibir(socketQueEscribe,&longScript,sizeof(uint32_t));

	char* script = malloc(longScript);
	recibir(socketQueEscribe,script,longScript);

	fprintf(archivo,script);

	fclose(archivo);
	free(identificador);
	free(nombre);
	free(script);

}

void recibirOrdenMap(int socketQueEscribe){
	uint32_t nroBloqueDelNodo, longNombreEnDondeGuardarlo;
	recibir(socketQueEscribe,&nroBloqueDelNodo,sizeof(uint32_t));
	recibir(socketQueEscribe,&longNombreEnDondeGuardarlo,sizeof(uint32_t));
	char* nombreEnDondeGuardarlo = malloc(longNombreEnDondeGuardarlo);
	recibir(socketQueEscribe,nombreEnDondeGuardarlo,longNombreEnDondeGuardarlo);

	char* identificador = malloc(15);
	identificador = string_itoa(socketQueEscribe);
	char* nombre = "mapper.sh";
	char respuesta;
	//logearOrdenMap(nombre);
	string_append(&identificador,nombre);

	char* rutaEntrada = generarNombreRandom();

	respuesta = ejecutarMap(identificador,nombreEnDondeGuardarlo,nroBloqueDelNodo, rutaEntrada);

	enviarRespuestaDelMapAlJob(respuesta,socketQueEscribe);
	free(identificador);
	free(nombreEnDondeGuardarlo);
	remove(rutaEntrada);
}

void recibirOrdenReduce(int socketQueEscribe){
	uint32_t longNombreEnDondeGuardarlo;
	recibir(socketQueEscribe,&longNombreEnDondeGuardarlo,sizeof(uint32_t));
	char* nombreEnDondeGuardarlo = malloc(longNombreEnDondeGuardarlo);
	recibir(socketQueEscribe,nombreEnDondeGuardarlo,longNombreEnDondeGuardarlo);


	//uint32_t tamanioPaquete;
	//recv(socketQueEscribe, &tamanioPaquete, sizeof(tamanioPaquete),0);
	//printf("recibi el tamanioPaquete: %d\n", tamanioPaquete);
	//char* mensajeSerializado = malloc(tamanioPaquete);
	//recibir(socketQueEscribe,mensajeSerializado, tamanioPaquete);
	//t_list* listaNodoNombres = deserializarOrdenDeReduce(mensajeSerializado);

	t_list* listaNodoNombres = recibirEstructurasDelNodo(socketQueEscribe);

	char* identificador = string_itoa(socketQueEscribe);
	char* nombre = "reducer.pl";
	//logearOrdenReduce(identificador,nombre);
	string_append(&identificador,nombre);

	char respuesta = ejecutarReduce(identificador,nombreEnDondeGuardarlo,listaNodoNombres);

	enviarRespuestaDelMapAlJob(respuesta,socketQueEscribe);

	free(nombreEnDondeGuardarlo);
	list_destroy_and_destroy_elements(listaNodoNombres, (void*) estructuraN_destroy);
}


static void estructuraN_destroy(estructuraNN *self) {
    free(self->nodo.ipNodo);
    free(self->nodo.puertoNodo);
    list_destroy_and_destroy_elements(self->listaNombres, (void*) nombres_destroy);
    free(self);
}

static void nombres_destroy(char *self) {
    free(self);
}

t_list*  recibirEstructurasDelNodo(int socketQueEscribe){
	uint32_t cantidadEstructuras;

	recv(socketQueEscribe,&cantidadEstructuras,sizeof(int),0);


	t_list* listaEstructura = list_create();
	int i=0;
	while(i<cantidadEstructuras){
		 estructuraNN* unaEstructura = malloc(sizeof(estructuraNN));

		recv(socketQueEscribe,&(unaEstructura->nodo.idNodo),sizeof(int),0);


		(unaEstructura->nodo).ipNodo = recibirCadena(socketQueEscribe);
		(unaEstructura->nodo).puertoNodo = recibirCadena(socketQueEscribe);


			int j=0;
			int cantidadBloques ;

			recv(socketQueEscribe,&cantidadBloques,sizeof(int),0);


			unaEstructura->listaNombres=list_create();

			while(j<cantidadBloques){
				char* cadena = recibirCadena(socketQueEscribe);
				list_add(unaEstructura->listaNombres,cadena);


				j++;
			}
			list_add(listaEstructura,unaEstructura);
			i++;
		}

		return listaEstructura;

}

t_list* deserializarOrdenDeReduce(char* mensajeSerializado){

	t_list* listaNodoNombres = list_create();
	list_clean(listaNodoNombres);
	uint32_t cantidadEstructuras;

	int offset = 0;
	memcpy(&cantidadEstructuras,mensajeSerializado,sizeof(cantidadEstructuras));
	offset+=sizeof(cantidadEstructuras);



	int i;
	for(i=0; i<cantidadEstructuras; i++){

		uint32_t idNodo;
		memcpy(&idNodo,mensajeSerializado + offset,sizeof(idNodo));
		offset+=sizeof(idNodo);

		uint32_t longIp;
		memcpy(&longIp,mensajeSerializado + offset,sizeof(longIp));
		offset+=sizeof(longIp);

		char* ipNodo = malloc(longIp);
		memcpy(ipNodo,mensajeSerializado + offset,longIp);
		offset+=longIp;


		uint32_t longPuerto;
		memcpy(&longPuerto,mensajeSerializado + offset,sizeof(longPuerto));
		offset+=sizeof(longPuerto);

		char* puerto = malloc(longPuerto);
		memcpy(puerto,mensajeSerializado + offset,longPuerto);
		offset+=longPuerto;

		estructuraNN* unaEstructura = malloc(sizeof(estructuraNN));
		unaEstructura->nodo.idNodo = idNodo;
		strcpy(unaEstructura->nodo.ipNodo,ipNodo);
		strcpy(unaEstructura->nodo.puertoNodo,puerto);
		unaEstructura->listaNombres = list_create();

		uint32_t cantidadBloques;
		memcpy(&cantidadBloques,mensajeSerializado + offset,sizeof(cantidadBloques));
		offset+=sizeof(cantidadBloques);

		int j;
		for(j=0; j < cantidadBloques; j++){

			uint32_t longNombre;
			memcpy(&longNombre,mensajeSerializado + offset,sizeof(longNombre));
			offset+=sizeof(longNombre);

			char* nombreEnDondeGuardarlo = malloc(longNombre);
			memcpy(nombreEnDondeGuardarlo,mensajeSerializado + offset,longNombre);
			offset+=longNombre;

			list_add(unaEstructura->listaNombres,nombreEnDondeGuardarlo);

		}


		list_add(listaNodoNombres,unaEstructura);

	}

	return listaNodoNombres;
}


char* pedirBloqueAOtroNodo(nodo nodo, char* nombre){
	int socket = conectarANodo(nodo.ipNodo,nodo.puertoNodo);
	char idProceso = 'N';
	char idOperacion = 'P';
	int tamanio = sizeof(char)*(strlen(nombre)+1);
	send(socket,&idProceso,sizeof(char),0);
	send(socket,&idOperacion,sizeof(char),0);
	send(socket,&tamanio,sizeof(int),0);
	send(socket,nombre,tamanio,0);


	recibirBloque(socket,nombre);
	return nombre;
}

void conseguirBloqueYEnviar(int socket){
	int tamanio;
	recv(socket,&tamanio,sizeof(int),0);
	char* nombre = malloc(tamanio);
	recv(socket,nombre,tamanio,0);


	int fd_archivo = open(nombre,O_RDONLY);
	char* bloqueConseguido;
	int tamanioArchivo = tamanio_archivo(fd_archivo);

	if( (bloqueConseguido = mmap(NULL, tamanioArchivo, PROT_READ, MAP_SHARED, fd_archivo, 0 )) == MAP_FAILED){
		//Si no se pudo ejecutar el MMAP, imprimir el error y aborta;
		printf("Error con MMAP");
		abort();
	}

	int tamanioEnvio = tamanioArchivo+1;
	enviar(socket,&tamanioEnvio,sizeof(uint32_t));
	enviar(socket,bloqueConseguido,tamanioEnvio);

	free(nombre);
	close(fd_archivo);
	munmap(bloqueConseguido,tamanioEnvio);
}

void recibirBloque(int socket,char* nombre){
	FILE* archivo = fopen(nombre,"a");
	uint32_t longitud;
	recibir(socket, &longitud, sizeof(uint32_t));
	char* bloque = malloc(longitud);
	//memset(bloque,'\0',longitud);
	recibir(socket, bloque,longitud);

	/*char* a = "\n";
	char *contenido1 = malloc(longitud+5);
	contenido1 =strcat(bloque,a);
	free(bloque);
	fwrite(contenido1,longitud,1,archivo);


	free(contenido1);*/
	bloque[longitud] ='\n';
	fwrite(bloque,sizeof(char),longitud,archivo);
	free(bloque);

	//loguearEscrituraDeEspacioTemporal();
	fclose(archivo);

}

// ENVIA Y RECIBE TODO

int enviar(int s, char *buf, int len)
   {
       int total = 0;        // cuántos bytes hemos enviado
       int bytesleft = len; // cuántos se han quedado pendientes
       int n;
       while(total < len) {
           n = send(s, buf+total, bytesleft, 0);
           if (n == -1) { break; }
           total += n;
           bytesleft -= n;
       }
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
       return n==-1?-1:0; // devuelve -1 si hay fallo, 0 en otro caso
   }

void enviarRespuestaDelMapAlJob(char respuesta, int socketQueEscribe){

	send(socketQueEscribe,&respuesta,sizeof(char),0);

}
//---------------Formateo del archivo de log---------------------------

void formatearArchivoDeLog(){

	remove("LogNodo");


}


//-------------------------Archivo de Log---------------------------
void loguearConexionConProceso(char* nombreProcesoAConectarse,int selogroConectar, int socket){

	t_log_level nivel;
	t_log* archivoDeLog;


	if (selogroConectar == 0) {

	nivel = LOG_LEVEL_INFO;
	archivoDeLog = log_create("LogNodo", "Nodo", 0, nivel);
	 char *unaPalabra = string_new();
			 string_append(&unaPalabra, "Conexion con ");
			 string_append(&unaPalabra, nombreProcesoAConectarse);

			 string_append(&unaPalabra, " usando el socket ");
			 string_append(&unaPalabra, string_itoa(socket));

	log_info(archivoDeLog,unaPalabra);
	log_destroy(archivoDeLog);
	free(unaPalabra);
	}else{

		nivel = LOG_LEVEL_ERROR;
		archivoDeLog = log_create("LogNodo", "Nodo", 0, nivel);
		 char *unaPalabra = string_new();
			 string_append(&unaPalabra, "Fallo conexion con ");
			 string_append(&unaPalabra, nombreProcesoAConectarse);


			 log_error(archivoDeLog,unaPalabra);
			 free(unaPalabra);
			 log_destroy(archivoDeLog);


}



}

void loguearDesconexionDeProceso(char* nombreProceso,int nroSocket){

	t_log_level nivel;
	t_log* archivoDeLog;
	nivel = LOG_LEVEL_INFO;
		archivoDeLog = log_create("LogNodo", "Nodo", 0, nivel);
		 char *unaPalabra = string_new();
				 string_append(&unaPalabra, "Desconexion de ");
				 string_append(&unaPalabra, nombreProceso);

				 string_append(&unaPalabra, " usando el socket ");
				 string_append(&unaPalabra, string_itoa(socket));

		log_info(archivoDeLog,unaPalabra);
		free(unaPalabra);
		log_destroy(archivoDeLog);
}

void aceptacionDeProceso(char* nombreProceso, int socket){
	t_log_level nivel;
		t_log* archivoDeLog;
		nivel = LOG_LEVEL_INFO;
			archivoDeLog = log_create("LogNodo", "Nodo", 0, nivel);
			 char *unaPalabra = string_new();
					 string_append(&unaPalabra, "Aceptacion de ");
					 string_append(&unaPalabra, nombreProceso);

					 string_append(&unaPalabra, " usando el socket ");
					 string_append(&unaPalabra, string_itoa(socket));

			log_info(archivoDeLog,unaPalabra);
			free(unaPalabra);
			log_destroy(archivoDeLog);

}

void logearOrdenMap(char* nombre) {

	t_log_level nivel;

	t_log* archivoDeLog;

	nivel = LOG_LEVEL_INFO;

	archivoDeLog = log_create("LogNodo", "Nodo", 0, nivel);

	 char *unaPalabra = string_new();

	 string_append(&unaPalabra, "Solicitud de mapping con el script ");

	 string_append(&unaPalabra, nombre);




	log_info(archivoDeLog,unaPalabra);

	free(unaPalabra);
	log_destroy(archivoDeLog);
}


void logearOrdenReduce(char* nombre){

	t_log_level nivel;

	t_log* archivoDeLog;

	nivel = LOG_LEVEL_INFO;

	archivoDeLog = log_create("LogNodo", "Nodo", 0, nivel);

		 char *unaPalabra = string_new();

		 string_append(&unaPalabra, "Solicitud de Reduce con el script ");

		 string_append(&unaPalabra, nombre);






		log_info(archivoDeLog,unaPalabra);

		free(unaPalabra);
		log_destroy(archivoDeLog);

}

void loguearLecturaDeBloques(int indiceDelBloque){

	t_log_level nivel;
		t_log* archivoDeLog;
		nivel = LOG_LEVEL_INFO;
		archivoDeLog = log_create("LogNodo", "Nodo", 0, nivel);

		 char *unaPalabra = string_new();
							 string_append(&unaPalabra, "Lectura del bloque: ");

							 string_append(&unaPalabra, string_itoa(indiceDelBloque));

		log_info(archivoDeLog,unaPalabra);
		free(unaPalabra);

					free(unaPalabra);
								 log_destroy(archivoDeLog);
}

void loguearEscrituraDeEspacioTemporal(){

	t_log_level nivel;
		t_log* archivoDeLog;
		nivel = LOG_LEVEL_INFO;
		archivoDeLog = log_create("LogNodo", "Nodo", 0, nivel);

		log_info(archivoDeLog,"Escritura en Espacio Temporal");


					 log_destroy(archivoDeLog);
}


void loguearFalloDeEscrituraDeEspacioTemporal(){

	t_log_level nivel;
		t_log* archivoDeLog;
		nivel = LOG_LEVEL_ERROR;
		archivoDeLog = log_create("LogNodo", "Nodo", 0, nivel);

		log_info(archivoDeLog,"Fallo escritura en Espacio Temporal");


					 log_destroy(archivoDeLog);
}


void loguearLecturaDeEspacioTemporal(){
	t_log_level nivel;
			t_log* archivoDeLog;
			nivel = LOG_LEVEL_INFO;
			archivoDeLog = log_create("LogNodo", "Nodo", 0, nivel);

			log_info(archivoDeLog,"Lectura en Espacio Temporal");

			 log_destroy(archivoDeLog);

}

void loguearEscrituraEnBloque(int indiceDelBloque){

	t_log_level nivel;
			t_log* archivoDeLog;
			nivel = LOG_LEVEL_INFO;
			archivoDeLog = log_create("LogNodo", "Nodo", 0, nivel);



			 char *unaPalabra = string_new();
										 string_append(&unaPalabra, "Escritura del bloque: ");

										 string_append(&unaPalabra, string_itoa(indiceDelBloque));

								log_info(archivoDeLog,unaPalabra);

								 log_destroy(archivoDeLog);
								 free(unaPalabra);

}

//GENERA RANDOM

char* generarNombreRandom(){
	char* nombreAleatorio;
		nombreAleatorio = string_new();
		 int numero;

		  srand(rdtsc()); //semilla para crear el nro random
		 numero = rand();
		 nombreAleatorio = string_itoa(numero);

		return nombreAleatorio;
}

int rdtsc()
{
	__asm__ __volatile__("rdtsc");
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


void conseguirArchivoFinal(int socket_FS){
	uint32_t tamanioRuta;
	recibir(socket_FS,&tamanioRuta,sizeof(uint32_t));
	char* ruta = malloc(tamanioRuta);
	recibir(socket_FS,ruta,tamanioRuta);
	uint32_t tamanioNombre;
	recibir(socket_FS,&tamanioNombre,sizeof(uint32_t));
	char* nombre = malloc(tamanioNombre);
	recibir(socket_FS,nombre,tamanioNombre);

	int fd_archivo = open(nombre,O_RDONLY);
	char* mapeo;
	int tamanioArchivo = tamanio_archivo(fd_archivo);

	if( (mapeo = mmap(NULL, tamanioArchivo, PROT_READ, MAP_SHARED, fd_archivo, 0 )) == MAP_FAILED){
	//Si no se pudo ejecutar el MMAP, imprimir el error y aborta;
		printf("Error con MMAP");
		abort();
		}

	char idProceso = 'N';
	char idOperacion = 'T';
	int tamanioEnvio = tamanioArchivo + 1;

	enviar(socket_FS,&idProceso,sizeof(char));
	enviar(socket_FS,&idOperacion,sizeof(char));
	enviar(socket_FS,&tamanioRuta,sizeof(uint32_t));
	enviar(socket_FS,ruta,tamanioRuta);
	enviar(socket_FS,&tamanioEnvio,sizeof(uint32_t));
	enviar(socket_FS,mapeo,tamanioEnvio);
	close(fd_archivo);
	munmap(mapeo,tamanioEnvio);
}

void conseguirBloqueYEnviarAlFS(int socket_FS){
	int bloqueABuscar;
	recibir(socket_FS,&bloqueABuscar,sizeof(int));
	int tamanioRuta;
	recibir(socket_FS,&tamanioRuta,sizeof(int));
	char* rutaLinux = malloc(tamanioRuta);
	recibir(socket_FS,rutaLinux,tamanioRuta);

	char* bloqueLeido;
	bloqueLeido = conseguirUnBloquePedido(bloqueABuscar);
	int tamanio = strlen(bloqueLeido)+1;

	//loguearLecturaDeBloques(bloqueABuscar);
	char idProceso = 'N';
	char idOperacion = 'B';

	enviar(socket_FS,&idProceso,sizeof(char));
	enviar(socket_FS,&idOperacion,sizeof(char));
	enviar(socket_FS,&tamanioRuta,sizeof(int));
	enviar(socket_FS,rutaLinux,tamanioRuta);
	enviar(socket_FS,&tamanio,sizeof(int));
	enviar(socket_FS,bloqueLeido,tamanio);

	free(bloqueLeido);
	free(rutaLinux);


}

void deletearBloque(int socket_FS,FILE* archivoAbierto){
	//fclose(ArchivoAbierto);
	int bloqueABorrar;
	recibir(socket_FS,&bloqueABorrar,sizeof(int));
	int offset = 20971520*bloqueABorrar;
	fseek(archivoAbierto,offset,SEEK_SET);

}

void cambiarCondicionANodoViejo(){
	formatearArchivoDeConfig();
		char* configDelParametro;
		char* parametro;


		nodoNuevo = "N";

		int i;
		for(i=0;i<7;i++){
			if (i==0){
				parametro = "puertoFS";
				configDelParametro = puertoFileSystem;
				guardarParametrosDeConfiguracion(parametro,configDelParametro);
				}
			if (i==1){
				parametro = "ipFS";
				configDelParametro = ipFileSystem;
				guardarParametrosDeConfiguracion(parametro,configDelParametro);
			}
		/*	if (i==2){
				parametro = "archivoBin";
				configDelParametro = archivoBin;
				guardarParametrosDeConfiguracion(parametro,configDelParametro);
					}
			if (i==3){
				parametro = "dirTemp";
				configDelParametro = dirTemp;
				guardarParametrosDeConfiguracion(parametro,configDelParametro);
							} */
			if (i==2){
				parametro = "nodoNuevo";
				configDelParametro = nodoNuevo;
				guardarParametrosDeConfiguracion(parametro,configDelParametro);
									}
			if (i==3){
				parametro = "ipNodo";
				configDelParametro = ipNodo;
				guardarParametrosDeConfiguracion(parametro,configDelParametro);
											}
			if (i==4){
				parametro = "puertoNodo";
				configDelParametro = puertoNodo;
				guardarParametrosDeConfiguracion(parametro,configDelParametro);
											}
			if (i==5){
							parametro = "tamanioNodo";
							configDelParametro = tamanioNodo;
							guardarParametrosDeConfiguracion(parametro,configDelParametro);
														}
			if (i==6){
									parametro = "dirTemp";
									configDelParametro = dirTemp;
									guardarParametrosDeConfiguracion(parametro,configDelParametro);
																}

		}


	}
void guardarParametrosDeConfiguracion( char* parametro, char* configDelParametro){

	FILE* archivo;
	archivo = fopen("configNodo","a");

		char* coma;
		char* resultado;
		resultado = string_new();
		char* resultadoFinal;
		resultadoFinal = string_new();
		coma = string_new();
		char* preResultado;
		preResultado = string_new();

		coma = ",";

		string_append(&preResultado,parametro);
		string_append(&preResultado,coma);  //le pone la coma al parametro por EJ puerto => puerto,

		string_append(&resultado,preResultado);
		string_append(&resultado,configDelParametro);						//concatena lo del paso anterior con el dato ingresado por EJ puerto, => puerto,4500


		string_append(&resultadoFinal,resultado);
		string_append(&resultadoFinal,"\n");       //le agrega el \n al final para que printf escriba y avance a la linea siguiente y no siga escribiendo todo pegado

		fprintf(archivo, "%s", resultadoFinal);

		fclose(archivo);

}


void formatearArchivoDeConfig(){

	remove("configNodo");


}
