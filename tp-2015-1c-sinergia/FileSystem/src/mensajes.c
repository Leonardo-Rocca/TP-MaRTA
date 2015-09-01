/*
 * mensajes.c
 *
 *  Created on: 3/6/2015
 *      Author: utnso
 */
#include "mensajes.h"
#include "md5.h"

int primerArchivo=1;
t_list* listaTamanioEstructuras;
int varDistribucion = 0;
// DIVIDE ARCHIVO EN BLOQUES, DADO EL FD DEL ARCHIVO. ADEMAS RECIBE LA ESTRUCTURA ARCHIVO PARA CREAR ESTRUCTURA DE BLOQUES

int distribuirArchivoEnBloques(int fd_archivoADividir, struct archivos *archivoEstructura){  //RECIBE FD del archivo, por lo tanto abrir archivo con OPEN.
	char * mapeo;
	int tamanioArchivo;
	int finalBloque = 20971518;
	int tamanio_bloques = 20971520; //Supongo bloques de 10 bytes. Palabras con su barra n al final + barra n de fin de bloque. En este caso 9 + barra n de fin de bloque.
	idNodoBloquePedido1 = -1;
	idNodoBloquePedido2 = -1;

	tamanioArchivo = tamanio_archivo(fd_archivoADividir);
	tamanioChanta = tamanioArchivo;// chanta
	flagChanta = 0;//chanta
	idNodoChanta = -1;

	if( (mapeo = mmap(NULL, tamanioArchivo, PROT_READ, MAP_SHARED, fd_archivoADividir, 0 )) == MAP_FAILED){
				//Si no se pudo ejecutar el MMAP, imprimir el error y aborta;
				printf("Error con MMAP en Distribucion de bloques\n");
				return 0;
	}

	char *aux;  //creo un puntero auxiliar para no perder la referencia(mapeo) del mmap.
	aux=mapeo;
	int h=0;
	char * ptr;
	struct bloques *unBloque = malloc(sizeof(struct bloques));
	struct bloques *ptrBloque = malloc(sizeof(struct bloques));
	archivoEstructura->bloque = unBloque;
	ptrBloque = unBloque;

	int idBloques=0;
	int bloqueResto = 0;
	while(h<= tamanioArchivo && bloqueResto!=1){

		char * punteroDestino; //puntero en donde se guarda provisoriamente un bloque.
		punteroDestino = (char *) malloc (sizeof(char)*tamanio_bloques);

		if(h+20971519 > tamanioArchivo){
			bloqueResto = 1;
			int resto = tamanioArchivo - h;
			memcpy(punteroDestino,aux,resto);
			rellenarBloque(tamanio_bloques - resto -1, &punteroDestino);
		}else{

		if(aux[finalBloque]=='\n'){  //si en la posicion 8, es decir elemento 9 (ultimo elemento que puede tener mi bloque) no me queda cortada una palabra.
			memcpy(punteroDestino,aux,20971519); //copio los 9 bytes al puntero.
			punteroDestino[20971519]='\n'; //agrego barra n de fin de bloque.
			aux=aux+20971519; //para la proxima iteración del for.
			h=h+20971519; //posicion en el aux

		}else{			//si me queda cortada la palabra
			int retroceso=1;
			ptr = aux + finalBloque;
			while(*(--ptr) !='\n'){ //retrocedo hasta encontrar un barra n.
				retroceso++;
			}

			memcpy(punteroDestino,aux,20971519-retroceso); //copio los bytes hasta inclusive dicho barra n.
			rellenarBloque(retroceso,&punteroDestino); //relleno el bloque con 0 al final para que sea de 10bytes.
			aux=aux+(20971519-retroceso); //para la proxima iteración del for.
			h=h+(20971519-retroceso); //posicion en el aux
		}

		}

	armarYEnviarBloqueANodo(punteroDestino,idBloques, ptrBloque);

	if(ptrBloque->numBloque==-1){
		printf("Abortando la operacion de distribucion de bloques a los nodos\n");
		loguearErrores("Abortando la operacion de distribucion de bloques a los nodos");
		(*archivoEstructura).bloquesDeCadaArchivo = idBloques+1;
		return 0; //se requieren mas nodos para seguir distribuyendo
	}

	if(!bloqueResto){
		struct bloques* bloqueSig = malloc(sizeof(struct bloques));
		ptrBloque->bloque = bloqueSig;
		ptrBloque = ptrBloque->bloque;
		idBloques++;
	}
	free(punteroDestino);
	}
	ptrBloque->bloque = NULL;
	(*archivoEstructura).bloquesDeCadaArchivo = idBloques + 1;
	//free(ptrBloque);
	munmap(mapeo, tamanioArchivo);
	return 1;
}


void armarYEnviarBloqueANodo(char * punteroDestino,int idBloques,struct bloques *ptrBloque){
	armaEstructurasYEnviaCopias(idBloques, ptrBloque, punteroDestino);
}

void armaEstructurasYEnviaCopias(int idBloques, struct bloques *ptrBloque, char* contenidoBloque){

	ptrBloque->numBloque = idBloques;
	struct copias *unaCopia = malloc(sizeof(struct copias));
	struct copias *ptrCopia = malloc(sizeof(struct copias));
	ptrBloque->copia = unaCopia;
	ptrCopia = unaCopia;

	int cantNodosActivosYconLugar = cantidadDeNodosActivosYConLugar();

	if(cantNodosActivosYconLugar<3){
		printf("Se requieren mas nodos para la operacion. No es posible distribuir las 3 copias en 3 nodos distintos\n");
		loguearErrores("Se requieren mas nodos para la operacion. No es posible distribuir las 3 copias en 3 nodos distintos");
		ptrBloque->numBloque = -1;
		ptrBloque->copia->nroCopia = -1;
		//formatearNodos
		return;
	}

	int i = 0;
	while(i < 3){
		printf("NRO BLOQUE: %d ", ptrBloque->numBloque);
		ptrCopia->nroCopia = i;
		enviaCopiaANodo(ptrCopia,contenidoBloque);
		if(ptrCopia->nroCopia == -1){
			ptrBloque->numBloque = -1;
			return;
			//formatearNodos
		}
		loguearCopiasEnviadas(ptrBloque->numBloque,ptrCopia->nroCopia,ptrCopia->unNodo->idNodo, ptrCopia->unNodo->nroBloqueNodo);
		if(i<2){
			struct copias* ptrSig = malloc(sizeof(struct copias));
			ptrCopia->copia = ptrSig;
			ptrCopia = ptrCopia->copia;
		}
	i++;
	}
	ptrCopia->copia=NULL;
	(*ptrBloque).copiasDeCadaBloque = i;
	//free(ptr);
}

int cantidadDeNodosActivosYConLugar(){
	return list_count_satisfying(socketsConectados,nodoActivoYConLugar);
}

bool nodoActivoYConLugar(void *elemento) {
	struct nodos* unNodo = (struct nodos*) elemento;
	int i=0;
	int hayLugar = 0;
	while(i< (unNodo->cantidadDeBloques) && hayLugar !=1){
			if(!bitarray_test_bit(unNodo->bloques, i)){
				hayLugar = 1;
			}
			i++;
	}

    return ((unNodo->estado) == 1) && (hayLugar);
}


bool nodosActivos(void *elemento) {
	struct nodos* unNodo = (struct nodos*) elemento;

    return ((unNodo->estado) == 1);
}

void enviaCopiaANodo(struct copias* copia, char* contenidoBloque){
	struct nodoPos* posNodo = obtenerDestinoDeBloque(copia->nroCopia);
	copia->unNodo = posNodo;
	if(posNodo->idNodo!=-1){ // si tiran un nodo mientras se esta haciendo la distribucion
		int nroSocket = posNodo->unNodo->nroSocket;
		int nroBloqueNodo = posNodo->nroBloqueNodo;
		t_Package package;
		printf("NRO COPIA: %d NRO NODO: %d NRO BLOQUE NODO: %d \n",copia->nroCopia, posNodo->idNodo, nroBloqueNodo);


		package.bloque = malloc(sizeof(contenidoBloque));
		fill_package(&package,contenidoBloque,nroBloqueNodo);
		char* mensajeSerializado = serializarEnvioDeBloqueANodo(&package);

		struct nodos* unNodo = obtenerNodo(posNodo->idNodo);
		if(unNodo->estado==1){
			int resultadoEnviar = enviar2(nroSocket, mensajeSerializado, (package.total_size + 2*sizeof(char) + 1));
			if(resultadoEnviar==-1){
				printf("fallo el envio de un bloque. Posible motivo: caida de un nodo.\n");
				loguearErrores("fallo el envio de un bloque. Posible motivo: caida de un nodo.");
				copia->nroCopia = -1;
			}
		}else{
			printf("fallo el envio de un bloque. Posible motivo: caida de un nodo.\n");
			loguearErrores("fallo el envio de un bloque. Posible motivo: caida de un nodo.");
			copia->nroCopia = -1;
		}
		free(mensajeSerializado);

	}else{
		printf("Bloque no enviado! . NO HAY ESPACIO EN LOS NODOS!\n");
		loguearErrores("Bloque no enviado! . NO HAY ESPACIO EN LOS NODOS!");
		copia->nroCopia = -1;
	}
}

_Bool comparatorMenorAMayor (struct nodos* estructuraDeTrabajo1, struct nodos* estructuraDeTrabajo2){// ahora es de menor a mayor
	 int bloquesLibres1 = cantBloquesLibres(estructuraDeTrabajo1->bloques, estructuraDeTrabajo1->cantidadDeBloques);
	 int lugarDisponible1 = bloquesLibres1 * 20971520;
	 int bloquesLibres2 = cantBloquesLibres(estructuraDeTrabajo2->bloques, estructuraDeTrabajo2->cantidadDeBloques);
	 int lugarDisponible2 = bloquesLibres2 * 20971520;

	 return (lugarDisponible1 > lugarDisponible2);
}

struct nodoPos* obtenerDestinoDeBloque(int nroCopia){
	t_list* nodosActivosYConLugar = list_filter(socketsConectados,nodoActivoYConLugar);
	int cantElementos = list_size(nodosActivosYConLugar);
	struct nodoPos* nodoPosicion = malloc(sizeof(struct nodoPos));

	if(cantElementos!=0){

		if(nroCopia==0){
			if(cantElementos<3) return nodoPosicion->idNodo=-1;
		}

		if(nroCopia==1){
			if(cantElementos<2) return nodoPosicion->idNodo=-1;
		}

		if(nroCopia==2){
			if(cantElementos<1) return nodoPosicion->idNodo=-1;
		}

		struct nodos* nodo;
		struct nodos* nodo2;

		if(nroCopia == 0){

			if(flagChanta == 0){
				t_list* unaLista = list_create();
				list_add_all(unaLista,nodosActivosYConLugar);
				list_sort(unaLista,*comparatorMenorAMayor);
				int q = 0;
				while(q < cantElementos){
					nodo = list_get(unaLista,q);
					int bloquesLibres1 = cantBloquesLibres(nodo->bloques, nodo->cantidadDeBloques);
					int lugarDisponible1 = bloquesLibres1 * 20971520;
					if(tamanioChanta <= lugarDisponible1){
						idNodoChanta = nodo->idNodo;
						break;
					}
					q++;
				}
				if(q == cantElementos){
					idNodoChanta = nodo->idNodo;
				}
				flagChanta = 1;
			}

			int a=0;

			while(a < cantElementos){
				nodo = list_get(nodosActivosYConLugar,a);
				if(nodo->idNodo == idNodoChanta) break;
				a++;
			}
			if(a == cantElementos){ // esta bien .. por si se cae el nodo que tiene todo
				nodo = list_get(nodosActivosYConLugar,varDistribucion);
			}

		idNodoBloquePedido1 = nodo->idNodo;

		}else{ // si es la copia 1 o 2
			if(nroCopia == 1){
				t_list* unaLista2 = list_create();
				list_add_all(unaLista2,nodosActivosYConLugar);
				int w = 0;

				while(w < cantElementos){
					nodo2 = list_get(unaLista2,w);
					if (nodo2->idNodo == idNodoBloquePedido1) break;
					w++;
				}
				list_remove(unaLista2,w);
				list_sort(unaLista2,*comparatorMenorAMayor);
				nodo = list_get(unaLista2,0);
				idNodoBloquePedido2 = nodo->idNodo;
			}

			if(nroCopia == 2){
			if(varDistribucion>=cantElementos){
				varDistribucion = 0;
			}
			int k = 0;
			while(k < cantElementos){
			nodo = list_get(nodosActivosYConLugar,varDistribucion);
			if(nodo->idNodo == idNodoBloquePedido1 || nodo->idNodo == idNodoBloquePedido2){
				if(cantElementos - 1 == varDistribucion){
					varDistribucion = 0;
					}else{
					varDistribucion++;
					}
				}else{
					if(cantElementos - 1 == varDistribucion){
						varDistribucion = 0;
						}else{
						varDistribucion++;
						}
					break;
				}k++;
			}
			}
		}

//	struct nodos* otroNodo = list_get(nodosActivosYConLugar,cantElementos-1);
		if(nroCopia == 2){
			if(cantElementos - 1 == varDistribucion){
				varDistribucion = 0;
			}else{
				varDistribucion++;
			}
		}
		//armaEstructura que necesita enviaCopiaANodo. Puede cambiarse y ponerse afuera
		nodoPosicion->unNodo = nodo;
		nodoPosicion->idNodo = nodo->idNodo;

	/*if(maximaCantidad==0){ //no hay espacio en los nodos!!
		nodoPosicion->idNodo = -1;
		nodoPosicion->nroBloqueNodo = -1;
		return nodoPosicion;
	}*/

		int j=0;
		while((bitarray_test_bit(nodo->bloques,j)!=0) && j<(nodo->cantidadDeBloques)){
			j++;
		}

		if(j<(nodo->cantidadDeBloques)){
			nodoPosicion->nroBloqueNodo=j;
			bitarray_set_bit(nodo->bloques,j);
		}

	}else{
		nodoPosicion->idNodo = -1;
	}

	return nodoPosicion;
}

void* cantidadDeBloquesLibres(void* element){
	struct nodos* unNodo = (struct nodos*)element;
	int cantidadDeBloquesLibres = 0;

	if(unNodo->estado==0){
		int* resultado;
		cantidadDeBloquesLibres = -1;
		resultado = &cantidadDeBloquesLibres;
		return (*resultado);
	}

	t_bitarray* bitbloq = unNodo->bloques;
	int i=0;
	while(i< (unNodo->cantidadDeBloques) ){
		if(!bitarray_test_bit(bitbloq, i)){
			cantidadDeBloquesLibres++;
		}
		i++;
	}
	int* resultado;
	resultado = &cantidadDeBloquesLibres;
	return (*resultado);
}

//RELLENA EL BLOQUE PARA QUE TENGA UN TAMANIO FIJO---------------------------------------------------------
void rellenarBloque(int retroceso, char **punteroDestino){

	while(retroceso>0){
		(*punteroDestino)[20971519-retroceso]='0';		//agrego 0 para hacer un puntero con 9 elementos y poder lograr un bloque de 10bytes.
		retroceso--;
	}
	(*punteroDestino)[20971519] ='\n';

}



//DADO UN FD DE UN ARCHIVO, SE OBTIENE SU TAMANIO--------------------------------------------------------------------
int tamanio_archivo(int fd){
	struct stat buf; //se crea una estructura buf de tipo stat(estructura que tiene un fd).
	fstat(fd, &buf);  //un poco de magia, "arma" la estructura que llamamos buf con el fd pasado por parametro.
	return buf.st_size; //retorna el tamanio del archivo.
}


//SERIALIZACION PARA ENVIAR BLOQUES A UN NODO -----------------------------------

 char* serializarEnvioDeBloqueANodo(t_Package* package){
	 char *serializedPackage = malloc(package->total_size + 2*sizeof(char) + 1);

	 int offset = 0;
	 int size_to_send;

	 char idProceso='F';
	 char idOperacion='B';

	 size_to_send = sizeof(char);
	 memcpy(serializedPackage + offset,&idProceso,size_to_send );
	 offset+=size_to_send;

	 size_to_send = sizeof(char);
	 memcpy(serializedPackage + offset,&idOperacion,size_to_send );
	 offset+=size_to_send;

	 size_to_send = sizeof(int);
	 memcpy(serializedPackage + offset, &(package->nroBloque),size_to_send);
	 offset+=size_to_send;

	 size_to_send = sizeof(package->bloque_long);
	 memcpy(serializedPackage + offset, &(package->bloque_long), size_to_send);
	 offset += size_to_send;

	 size_to_send = package->bloque_long;
	 memcpy(serializedPackage + offset, package->bloque, size_to_send);
	 offset += size_to_send;

	 return serializedPackage;
 }


 void fill_package(t_Package *package, char *bloque, int nroBloque){
	 package->bloque = bloque;
	 package->bloque_long = strlen(bloque) + 1;
	 package->nroBloque = nroBloque;
	 package->total_size = sizeof(package->bloque_long) + package->bloque_long + sizeof(int);

 }

char** desglosarRuta(char* ruta){
	return string_split(ruta, "/");
}

int obtenerUltimaPosicionDeUnArray(char** arrayDeDirectorios){
	int i=0;
		while(arrayDeDirectorios[i]!=NULL){
			i++;
		}
	return i;
}

//COPIAR UN ARCHIVO LOCAL AL MDFS DADA LA RUTA DE DICHO ARCHIVO------------------------

 void copiarArchivoLocalAlMDFS(char * path){
	 char ** arrayDeDirectorios = desglosarRuta(path);
	 int ultimaPos = obtenerUltimaPosicionDeUnArray(arrayDeDirectorios);
	 char* nombreDelArchivo = arrayDeDirectorios[ultimaPos - 1];
	 int fd_archivo = open(path,O_RDONLY);
	 int tamanioArchivo = tamanio_archivo(fd_archivo);

	 if(sePuedeDistribuirElArchivo(tamanioArchivo)){

		 struct archivos* archivo = malloc(sizeof(struct archivos));
		 crearDirectorios(sacaElArchivoALaRuta(path));
		 archivo->nombre = malloc(sizeof(nombreDelArchivo));
		 strcpy(archivo->nombre,nombreDelArchivo);
		 archivo->dirPadre = obtenerIdDelUltimoDirectorioDe(sacaElArchivoALaRuta(path));
		 archivo->tamanio = tamanioArchivo;
		 archivo->estado = 1;

		 int resultadoOp = distribuirArchivoEnBloques(fd_archivo,archivo);

		 if(resultadoOp){

			 if(primerArchivo){
				 ptrArchivo = archivo;
				 aux_ptrArchivo = archivo;
				 primerArchivo=0;
			 }else{
				 aux_ptrArchivo->archivo = archivo;
				 aux_ptrArchivo = archivo;
			 }

			 close(fd_archivo);
			 persistirArchivoPosta(archivo);
			 struct archivos* unArchivo = despersistirArchivoPosta(); //PARA PROBAR DESPERSISTIR
			 //PARA PROBAR DESPERSISTIR

			 loguearEspacioDisponible(espacioDisponibleFS());

		 }else{
			 reestablecerDistribucion(archivo);
			 printf("NO SE PUEDE DISTRIBUIR EL ARCHIVO. SE LLENARON LOS NODOS O BIEN SE CAYO UNO DE ELLOS\n");
			 loguearErrores("NO SE PUEDE DISTRIBUIR EL ARCHIVO. SE LLENARON LOS NODOS O BIEN SE CAYO UNO DE ELLOS");
		 }
	}else{
		printf("NO SE PUEDE DISTRIBUIR EL ARCHIVO. NO HAY ESPACIO DISPONIBLE EN LOS NODOS O BIEN NO HAY SUFICIENTES NODOS CONECTADOS.\n NOTA: SE REQUIERE UN MINIMO DE 3 NODOS\n");
	}

 }

 int sePuedeDistribuirElArchivo(int tamanioArchivo){
	 int cantNodosActivosYConLugar = cantidadDeNodosActivosYConLugar();
	 if((cantNodosActivosYConLugar>=3) && (nodosTienenLugarSuficiente(tamanioArchivo)) ){
		 return 1;
	 }else{
		 return 0;
	 }
 }

 int nodosTienenLugarSuficiente(int tamanioArchivo){
	 t_list* nodosAptos = list_filter(socketsConectados,nodoActivoYConLugar);
	 int cantNodos = list_size(nodosAptos);
	 int i=0;
	 long double lugarTotalDisponible = 0;
	 while(i<cantNodos){
		 struct nodos* unNodo = list_get(nodosAptos,i);
		 int bloquesLibres = cantBloquesLibres(unNodo->bloques, unNodo->cantidadDeBloques);
		 int lugarDisponible = bloquesLibres * 20971520;
		 lugarTotalDisponible=lugarTotalDisponible+(long double)lugarDisponible;
		// printf("lugar disponible1: %d\n", lugarTotalDisponible);
		 i++;
	 }

	 //printf("lugar disponible1: %d\n", lugarTotalDisponible);
	 long double tamanioPosta = (long double) 3*tamanioArchivo;
	 if(lugarTotalDisponible>=tamanioPosta){
		// printf("lugar disponible: %d\n", lugarTotalDisponible);
		 return 1;
	 }

	 return 0;
 }

 int cantBloquesLibres(t_bitarray* bloques, int cantBloques)
 {
	 int cantidadDeBloquesLibres = 0;
	 int i=0;
	 while(i< (cantBloques) ){
	 	if(!bitarray_test_bit(bloques, i)){
	 		cantidadDeBloquesLibres++;
	 	}
	 	i++;
	 }
	return cantidadDeBloquesLibres;
 }

 void crearDirectorios(char *path){
	 char ** arrayDeDirectorios = desglosarRuta(path);
	 int i=0;
	 int primeraVez=1;
	 char* ruta = (char *) malloc(sizeof(char)*200);
	 while(arrayDeDirectorios[i]!=NULL){

		 if(primeraVez){
			 if(yaExisteElDirectorio(arrayDeDirectorios[i],"raiz")){
				 printf("el directorio ya existe!\n");
			 }else{
				 crearDirectorio(arrayDeDirectorios[i],"raiz");
			 }
			 primeraVez=0;
		 }else{
			 string_append(&ruta,arrayDeDirectorios[i-1]);
			 string_append(&ruta,"/");
			 if(yaExisteElDirectorio(arrayDeDirectorios[i],ruta)){
				 printf("el directorio ya existe!\n");
			 }else{
				 crearDirectorio(arrayDeDirectorios[i],ruta);
			 }
		 }

		i++;
	 }
	 persistirDirectorios();
	 free(ruta);
 }

 int yaExisteElDirectorio(char* nombreDirectorio, char* rutaDelPadre){

	 int idPadre;
	 int esta=1;
	 if(strcmp(rutaDelPadre,"raiz")){
	 		 idPadre = obtenerIdDelUltimoDirectorioDe(rutaDelPadre);
	 }else{
	 		 idPadre = 0;
	 }

	 int i=0;
	 while(strcmp(directorio[i].nombre,nombreDirectorio)!=0 || directorio[i].dirPadre!=idPadre){
	 	 i++;
	 	 if(i>=1024){
	 		 esta = 0;
	 		 return esta;
	 	 }
	 }

	 return esta;
 }

 //Por ejemplo, home/utnso/hola.txt te devuelve home/utnso

 char* sacaElArchivoALaRuta(char *path){
	 int i=strlen(path);
	 while(*(path+(i-1))!='/'){
		 i--;
	 }
	 char* resultado = (char *) malloc(sizeof(char)*(i-1));
	 resultado = string_substring_until(path,i-1);

	 return resultado;

 }

 //OBTENER ID DEL ULTIMO DIRECTORIO DE UNA RUTA. Ejemplo: home/utnso/juan , obtiene el ID de juan.

 int obtenerIdDelUltimoDirectorioDe(char * path){
	 char ** arrayDeDirectorios = desglosarRuta(path);
	 int j=0;
	 int idDirectorio = 0;
	 while(arrayDeDirectorios[j]!=NULL){
		 verificarRuta(&idDirectorio,arrayDeDirectorios[j]);
		 j++;
	 }
	 return idDirectorio;
 }

 void verificarRuta(int* idDirectorio, char* nombreDirectorio){
	 int i=0;
	 while(strcmp(directorio[i].nombre,nombreDirectorio)!=0 || directorio[i].dirPadre!=*idDirectorio){
		 i++;
	 }
	 *idDirectorio = directorio[i].id;
 }

 //CREA DIRECTORIOS-------------------------------------------------

 void crearDirectorio(char *nombreDirectorio, char * rutaDelPadre){ // no esta contemplado DATOS invalidos
	 int indice = obtenerPosLibreDeTablaDeDirectorios(directorio);
	 directorio[indice].id = indice+1;
	 strcpy(directorio[indice].nombre, nombreDirectorio);
	 if(strcmp(rutaDelPadre,"raiz")){
		 directorio[indice].dirPadre = obtenerIdDelUltimoDirectorioDe(rutaDelPadre);
	 }else{
		 directorio[indice].dirPadre = 0;
	 }
	 mostrarDirectorios(); //PARA PROBAR
	 persistirDirectorios();
 }



 int obtenerPosLibreDeTablaDeDirectorios(struct directorios directorio[]){
	int i=0;
	while(directorio[i].id!=-1){
		i++;
	}
	return i;
 }


void inicializarTablaDeDirectorios(){ //Ver en que momento se necesitara inicializarla. EN principio al comienzo es necesario.
	int i;
	for(i=0;i<1023;i++){
		directorio[i].id=-1;
		directorio[i].dirPadre=-1;
	}
}

//PARA PROBAR FUNCIONES DE LA CONSOLA

void mostrarDirectorios(){
	int i=0;
	while(i<1023){
		if(directorio[i].id!=-1){
			if(directorio[i].id!= 0){
			printf("%d %s, %d\n", directorio[i].id ,directorio[i].nombre, directorio[i].dirPadre);
			}
		}
	i++;
}
}
//ELIMINAR UN DIRECTORIO---------------------------------------------------------------

void eliminarDirectorio(char *path){
	int idDirectorio = obtenerIdDelUltimoDirectorioDe(path);
	eliminarUnDirectorio(idDirectorio);
	persistirDirectorios();
	mostrarDirectorios(); //PARA PROBAR
}

void eliminarUnDirectorio(int idDirectorio){
	eliminarLosDirectoriosHijosDe(idDirectorio);
	//eliminar los archivos dentro del directorio
	directorio[idDirectorio-1].id = -1;
	directorio[idDirectorio-1].dirPadre = -1;
	persistirDirectorios();
}

void eliminarLosDirectoriosHijosDe(int idPadre){
	int i=0;
	for(i=0; i<1023; i++){
		if(directorio[i].dirPadre==idPadre){
			eliminarUnDirectorio(directorio[i].id);
		}
	}
}

//RENOMBRAR UN DIRECTORIO---------------------------------------------------------------

void renombrarDirectorio(char* nuevoNombre, char* path){
	int idDirectorio = obtenerIdDelUltimoDirectorioDe(path);
	renombrarUnDirectorio(nuevoNombre,idDirectorio);
	persistirDirectorios();
	mostrarDirectorios(); //PARA PROBAR
}

void renombrarUnDirectorio(char* nuevoNombre,int idDirectorio){
	strcpy(directorio[idDirectorio-1].nombre,nuevoNombre);
}

//MOVER UN DIRECTORIO-------------------------------------------------------------------

void moverDirectorioA(char* directorio, char* destino){
	int idDirectorio = obtenerIdDelUltimoDirectorioDe(directorio);
	int idDirectorioDestino = obtenerIdDelUltimoDirectorioDe(destino);
	moverUnDirectorioA(idDirectorioDestino,idDirectorio);
	persistirDirectorios();
	mostrarDirectorios(); //PARA PROBAR
}

void moverUnDirectorioA(int idPadreNuevo, int idDirectorio){
	directorio[idDirectorio-1].dirPadre = idPadreNuevo;
}

//RENOMBRAR UN ARCHIVO----------------------------------------------------------------------
void renombrarUnArchivo(char* rutaArchivo, char* nuevoNombre){
	char ** arrayDeDirectorios = desglosarRuta(rutaArchivo);
	int ultimaPos = obtenerUltimaPosicionDeUnArray(arrayDeDirectorios);
	char* nombreDelArchivo = arrayDeDirectorios[ultimaPos-1];
	char* path= sacaElArchivoALaRuta(rutaArchivo);
	struct archivos* archivo = encontrarAlArchivo(nombreDelArchivo, path);
	strcpy(archivo->nombre,nuevoNombre);
	//persistirArchivoPosta(archivo);

}

struct archivos* encontrarAlArchivo(char* nombreDelArchivo, char* path){
	struct archivos* auxptr = (struct archivos*) malloc(sizeof(struct archivos));
	auxptr = ptrArchivo;
	int idPadre = obtenerIdDelUltimoDirectorioDe(path);
	while(strcmp(auxptr->nombre,nombreDelArchivo)!=0 || auxptr->dirPadre != idPadre){
		auxptr = auxptr->archivo;
	}
	return auxptr;
}

//ELIMINAR UN ARCHIVO------------------------------------------------------------------------

void eliminarUnArchivo(char* rutaArchivo){  //DUDA: NO SE SABE CUANDO SE UTILIZA Y QUE CONSECUENCIAS TIENE
	char ** arrayDeDirectorios = desglosarRuta(rutaArchivo);
	int ultimaPos = obtenerUltimaPosicionDeUnArray(arrayDeDirectorios);
	char* nombreDelArchivo = arrayDeDirectorios[ultimaPos-1];
	char* path= sacaElArchivoALaRuta(rutaArchivo);
	struct archivos* archivo = encontrarAlArchivo(nombreDelArchivo, path);
	archivo->estado = 0; // suponiendo a 0 como eliminado.
	//persistirArchivoPosta(ptrArchivo);
}
//MOVER UN ARCHIVO------------------------------------------------------------------------

void moverUnArchivo(char* rutaArchivo, char* rutaDestino){  //DUDA: NO SE SABE CUANDO SE UTILIZA Y QUE CONSECUENCIAS TIENE
	char ** arrayDeDirectorios = desglosarRuta(rutaArchivo);
	int ultimaPos = obtenerUltimaPosicionDeUnArray(arrayDeDirectorios);
	char* nombreDelArchivo = arrayDeDirectorios[ultimaPos-1];
	char* path= sacaElArchivoALaRuta(rutaArchivo);
	struct archivos* archivo = encontrarAlArchivo(nombreDelArchivo, path);
	int idPadre = obtenerIdDelUltimoDirectorioDe(rutaDestino);
	archivo->dirPadre = idPadre;
	//persistirArchivoPosta(ptrArchivo);
}


//CREA EL BITARRAY EN UNA ESTRUCTURA NODO

unsigned int tamanioBloques = 20971520;

void cargarDatosDeBloques(struct nodos* nodo,int tamanioEspacioAlmacenamiento){
	int cantidadBloques = tamanioEspacioAlmacenamiento / tamanioBloques;
	int cantidadBytesArray = (cantidadBloques/8)+1;
	char* datos = malloc(cantidadBytesArray * sizeof(char));
	int i;

	for(i=0;i<cantidadBytesArray;i++){
		datos[i] = 0;
	}

	nodo->cantidadDeBloques = cantidadBloques;
	nodo->bloques = bitarray_create(datos,cantidadBytesArray);
}


//FORMATEA EL MDFS

void formatearMDFS(){
	char nombreArchivos[40] = "funciona.dat";
	char nombreDirectorios[40]= "Directorios.dat";
	remove(nombreArchivos);
	remove(nombreDirectorios);
	primerArchivo = 1;

	int cantNodos = list_size(socketsConectados);
	int i=0;
	while(i<cantNodos){
		struct nodos* unNodo = list_get(socketsConectados,i);
		int cantBloques = unNodo->cantidadDeBloques;
		int j=0;
		while(j<cantBloques){
			bitarray_clean_bit(unNodo->bloques,j);
			j++;
		}

		i++;
	}

	inicializarTablaDeDirectorios(); //Inicializa la tabla de directorios
	//son 3 remove, uno por cada archivo que existe
}

void despersistirArchivos(){
	despersistirDirectorios();
	ptrArchivo = despersistirArchivoPosta();
	//falta determinar aux_ptrArchivo
	primerArchivo=0;
}

void verBloques(){
	char *ruta = malloc(sizeof(char)*40);
	printf("Ruta: ");
	scanf("%s",ruta);
	verBloquesQueComponenAUnArchivo(ruta);
	free(ruta);
}

void verBloquesQueComponenAUnArchivo(char *ruta){
	char** arrayDeDirectorios = desglosarRuta(ruta);
	int ultimaPosicion = obtenerUltimaPosicionDeUnArray(arrayDeDirectorios);
	char* nombreArchivo = arrayDeDirectorios[ultimaPosicion-1];
	struct archivos* archivo = despersistirArchivoPosta();
	struct archivos* ptr = archivo;
	while(strcmp((*ptr).nombre,nombreArchivo) != 0){
		ptr = (*ptr).archivo;
	}
	//imprimimos las porquerias de archivo
		printf("Estos son los datos del archivo: Nombre: %s\nTamanio: %d\nEstado: %d\nPadre: %d\nNro de bloques: %d\n",(*ptr).nombre,(*ptr).tamanio,(*ptr).estado,(*ptr).dirPadre,(*ptr).bloquesDeCadaArchivo);
		//imprimo sus bloques
		int h=0;
		if((*ptr).bloquesDeCadaArchivo == 0 || (*ptr).bloque == NULL){

		}
		struct bloques bloqueImprimir = *((*ptr).bloque);
		while(h < (*ptr).bloquesDeCadaArchivo){
		printf("estos son los datos del bloque: Id: %d\nCopias: %d\n",bloqueImprimir.numBloque,bloqueImprimir.copiasDeCadaBloque);
			int y = 0;
			//imprimo lo de cada copia
			struct copias copiaImprimir = *(bloqueImprimir.copia);
			int q = bloqueImprimir.copiasDeCadaBloque;
			for(;y < q; y++){
				printf("Nro: %d\n",(copiaImprimir).nroCopia);
				//imprimo su nodo
				struct nodos nodoImprimir = *((copiaImprimir).unNodo->unNodo);
				printf("Esta en el nodo: %d\nEn su bloque numero: %d\n",nodoImprimir.idNodo,copiaImprimir.unNodo->nroBloqueNodo);
				if(copiaImprimir.copia == NULL){
				}
				else copiaImprimir = *copiaImprimir.copia;
			}
			if(bloqueImprimir.bloque != NULL){
		bloqueImprimir = *bloqueImprimir.bloque;}

		h++;
		}
}

void borrarBloques(){
	char *ruta = malloc(sizeof(char)*40);
	int nroBloque[20];
	printf("Ruta: ");
	scanf("%s",ruta);
	printf("NroBloque: ");
	int j=0;
	scanf("%d",&nroBloque[j]);
	while(nroBloque[j]!=-1){
		j++;
		scanf("%d",&nroBloque[j]);
	}
	borrarBloquesQueComponenAUnArchivo(ruta, nroBloque);
	free(ruta);
}

void copiarBloques(){
	char *ruta = malloc(sizeof(char)*40);
	int nroBloque[20];
	printf("Ruta: ");
	scanf("%s",ruta);
	printf("NroBloque: ");
	int j=0;
	scanf("%d",&nroBloque[j]);
	while(nroBloque[j]!=-1){
		j++;
		scanf("%d",&nroBloque[j]);
	}
	copiarBloquesQueComponenAUnArchivo(ruta, nroBloque);
	free(ruta);

}

struct archivos* despersistirArchivoPosta(){
	FILE* archivoAbierto = fopen("funciona.dat","rb");

	//leo el archivo
	struct archivos* primerArchivo = malloc (sizeof(struct archivos));
	struct archivos archivo[50];
	int indiceArchivo = 0;
	int nroNodo, nroBloqueEnNodo;
	primerArchivo = &archivo[indiceArchivo];
	fseek(archivoAbierto,0,SEEK_SET);
	while(fread(&archivo[indiceArchivo],sizeof(struct archivos),1,archivoAbierto)!= 0){
		if(indiceArchivo != 0){
			archivo[indiceArchivo -1].archivo = &archivo[indiceArchivo];
			}
	int tamanioNombre;
	fread(&tamanioNombre,sizeof(int),1,archivoAbierto);
	char* nombreArchivo = malloc(tamanioNombre);
	fread(nombreArchivo,tamanioNombre,1,archivoAbierto);

	strcpy(archivo[indiceArchivo].nombre,nombreArchivo);

	//leo los bloques

	struct bloques* bloque = malloc(sizeof(struct bloques));
	struct bloques* aux2;
	fread(bloque,sizeof(struct bloques),1,archivoAbierto);
	//busco copias para ese bloque
	struct copias* copia = malloc(sizeof(struct copias));
	struct copias* copia2 = malloc(sizeof(struct copias));
	struct copias* copia3 = malloc(sizeof(struct copias));
	fread(copia,sizeof(struct copias),1,archivoAbierto);
	bloque->copia = copia;


	/*agregado*/
	struct nodoPos* unNodo = malloc(sizeof(struct nodoPos));
	int nroNodo, nroBloqueEnNodo;
	fread(&nroNodo, sizeof(int),1,archivoAbierto);
	fread(&nroBloqueEnNodo, sizeof(int),1,archivoAbierto);
	unNodo->nroBloqueNodo = nroBloqueEnNodo;
	unNodo->idNodo = nroNodo;
	copia->unNodo = unNodo;
	/*fin de agregado*/

	fread(copia2,sizeof(struct copias),1,archivoAbierto);
	copia->copia = copia2;

	/*agregado*/
	struct nodoPos* unNodo2 = malloc(sizeof(struct nodoPos));
	fread(&nroNodo, sizeof(int),1,archivoAbierto);
	fread(&nroBloqueEnNodo, sizeof(int),1,archivoAbierto);
	unNodo2->nroBloqueNodo = nroBloqueEnNodo;
	unNodo2->idNodo = nroNodo;
	copia2->unNodo = unNodo2;
	/*fin de agregado*/

	fread(copia3,sizeof(struct copias),1,archivoAbierto);
	copia2->copia = copia3;
	copia3->copia = NULL;

	/*agregado*/
	struct nodoPos* unNodo3 = malloc(sizeof(struct nodoPos));
	fread(&nroNodo, sizeof(int),1,archivoAbierto);
	fread(&nroBloqueEnNodo, sizeof(int),1,archivoAbierto);
	unNodo3->nroBloqueNodo = nroBloqueEnNodo;
	unNodo3->idNodo = nroNodo;
	copia3->unNodo = unNodo3;
	/*fin de agregado*/

	//Ya encontro todas entonces guardo la primer referencia
	archivo[indiceArchivo].bloque = bloque;

	//sigo buscando los bloques
	aux2 = bloque;
	int k = 1;

	//este es el for que busca todos los bloques restantes
	for(; k < archivo[indiceArchivo].bloquesDeCadaArchivo; k++){
		struct bloques* auxBloqueSig= malloc(sizeof(struct bloques));
		fread(auxBloqueSig,sizeof(struct bloques),1,archivoAbierto);
		//busco copias para ese bloque
		struct copias* unaCopia = malloc(sizeof(struct copias));
		struct copias* unaCopia2 = malloc(sizeof(struct copias));
		struct copias* unaCopia3 = malloc(sizeof(struct copias));

		fread(unaCopia,sizeof(struct copias),1,archivoAbierto);
		auxBloqueSig->copia = unaCopia;
		unaCopia->copia = unaCopia2;

		/*agregado*/
		struct nodoPos* unNodo4 = malloc(sizeof(struct nodoPos));
		fread(&nroNodo, sizeof(int),1,archivoAbierto);
		fread(&nroBloqueEnNodo, sizeof(int),1,archivoAbierto);
		unNodo4->nroBloqueNodo = nroBloqueEnNodo;
		unNodo4->idNodo = nroNodo;
		unaCopia->unNodo = unNodo4;
		/*fin de agregado*/

		fread(unaCopia2,sizeof(struct copias),1,archivoAbierto);
		unaCopia2->copia = unaCopia3;

		/*agregado*/
		struct nodoPos* unNodo5 = malloc(sizeof(struct nodoPos));
		fread(&nroNodo, sizeof(int),1,archivoAbierto);
		fread(&nroBloqueEnNodo, sizeof(int),1,archivoAbierto);
		unNodo5->nroBloqueNodo = nroBloqueEnNodo;
		unNodo5->idNodo = nroNodo;
		unaCopia2->unNodo = unNodo5;
		/*fin de agregado*/

		fread(unaCopia3,sizeof(struct copias),1,archivoAbierto);
		unaCopia3->copia = NULL;

		/*agregado*/
		struct nodoPos* unNodo6 = malloc(sizeof(struct nodoPos));
		fread(&nroNodo, sizeof(int),1,archivoAbierto);
		fread(&nroBloqueEnNodo, sizeof(int),1,archivoAbierto);
		unNodo6->nroBloqueNodo = nroBloqueEnNodo;
		unNodo6->idNodo = nroNodo;
		unaCopia3->unNodo = unNodo6;
		/*fin de agregado*/

		//Encontro todas las copias y sigue con el proximo bloque
		aux2->bloque = auxBloqueSig;
		aux2 = auxBloqueSig;

	}
	//termino el for de los bloques
	aux2->bloque = NULL;
	indiceArchivo++;
}
	fclose(archivoAbierto);

	return primerArchivo;
}


void borrarBloquesQueComponenAUnArchivo(char* ruta,int nroBloques[]){
	char** arrayDeDirectorios = desglosarRuta(ruta);
	int ultimaPosicion = obtenerUltimaPosicionDeUnArray(arrayDeDirectorios);
	char* nombreArchivo = arrayDeDirectorios[ultimaPosicion-1];
	struct archivos* archivo = despersistirArchivoPosta();
	struct archivos* ptr = archivo;
	while(strcmp((*ptr).nombre,nombreArchivo) != 0){
		ptr = (*ptr).archivo;
		}
	int i = 0;
	int cantBloquesDeArchivo = (*ptr).bloquesDeCadaArchivo;
	struct bloques* ptrBloqueAnterior = (*ptr).bloque;
	struct bloques* ptrBloqueSiguiente = (*ptrBloqueAnterior).bloque;
	int primerosBorrados = 0;

	while(esta((*ptrBloqueAnterior).numBloque,nroBloques)){  //Mientras que haya que borrar el primer bloque
		(*ptr).bloque = ptrBloqueSiguiente;
		primerosBorrados++;
		archivo->bloquesDeCadaArchivo = archivo->bloquesDeCadaArchivo -1;
		if(ptrBloqueSiguiente == NULL)break;
		ptrBloqueAnterior = ptrBloqueSiguiente;
		ptrBloqueSiguiente = (*ptrBloqueSiguiente).bloque;

	}
	for(;i < (cantBloquesDeArchivo-primerosBorrados);i++){
		if(esta((*ptrBloqueSiguiente).numBloque,nroBloques)){
			(*ptrBloqueAnterior).bloque = (*ptrBloqueSiguiente).bloque;
			archivo->bloquesDeCadaArchivo = archivo->bloquesDeCadaArchivo -1;
		}
		ptrBloqueAnterior = ptrBloqueSiguiente;
		ptrBloqueSiguiente = (*ptrBloqueSiguiente).bloque;
	}
	char nombre[40] = "funciona.dat";
	FILE* fichero = fopen(nombre,"r");
		if (fichero != NULL){
			fclose(fichero);
			remove(nombre);
		}
	persistirArchivoPosta(archivo);
}

int esta(int bloque, int bloquesABorrar[]){
	int i=0;
	while(bloquesABorrar[i]!=-1){
		if(bloque == bloquesABorrar[i]){
			return 1;
		}
		i++;
	}
	return 0;
}

int persistirArchivoPosta(struct archivos* archivo){
	char *persistenciaArchivos = "funciona.dat";
	FILE* archivoAbierto = fopen(persistenciaArchivos,"ab");
	persistirArchivo(archivoAbierto, archivo);
	fclose(archivoAbierto);
	return 0;
}

void persistirArchivo(FILE *archivoAbierto,struct archivos* archivo){

	fwrite(archivo,sizeof(struct archivos), 1,archivoAbierto);
	int tamanioNombre = strlen(archivo->nombre)+1;
	char* nombreArchivo = malloc(tamanioNombre);
	strcpy(nombreArchivo,archivo->nombre);
	fwrite(&tamanioNombre,sizeof(int),1,archivoAbierto);
	fwrite(nombreArchivo,tamanioNombre,1,archivoAbierto);
	if(archivo->bloque == NULL || archivo->bloquesDeCadaArchivo == 0){

	}
	else persistirBloques(archivoAbierto, archivo->bloque);
}

void persistirBloques(FILE *archivoAbierto, struct bloques* bloque){
	struct bloques* aux = malloc(sizeof(struct bloques));
	struct bloques* bloqueAux = malloc(sizeof(struct bloques));
	bloqueAux = bloque;
	aux = bloque->bloque;
	while(aux != NULL){
		fwrite(bloqueAux,sizeof(struct bloques), 1,archivoAbierto);
		persistirCopias(archivoAbierto, bloqueAux->copia);
		bloqueAux = bloqueAux->bloque;
		aux = bloqueAux->bloque;
	}
	if(aux == NULL){
		fwrite(bloqueAux,sizeof(struct bloques), 1,archivoAbierto);
		persistirCopias(archivoAbierto, bloqueAux->copia);
	}
}

void persistirCopias(FILE *archivoAbierto,struct copias* copia){
	struct copias* aux = malloc(sizeof(struct copias));
	aux = copia;
	while(aux != NULL){
		fwrite(aux,sizeof(struct copias), 1,archivoAbierto);
		persistirNodoPos(archivoAbierto, aux->unNodo);
		aux = aux->copia;
	}

}

void persistirNodoPos(FILE *archivoAbierto, struct nodoPos* unNodoP){
	int nroNodo = unNodoP->idNodo;
	int nroBloqueNodo = unNodoP->nroBloqueNodo;

	fwrite(&nroNodo,sizeof(int),1,archivoAbierto);
	fwrite(&nroBloqueNodo,sizeof(int),1,archivoAbierto);
}

void copiarBloquesQueComponenAUnArchivo(char*ruta,int nroBloques[]){
	char** arrayDeDirectorios = desglosarRuta(ruta);
	int ultimaPosicion = obtenerUltimaPosicionDeUnArray(arrayDeDirectorios);
	char* nombreArchivo = arrayDeDirectorios[ultimaPosicion-1];
	struct archivos* archivo = despersistirArchivoPosta();
	struct archivos* ptr = archivo;
	while(strcmp((*ptr).nombre,nombreArchivo) != 0){
		ptr = (*ptr).archivo;
		}
	int i = 0;
	int cantBloquesDeArchivo = archivo->bloquesDeCadaArchivo;
	struct bloques* ptrBloqueAnterior = (*ptr).bloque;
	struct bloques* ptrBloqueSiguiente = (*ptrBloqueAnterior).bloque;


	for(;i < (cantBloquesDeArchivo);i++){
			if(esta((*ptrBloqueAnterior).numBloque,nroBloques)){
				struct bloques bloqueNuevo = (*ptrBloqueAnterior);
				(*ptrBloqueAnterior).bloque = &bloqueNuevo;
				archivo->bloquesDeCadaArchivo = archivo->bloquesDeCadaArchivo +1;
				ptrBloqueAnterior = (*ptrBloqueAnterior).bloque;
				(bloqueNuevo).bloque = ptrBloqueSiguiente;
			}
			else {ptrBloqueAnterior = ptrBloqueSiguiente;
			if(ptrBloqueSiguiente == NULL){
			}
			else ptrBloqueSiguiente = (*ptrBloqueSiguiente).bloque;
			}
		}

	char nombre[40] = "funciona.dat";
	remove(nombre);
	persistirArchivoPosta(archivo);
}

void persistirDirectorios(){
	FILE* archivoAbierto = fopen("Directorios.dat","wb");
	int i=0;
	for(;i<1024;i++){
		fwrite(&directorio[i],sizeof(struct directorios),1,archivoAbierto);
	}
}

void despersistirDirectorios(){
	FILE* archivoAbierto = fopen("Directorios.dat","rb");
	int i=0;
	for(;i<1024;i++){
		fread(&directorio[i],sizeof(struct directorios),1,archivoAbierto);
	}
	fclose(archivoAbierto);
}


//MANDAR AL FS LA INFORMACION QUE NECESITA

void martaPideBloquesDeArchivos(int nroSocket){
	listaTamanioEstructuras = list_create();
	uint32_t tamanioPaquete;
	recv(nroSocket, &tamanioPaquete, sizeof(tamanioPaquete),0);
	char* mensajeSerializado = malloc(tamanioPaquete);
	recibir(nroSocket,mensajeSerializado, tamanioPaquete);
	t_list* urls = deserializarPedidoDeMarta(mensajeSerializado);

	/*char idProceso = 'F';
	char idOperacion = 'R'; // R es rta de P
	enviar(nroSocket,&idProceso, sizeof(char));
	enviar(nroSocket,&idOperacion, sizeof(char));
	int tamanio = 11;
	int cantEstructuras = 1;
	enviar(nroSocket,&tamanio, sizeof(int));
	enviar(nroSocket,&cantEstructuras, sizeof(int));
*/
	t_list* urlsDisponibles = list_filter(urls,tieneElArchivoDisponible);
	int cantUrls = list_size(urls);
	int cantUrlsDisponibles = list_size(urlsDisponibles);

	if(cantUrlsDisponibles>0){

	if(cantUrlsDisponibles!=cantUrls){
		printf("No todos los archivos solicitados se encuentran completos\n");
		loguearComentarios("No todos los archivos solicitados se encuentran completos");
	}

	//despersistirArchivos();
	t_list* estructurasArchivos = list_map(urlsDisponibles,obtenerEstructuraDeArchivo);

	char idProceso = 'F';
	char idOperacion = 'R'; // R es rta de P
	enviar(nroSocket,&idProceso, sizeof(char));
	enviar(nroSocket,&idOperacion, sizeof(char));
	int cantEstructuras = list_size(estructurasArchivos);
	enviar(nroSocket,&cantEstructuras,sizeof(int));
	int i;
	for(i=0;  i < cantEstructuras ; i++){
		int* tamanioDeEstruc = list_get(listaTamanioEstructuras,i);
		enviar(nroSocket,tamanioDeEstruc,sizeof(int));
		char* respuestaSerializada = list_get(estructurasArchivos, i);
		enviar(nroSocket,respuestaSerializada, *tamanioDeEstruc);
	}

	}else{
		//notificar a marta que no es posible ejecutar ese job por falta de archivos.
		char idProceso = 'F';
		char idOperacion = 'M'; // M significa que no fue posible cumplir el pedido de marta de ninguna url y se debe abortar el job.
		enviar(nroSocket,&idProceso, sizeof(char));
		enviar(nroSocket,&idOperacion, sizeof(char));
		printf("No es posible cumplir el pedido de Marta, ya que no se cuenta con los archivos completos de ninguna de las urls pedidas\n");
		loguearErrores("No es posible cumplir el pedido de Marta, ya que no se cuenta con los archivos completos de ninguna de las urls pedidas");
	}
}


void* obtenerEstructuraDeArchivo(void* element){

	char* rutaArchivo = (char*)element;
	int fd_archivo = open(rutaArchivo,O_RDONLY);
	int tamanioArchivo = tamanio_archivo(fd_archivo);

	char ** arrayDeDirectorios = desglosarRuta(rutaArchivo);
	int ultimaPos = obtenerUltimaPosicionDeUnArray(arrayDeDirectorios);
	char* nombreDelArchivo = arrayDeDirectorios[ultimaPos-1];
	char* path= sacaElArchivoALaRuta(rutaArchivo);
	struct archivos* archivo = encontrarAlArchivo(nombreDelArchivo, path);

	//ARMADO DE LA INFORMACION QUE NECESITA MARTA (POR ARCHIVO)
	char* ipNodo = malloc(15);
	char* puerto = malloc(10);
	int tamanioIp = 15;
	int tamanioPuerto = 10;

	int tamanioRuta = (strlen(rutaArchivo) + 1) * sizeof(char);
	int tamanioCantBloques = sizeof(int);
	int tamanioTamanioArch = sizeof(int);
	int tamanioCantCopias = sizeof(int);
	int tamanioNroBloqueEnNodo = sizeof(int);
	int tamanioIdNodo = sizeof(int);

	int cantCopiasDisponiblesArch = cantCopiasDisponiblesDeUnArchivo(archivo);

	int size_archivo = tamanioCantBloques;
	int size_whileBloque = tamanioCantCopias*(archivo->bloquesDeCadaArchivo);
	int size_whileCopia = (tamanioIdNodo + sizeof(tamanioIp) + sizeof(tamanioPuerto) +tamanioIp + tamanioPuerto + tamanioNroBloqueEnNodo + sizeof(tamanioRuta) + tamanioRuta + tamanioTamanioArch)*cantCopiasDisponiblesArch;

	int size_total = size_archivo + size_whileBloque + size_whileCopia;

	char* infoPorArchivo = malloc(size_total + 1);

	int tamanioDeEstructura = size_total + 1;
	list_add(listaTamanioEstructuras,&tamanioDeEstructura); //lista tamanios Estructuras Global

	int size;
	int offset = 0;

	//size = sizeof(size_total);
	//memcpy(infoPorArchivo + offset,&size_total, size);
	//offset+= size;

	int cantidadDeBloques = archivo->bloquesDeCadaArchivo;

	size = tamanioCantBloques;
	memcpy(infoPorArchivo + offset,&(cantidadDeBloques),size);
	offset+= size;

	struct bloques* ptrBloque = malloc(sizeof(struct bloques));
	ptrBloque = archivo->bloque;

	while(ptrBloque != NULL){


		struct copias* ptrCopia = malloc(sizeof(struct copias));
		ptrCopia = ptrBloque->copia;

		int cantCopias = ptrBloque->copiasDeCadaBloque;
		int cantCopiasDisponibles = cantCopiasDisponiblesDeUnBloque(ptrBloque);

		size = tamanioCantCopias;
		memcpy(infoPorArchivo + offset,&(cantCopiasDisponibles),size);
		offset+= size;


		//t_list* nodos = list_create();
		int i = 0;
		while(i<(ptrBloque->copiasDeCadaBloque)){

			int idNodo = ptrCopia->unNodo->idNodo;
			struct nodos* unNodo = obtenerNodo(idNodo);

			if(unNodo->estado==1){

			//ARMADO INFO DE NODO

			// NOTA: A MARTA SOLO LE MANDO UNA UBICACION DEL BLOQUE EN UN NODO.
			//       SI ESTAN LAS 3 COPIAS EN UN NODO, SOLO SE MANDA UNA DE ELLAS.
			//if(seEncuentraElBloqueEnElNodo(ptrCopia->unNodo->idNodo,nodos)!=1){

				//falta agregar puerto e ip del nodo


				strcpy(ipNodo,unNodo->ipNodo);
				strcpy(puerto,unNodo->puerto);

				int tamanioIp = strlen(ipNodo) + 1;
				int tamanioPuerto = strlen(puerto) + 1;

				size = sizeof(int);
				memcpy(infoPorArchivo + offset,&(idNodo),size);
				offset+= size;


				size = sizeof(int);
				memcpy(infoPorArchivo + offset,&(tamanioIp),size);
				offset+= size;

				size = tamanioIp;
				memcpy(infoPorArchivo + offset,ipNodo,size);
				offset+= size;

				size = sizeof(int);
				memcpy(infoPorArchivo + offset,&(tamanioPuerto),size);
				offset+= size;

				size = tamanioPuerto;
				memcpy(infoPorArchivo + offset,puerto,size);
				offset+= size;


				//ARMADO DE POSICION EN NODO (DE UNA COPIA)

				int nroBloqueNodo = ptrCopia->unNodo->nroBloqueNodo;

				size = sizeof(int);
				memcpy(infoPorArchivo + offset,&(nroBloqueNodo),size);
				offset+= size;


				/*//registro que el bloque ya se encuentra en un Nodo determinado.
				int idNodo = ptrCopia->unNodo->idNodo;
				int* id = malloc(sizeof(int));
				*id = idNodo;
				list_add(nodos,id);
				*/

				//ARMADO DE ESTRUCTURA ARCHIVO (DE UNA COPIA)

				size = sizeof(tamanioRuta);
				memcpy(infoPorArchivo + offset,&tamanioRuta, size);
				offset+= size;


				size = tamanioRuta;
				memcpy(infoPorArchivo + offset,rutaArchivo,size);
				offset+= size;


				size = sizeof(int);
				memcpy(infoPorArchivo + offset,&tamanioArchivo,size);
				offset+= size;


			}
			if(ptrCopia->copia != NULL){
				ptrCopia = ptrCopia->copia;
			}
			i++;
		}
		//list_clean(nodos);


		ptrBloque = ptrBloque->bloque;
	}
	//enviar(7,infoPorArchivo, 800);

	return infoPorArchivo;
}

int cantCopiasDisponiblesDeUnArchivo(struct archivos* unArchivo){
	struct bloques* ptrBloque = malloc(sizeof(struct bloques));
	ptrBloque = unArchivo->bloque;
	int cantBloques = unArchivo->bloquesDeCadaArchivo;
	int j=0;
	int cantCopiasDisponibles = 0;
	while(j<cantBloques){
		cantCopiasDisponibles+= cantCopiasDisponiblesDeUnBloque(ptrBloque);
		if(ptrBloque->bloque != NULL){
			ptrBloque = ptrBloque->bloque;
		}
		j++;
	}
	return cantCopiasDisponibles;
}

int cantCopiasDisponiblesDeUnBloque(struct bloques* unBloque){
	int cantCopias = unBloque->copiasDeCadaBloque;
	int i=0;
	int cantCopiasDisponibles = 0;
	struct copias* ptrCopia = malloc(sizeof(struct copias));
	ptrCopia = unBloque->copia;
	while(i<cantCopias){
		int idNodo = ptrCopia->unNodo->idNodo;
		struct nodos* unNodo = obtenerNodo(idNodo);

		if(unNodo->estado==1){
			cantCopiasDisponibles++;
		}

		if(ptrCopia->copia != NULL){
			ptrCopia = ptrCopia->copia;
		}
		i++;
	}
	return cantCopiasDisponibles;
}

bool tieneElArchivoDisponible(void* elemento){
	char* rutaArchivo = (char*)elemento;
	char ** arrayDeDirectorios = desglosarRuta(rutaArchivo);
	int ultimaPos = obtenerUltimaPosicionDeUnArray(arrayDeDirectorios);
	char* nombreDelArchivo = arrayDeDirectorios[ultimaPos-1];
	char* path= sacaElArchivoALaRuta(rutaArchivo);
	struct archivos* archivo = encontrarAlArchivo(nombreDelArchivo, path);

	return estaElArchivoCompleto(archivo);

}

int estaElArchivoCompleto(struct archivos* archivo){
	int cantBloques = archivo->bloquesDeCadaArchivo;
	int i=0;
	int bloquesDisponibles=0;
	struct bloques* unBloque = archivo->bloque;
	while(i<cantBloques){
		int cantCopias = unBloque->copiasDeCadaBloque;
		int j=0;
		struct copias* unaCopia = unBloque->copia;
		int hayCopia = 0;
		while(j<cantCopias){
			int idNodo = unaCopia->unNodo->idNodo;
			struct nodos* unNodo = obtenerNodo(idNodo);

			if(unNodo->estado==1 && hayCopia !=1){
				hayCopia = 1;
				bloquesDisponibles++;
			}

			unaCopia = unaCopia->copia;
			j++;
		}
		unBloque = unBloque->bloque;
		i++;
	}

	if(bloquesDisponibles == cantBloques){
		return 1;
	}
	return 0;
}

struct nodos* obtenerNodo(int idNodo){
	int cantidadNodos = list_size(socketsConectados);
	int i=0;
	int esta=0;
	while(i<cantidadNodos && esta!=1){
		struct nodos* unNodo = list_get(socketsConectados,i);
		if(unNodo->idNodo == idNodo){
			esta=1;
		}
		i++;
	}
	return list_get(socketsConectados,i-1);
}


/*int seEncuentraElBloqueEnElNodo(int elemento, t_list* lista){
	int esta = -1;
	int cantElementos = list_size(lista);
	printf("cant elementos: %d y elemento: %d \n", cantElementos, elemento);
	int i=0;
	while(i<cantElementos && esta!=1){
		int* unElemento = (int*) list_get(lista,i);
		printf("una vueltita\n");
		if(elemento == *unElemento){
			esta=1;
		}

		i++;
	}
	printf("resultado de si esta dicho nodo: %d\n", esta);
	return esta;
}*/


t_list* deserializarPedidoDeMarta(char *mensajeSerializado){
	t_list* urls = list_create();
	list_clean(urls);
	uint32_t cantidadUrl;
	uint32_t tamanioUrl;

	int offset = 0;
	memcpy(&cantidadUrl,mensajeSerializado,sizeof(cantidadUrl));
	offset+=sizeof(cantidadUrl);


	int i;
	for(i=0; i<cantidadUrl; i++){
		memcpy(&tamanioUrl,mensajeSerializado + offset,sizeof(tamanioUrl));
		offset+=sizeof(tamanioUrl);


		char* unaUrl = malloc(tamanioUrl);

		memcpy(unaUrl, mensajeSerializado + offset,tamanioUrl);
		offset+=tamanioUrl;

		list_add(urls, unaUrl);

	}

	return urls;
}

int obtenerMD5ArchivoMDFS(char* rutaMDFS,char** md5ArchivoMDFS){
	pthread_mutex_t mutex;
	if (pthread_mutex_init(&mutex, NULL) != 0)
		{
		        printf("\n mutex init failed\n");
		}


	md5_state_t unMD5;
	md5_byte_t codigoMD5[16];
	int padre;
	int bloque;
	char* datosBloque;
	char* nombre;

	char ** arrayDeDirectorios = desglosarRuta(rutaMDFS);
	int ultimaPos = obtenerUltimaPosicionDeUnArray(arrayDeDirectorios);
	nombre = arrayDeDirectorios[ultimaPos - 1];

	if((padre = obtenerIdDelUltimoDirectorioDe(rutaMDFS)) == -1) {
		printf("Error. El archivo no existe en el MDFS.\n");
		return 1;
	}


	struct archivos* archivo = encontrarAlArchivo(nombre, rutaMDFS);
	struct bloques* bloqueNodo = archivo->bloque;
	*md5ArchivoMDFS = malloc(17);
	md5_init(&unMD5);

	pthread_mutex_lock(&mutex);

	for(bloque=0;bloque<(*archivo).bloquesDeCadaArchivo;bloque++){
	//	pedirBloque((*bloqueNodo).numBloque,&datosBloque);
		md5_append(&unMD5, ((md5_byte_t*) datosBloque), 20971520);
		free(datosBloque);
		bloqueNodo = (*bloqueNodo).bloque;
	}
	pthread_mutex_unlock(&mutex);

	md5_finish(&unMD5, codigoMD5); strcpy(*md5ArchivoMDFS,(char*)codigoMD5);


return 0;
}

void martaPideQueGuardeArchivoFinal(int socketQueEscribe){
	uint32_t idNodo;
	recibir(socketQueEscribe,&idNodo,sizeof(uint32_t));
	uint32_t tamanioRuta;
	recibir(socketQueEscribe,&tamanioRuta,sizeof(uint32_t));
	char* ruta = malloc(tamanioRuta);
	recibir(socketQueEscribe,ruta,tamanioRuta);
	uint32_t tamanioNombre;
	recibir(socketQueEscribe,&tamanioNombre,sizeof(uint32_t));
	char* nombre = malloc(tamanioNombre);
	recibir(socketQueEscribe,nombre,tamanioNombre);

	char idProceso = 'F';
	char idOperacion = 'Z';

	struct nodos* nodo = obtenerNodo(idNodo);
	int socket_Nodo = nodo->nroSocket;
	enviar(socket_Nodo,&idProceso,sizeof(char));
	enviar(socket_Nodo,&idOperacion,sizeof(char));
	enviar(socket_Nodo,&tamanioRuta,sizeof(uint32_t));
	enviar(socket_Nodo,ruta,tamanioRuta);
	enviar(socket_Nodo,&tamanioNombre,sizeof(uint32_t));
	enviar(socket_Nodo,nombre,tamanioNombre);
}

void pedirArchivoAlNodo(int socket, char* nombre,char* ruta){
	char idProceso = 'N';
	char idOperacion = 'A';
	uint32_t tamanioNombre = strlen(nombre)+1;

	enviar(socket,&idProceso,sizeof(char));
	enviar(socket,&idOperacion,sizeof(char));
	enviar(socket,&tamanioNombre,sizeof(uint32_t));
	enviar(socket,nombre,tamanioNombre);

	recibirArchivoDelNodo(socket,ruta);

}

void recibirArchivoDelNodo(int socketQueEscribe,char* ruta){
	uint32_t tamanioArchivo;
	recibir(socketQueEscribe,&tamanioArchivo,sizeof(uint32_t));
	char* mapeo;
	recibir(socketQueEscribe,mapeo,tamanioArchivo);


	FILE* archivo = fopen(ruta,'w');

	if(fprintf(archivo,mapeo)!=-1){
		loguearComentarios("Lectura de Bloque");
	}



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
           printf("Envio: %d\n",n);
           if (n == -1) { break; }
           total += n;
           bytesleft -= n;
       }
       printf("Total enviado: %d\n",total);
       return n==-1?-1:0; // devuelve -1 si hay fallo, 0 en otro caso
   }

int enviar2(int s, char *buf, int len)
   {
       int total = 0;        // cuántos bytes hemos enviado
       int bytesleft = len; // cuántos se han quedado pendientes
       int n;
       while(total < len) {
           n = send(s, buf+total, bytesleft, 0);
           printf("\033[30m Envio: %d\n",n);
           if (n == -1 || n!=20971532) { return -1; }
           total += n;
           bytesleft -= n;
       }
       printf("Total enviado: %d",total);
       printf("\033[0m \n");
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
       printf("Total recibido: %d\n",total);
       return n==-1?-1:0; // devuelve -1 si hay fallo, 0 en otro caso
   }


//----------------------Formatear Archivo de Log----------



void formatearArchivoDeLog(){

	remove("LogFileSystem");


}




//--------------------------------Archivo de Log------------------------------------

void loguearCopiasEnviadas(int numBloque,int nroCopia,int idNodo, int nroBloqueNodo){

	t_log_level nivel;
				t_log* archivoDeLog;
				nivel = LOG_LEVEL_INFO;
					archivoDeLog = log_create("LogFileSystem", "FileSystem", 0, nivel);

					char *unaPalabra = string_new();


					 string_append(&unaPalabra, "NRO BLOQUE: ");
					 string_append(&unaPalabra, string_itoa(numBloque));

										string_append(&unaPalabra, "NRO COPIA: ");
										 string_append(&unaPalabra, string_itoa(nroCopia));

										 string_append(&unaPalabra, " NRO NODO: ");
										 string_append(&unaPalabra, string_itoa(idNodo));

										 string_append(&unaPalabra, " NRO BLOQUE NODO: ");
										 string_append(&unaPalabra, string_itoa(nroBloqueNodo));



								log_info(archivoDeLog,unaPalabra);

								free(unaPalabra);
								log_destroy(archivoDeLog);



}








void loguearComentarios(char* palabrita){

t_log_level nivel;
			t_log* archivoDeLog;
			nivel = LOG_LEVEL_INFO;
				archivoDeLog = log_create("LogFileSystem", "FileSystem", 0, nivel);
				log_info(archivoDeLog,palabrita);
			log_destroy(archivoDeLog);
}


void loguearErrores(char* palabrita){

t_log_level nivel;
			t_log* archivoDeLog;
			nivel = LOG_LEVEL_ERROR;
				archivoDeLog = log_create("LogFileSystem", "FileSystem", 0, nivel);
				log_error(archivoDeLog,palabrita);
				log_destroy(archivoDeLog);
}




void aceptacionDeProceso(char* nombreProceso, int socket){
	t_log_level nivel;
		t_log* archivoDeLog;
		nivel = LOG_LEVEL_INFO;
			archivoDeLog = log_create("LogFileSystem", "FileSystem", 0, nivel);
			 char *unaPalabra = string_new();
					 string_append(&unaPalabra, "Aceptacion de ");
					 string_append(&unaPalabra, nombreProceso);

					 string_append(&unaPalabra, " usando el socket ");
					 string_append(&unaPalabra, string_itoa(socket));

			log_info(archivoDeLog,unaPalabra);

			free(unaPalabra);
			log_destroy(archivoDeLog);
}


void loguearDesconexionDeProceso(char* nombreProceso){

	t_log_level nivel;
	t_log* archivoDeLog;
	nivel = LOG_LEVEL_INFO;
		archivoDeLog = log_create("LogFileSystem", "FileSystem", 0, nivel);
		 char *unaPalabra = string_new();
				 string_append(&unaPalabra, "Desconexion de ");
				 string_append(&unaPalabra, nombreProceso);



		log_info(archivoDeLog,unaPalabra);
		log_destroy(archivoDeLog);
}



void loguearEspacioDisponible(int espacioDisponible){

	t_log_level nivel;
		t_log* archivoDeLog;
		nivel = LOG_LEVEL_INFO;
			archivoDeLog = log_create("LogFileSystem", "FileSystem", 0, nivel);
			 char *unaPalabra = string_new();
					 string_append(&unaPalabra, "Espacio disponible del FileSystem en Mb: ");
					 string_append(&unaPalabra, string_itoa(20 * espacioDisponible));

			log_info(archivoDeLog,unaPalabra);
			log_destroy(archivoDeLog);


}





void recibirArchivoFinal(int socketQueEscribe){
	uint32_t tamanioRuta;
	recibir(socketQueEscribe,&tamanioRuta,sizeof(uint32_t));
	char* rutaArchivo = malloc(tamanioRuta);
	recibir(socketQueEscribe,rutaArchivo,tamanioRuta);
	uint32_t tamanioEnvio;
	recibir(socketQueEscribe,&tamanioEnvio,sizeof(uint32_t));
	char* contenido = malloc(tamanioEnvio);
	recibir(socketQueEscribe,contenido,tamanioEnvio);

	FILE* archivo = fopen(rutaArchivo,"w");

	if(fchmod(fileno(archivo), 0755)==-1){ //creo que no hace falta
	printf("No se genero el permiso\n");
	}

	fprintf(archivo,contenido);

	fclose(archivo);
	free(contenido);
	free(rutaArchivo);
	}


int calcularMD5ArchivoLocal(char* rutaArchivo, char** resultado){
	FILE* archivo;
	md5_state_t unMD5;
	char* datos;
	md5_byte_t codigoMD5[16];
	*resultado = malloc(17);
	int unMega = 1024*1024;
	int restante;

	archivo = fopen(rutaArchivo,"r");

	if(archivo == NULL){
		printf("Error. El archivo solicitado no existe.\n");
		return 1;
	}

	md5_init(&unMD5);

	restante = tamanio_archivo(fileno(archivo));
	while(1){
		if(restante<unMega){
			datos = malloc(restante);
			fread(datos,sizeof(char),restante,archivo);
			md5_append(&unMD5, ((md5_byte_t*) datos), restante);
			free(datos);
			break;
		} else{
			datos = malloc(unMega);
			fread(datos,sizeof(char),unMega,archivo);
			md5_append(&unMD5, ((md5_byte_t*) datos), unMega);
			free(datos);
			restante -= unMega;
		}
	}

	md5_finish(&unMD5, codigoMD5);
	strcpy(*resultado,(char*)codigoMD5);

	fclose(archivo);

	return 0;
}



void copiarArchivoDelMDFSAlFSLocal(char *rutaMDFS, char *rutaLinux){
	if (pthread_mutex_init(&lockEnvioBloque, NULL) != 0) {
		printf("\n mutex init failed\n");
	}

	struct archivos* estructuraArchivo;

	//Necesario del MDFS
	char ** arrayDeDirectoriosMDFS = desglosarRuta(rutaMDFS);
	int ultimaPos = obtenerUltimaPosicionDeUnArray(arrayDeDirectoriosMDFS);
	char* nombreArchivo = arrayDeDirectoriosMDFS[ultimaPos - 1];

	//programa

	char* path= sacaElArchivoALaRuta(rutaMDFS);
	estructuraArchivo = encontrarAlArchivo(nombreArchivo, path);
	struct bloques *auxbloque = malloc(sizeof(struct bloques));
	auxbloque = estructuraArchivo->bloque;
	int cantidadBloques = estructuraArchivo->bloquesDeCadaArchivo;

	int bloqueABuscar;
	int socketNodo;

	int i = 0;
	for(;i<(cantidadBloques+1);i++){
		bloqueABuscar = auxbloque->copia->unNodo->nroBloqueNodo;
		int idNodo = auxbloque->copia->unNodo->idNodo;
		struct nodos* unNodo = obtenerNodo(idNodo);
		socketNodo = unNodo->nroSocket;
		pthread_mutex_lock(&lockEnvioBloque);

		if(cantidadBloques == i){
			break;
		}

		pedirBloque(bloqueABuscar,socketNodo,rutaLinux);

		if(i != cantidadBloques-1 && i != cantidadBloques){
			auxbloque = auxbloque->bloque;
		}

	}

	free(auxbloque);

}


void pedirBloque(int bloqueABuscar,int socketNodo,char* rutaLinux){
	char proceso = 'F';
	char operacion = 'G';
	int tamanioRuta = strlen(rutaLinux)+1;
	enviar(socketNodo,&proceso,sizeof(char));
	enviar(socketNodo,&operacion,sizeof(char));
	enviar(socketNodo,&bloqueABuscar,sizeof(int));
	enviar(socketNodo,&tamanioRuta,sizeof(int));
	enviar(socketNodo,rutaLinux,tamanioRuta);

}

void recibirBloqueDelNodo(int socketQueEscribe){
	int tamanioRuta;
	recibir(socketQueEscribe,&tamanioRuta,sizeof(int));
	char* rutaLinux = malloc(tamanioRuta);
	recibir(socketQueEscribe,rutaLinux,tamanioRuta);
	int tamanio;
	recibir(socketQueEscribe,&tamanio,sizeof(int));
	char* contenido = malloc(tamanio);
	recibir(socketQueEscribe,contenido,tamanio);
	FILE* archivo = fopen(rutaLinux,"a"); //La forma de apertura "a" permite escribir al final
	char* a = "\n";
	char *contenido1 = malloc(tamanio+1);
	contenido1 =strcat(contenido,a);
	fwrite(contenido,tamanio,1,archivo);

	fclose(archivo);
	pthread_mutex_unlock(&lockEnvioBloque);

	free(rutaLinux);
	free(contenido);


}

int ubicacionDelNodo(int idNodo){
	int cantidadNodos = list_size(socketsConectados);
	int i=0;
	int esta=0;
	while(i<cantidadNodos && esta!=1){
		struct nodos* unNodo = list_get(socketsConectados,i);
		if(unNodo->idNodo == idNodo){
			esta=1;
		}
		i++;
	}
	return i-1;
}

int seReconectoNodo(char* puerto){
	int cantidadNodos = list_size(socketsConectados);
	int i=0;
	int esta=-1;
	while(i<cantidadNodos && esta!=1){
		struct nodos* unNodo = list_get(socketsConectados,i);
		if(!strcmp(unNodo->puerto,puerto)){
			esta=1;
		}
		i++;
	}

	if(esta!=-1){
		esta = i-1;
	}

	return esta;
}

void reestablecerDistribucion(struct archivos* archivo){
	int cantBloques = archivo->bloquesDeCadaArchivo;
	int i=0;
	int varChanta = 0;
	t_list* nodoActivos = list_filter(socketsConectados,nodosActivos);
	int cantElementos = list_size(nodoActivos);
	struct bloques* unBloque = archivo->bloque;
	while(i<cantBloques){
		int cantCopias = unBloque->copiasDeCadaBloque;
		int j=0;
		struct copias* unaCopia = unBloque->copia;
		while(j<cantCopias && unaCopia->nroCopia!=-1){
			int idNodo = unaCopia->unNodo->idNodo;

			struct nodos* unNodo = obtenerNodo(idNodo);


			bitarray_clean_bit(unNodo->bloques,unaCopia->unNodo->nroBloqueNodo);

				if(varChanta<cantElementos){
					struct nodos* nodo = list_get(nodoActivos,varChanta);
					if(nodo->estado==1){
						notificaEliminacionAlNodo(nodo->nroSocket,unaCopia->unNodo->nroBloqueNodo);
					}
				varChanta++;
				}

				if(unaCopia->copia != NULL){
					unaCopia = unaCopia->copia;
				}
			j++;
		}

		if(unaCopia->nroCopia==-1){
			return;
		}

		if(unBloque->bloque != NULL){
			unBloque = unBloque->bloque;
		}
		i++;
	}

}

void notificaEliminacionAlNodo(int nroSocket,int nroBloqueNodo){
	char proceso = 'F';
	char operacion = 'D';
	enviar(nroSocket,&proceso,sizeof(char));
	enviar(nroSocket,&operacion,sizeof(char));
	enviar(nroSocket,&nroBloqueNodo,sizeof(int));
}



int espacioDisponibleFS(){
int nodosConectados, j;
int cantidadDeBloquesLibres=0;


nodosConectados = list_size(socketsConectados);

for (j=0; j<nodosConectados; j++){

	struct nodos* unNodo = list_get(socketsConectados,j);
	t_bitarray* bitbloq = unNodo->bloques;
		int i=0;
		while(i< (unNodo->cantidadDeBloques) ){
			if(!bitarray_test_bit(bitbloq, i)){
				cantidadDeBloquesLibres++;
			}
			i++;
		}

	//	int espacioDisponible = 20971520 * cantidadDeBloquesLibres;


}
return cantidadDeBloquesLibres;

 }

void cargarArchivoDeConfiguracion() {

	t_dictionary* diccionario;
	diccionario = dictionary_create();
	char* dato;
	char* clave;

	char textoLeido[200];
	FILE* archivo;
	//archivo = fopen("/home/utnso/workspace/tp-2015-1c-sinergia/Nodo/Debug/configNodo","r");
	char* config = "configFS";
	archivo = fopen(config,"r");


	while ((feof(archivo)) == 0) {
		fgets(textoLeido, 200, archivo);

		clave = string_split(textoLeido, ",")[0];
		dato = string_split(textoLeido, ",")[1];
		dictionary_put(diccionario, clave, dato);

	}

	// IMPORTANTE hay que hacer esto que viene abajo pq el dato obtenido del diccionario viene con un caracter
	//de mas entonces tenemos que "limpiarlo" (se lo sacamos)




	puertoFS = malloc(10);
	//cantidadDeNodos = malloc(10);

	puertoFS = obtenerDatoLimpioDelDiccionario(diccionario, "puertoFS");
	cantidadDeNodos = obtenerDatoLimpioDelDiccionario(diccionario, "cantidadDeNodos");
	dictionary_destroy(diccionario);

}

char* obtenerDatoLimpioDelDiccionario(t_dictionary* diccionario, char* dato) {

	char* datoProvisorio;
	char* datoLimpio;
	datoProvisorio = dictionary_get(diccionario, dato);
	datoLimpio = string_substring_until(datoProvisorio,(string_length(datoProvisorio) - 1));
	return datoLimpio;
}
