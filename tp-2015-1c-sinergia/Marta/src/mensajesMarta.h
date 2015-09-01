/*
 * mensajesMarta.h
 *
 *  Created on: 16/6/2015
 *      Author: utnso
 */

#ifndef MENSAJES_H_
#define MENSAJES_H_

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>
#include <ctype.h>
#include <time.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <commons/collections/dictionary.h>
#include <commons/bitarray.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netdb.h>
#include <stdbool.h>

char* puertoFileSystem;
char* ipFileSystem;
int varChanta;


t_list* socketsConectados;

typedef struct archivo{
		char* ruta;
		int tamanio;
}archivo;

typedef	struct nodo{
		int idNodo;
	    char* ipNodo;
	    char* puertoNodo;
}modelon;

typedef	struct bloquePrima {
		int idBloqueNodo; //id del bloque en el nodo. Es decir donde se va a guardar el bloque dentro del nodo.
		int idBloqueArchivo;
		struct archivo idArchivo;
}modelob;

typedef struct{
		struct nodo nodo;
		t_list* bloques; //de struct bloquePrima
}modeloe;

struct ruta{
		char* rutaArchrivos;
};

struct nodoDeDondeSacarloYArchivoTemporal{
		struct nodo nodoDeDondeSacarlo;
	    char* archivoTemporal;
};

typedef struct eslabonDeSubTabla{
		struct nodo nodo;
		int estado; //0 en ejecucion , 1 ejecutado
		t_list* nombreDelMapp;
		char* nombreEnElArchivoTemporal;
}vagon;

typedef struct archivoMap{
	  	char* nombreEnElArchivoTemporal;
	  	int estado;
	  	int idBloqueArchivo; //solo esta este campo para la replanificacion
}archivoMap;

typedef struct{
		int combiner;
		char* rutaDestinoDeResultado;
		uint32_t long_RutaDestino;
		t_list* rutasDeArchivos;
}t_Package;

int socketFS;
struct nodo variableGlobalNodo;
t_list* variableGlobalBloques;
	//Encabezados


	void cargarArchivoDeConfiguracion();
	char* obtenerDatoLimpioDelDiccionario(t_dictionary* diccionario, char* dato);
	int rdtsc(); //para los nros aleatorios
	int conectarAFileSystem();
	void* manejoConexionConFS(void*);
	void* manejoConexionesConJob(void*);
	bool escribio(void*);
	int recibirMensajeDe(int);
	char* serializarPedidoDeArchivosAlFileSystem(t_list*,int*);
	void gestionarPedidoDeJob(int);
	void recibirYdeserializarBloques(t_Package*,int);
	t_list* deserializarYarmarEstructura(void);
	t_list* solicitarArchivosAlFileSystem(t_list*);
	void solicitarFyleSystemCopiarResultado(char*,struct nodoDeDondeSacarloYArchivoTemporal );
	void ejecutarJob();
	t_list* planificarNodos(t_list*,int);
	struct nodoDeDondeSacarloYArchivoTemporal AplicarJob(t_list*);//(nodoConBloquesAEnviar)
	void avisarJobTareaFInalizada();
	void *manejoHiloJob(void *);

	///Ejecucion de MAPS Y REDUCES///
	int mandarAEjecutarJob(t_list* estructuraDeTrabajo,int combiner,int socketQueEscribe,char* rutaDeResultadoDondeLoQuiere,t_list* estructuraDeTrabajoCompleta,t_list* l);
	int manejarElFalloDeEjecucucion(int socketQueEscribe,t_list* tabla, t_list* estructuraDeTrabajo,int combiner,t_list* rutas);
	void replanificarMap(int socketQueEscribe,int idNodoCaido ,t_list* tabla, t_list* estructuraDeTrabajoConTodosLosNodos,int combiner,t_list* rutas);
	t_list* mandarAEjecutarUnMappACadaNodo(t_list*,t_list*,int);
	char recibirChar(int unSocket);
	int ubicacionDelNodoEnEstructuras(int, t_list*);
	int enviar(int, char*,int);
	int recibir(int, char*,int);
	t_list* agregarNodoATabla(t_list* tabla,struct nodo nodoAMappear);
	t_list* enviarEstructuraYArmarTabla(modeloe* estructuraAuxiliar,int socket,t_list* tabla);
	t_list* agregarATablaEslabon(t_list* tabla,struct eslabonDeSubTabla eslabonAux);
	_Bool sonTodosLosEstadosArchivosUno (archivoMap* h);
	_Bool sonTodosLosEstadosNodosUno (struct eslabonDeSubTabla* h);
	_Bool esEslabonQueBusco(struct eslabonDeSubTabla* eslabon);
	struct eslabonDeSubTabla*	obtenerEslabon(int idNodo,t_list* tabla);
	void informarListaArchivosResultadoExitoso(t_list* listaDeArchivos,char* archivoDondeSeGuardo);
	void enviarAEjecutarReduce(struct eslabonDeSubTabla* ,t_list* tabla,int socketQueEscribe,int agrupaciones,int combiner);
	int generarNumeroDeAgrupaciones(t_list* estructuraDeTrabajo,int combiner);
	bool estaElNodo(void*);
	char* generarNombreRandom();
	void escucharYMandarAEjecutarReduce(t_list* tabla,int combiner,int socketQueEscribe,int agrupaciones);
	int  verificarQueNoTerminoLaOperacion(t_list* tabla,int agrupaciones,int combiner);
	int verificarQueSeEjecutoBien(int);
	t_list* obtenerRama(t_list* tabla,modelon ,int agrupaciones);
	int obtenerTamanioQueDeberiaTenerFilaDeNodo(t_list* tabla,modelon unNodo,int agrupaciones,int combiner);
	int verSiCorrespondeReducirSegunNroNodosDeLaRama(int tamanioDeFilaQueDeberiaTener,int tamanioRama,int losDeLaFilaQueSonUno,int agrupaciones);
	int obtenerTamanioQueDeberiaTenerFilaSegunUnIndice(t_list* tabla,int ,int agrupaciones,int combiner);
	int verSiCorresponde2(int tamanioDeFilaQueDeberiaTener,int tamanioRama ,int agrupaciones,int tamanioQueTiene);

	///Para la Planificacion///
	t_list* planificarNodosConCombiner(t_list* listaNodoConArchivos);
	t_list* planificarNodosSinCombiner(t_list* listaNodoConArchivos);
	_Bool comparatorMenorAMayor (modeloe* estructuraDeTrabajo1, modeloe* estructuraDeTrabajo2);
	_Bool comparatorMayorAMenor (modeloe* estructuraDeTrabajo1, modeloe* estructuraDeTrabajo2);
	int cantidadDeBloquesArchivo(t_list* listaNodoConArchivos);
	int posicionBloqueArchivoEnEstructura(modeloe estructuraDeTrabajo,int idBloqueArchivo); //Devuelve -1 si no la encuentra.
	int posicionListaAuxiliarContieneNodo(t_list* listaAuxiliar,modeloe estructuraDeTrabajo); //Devuelve -1 si no la encuentra.
	void* llenarListaBloques(modeloe* estructura);

	//RE-Planificar
	t_list* borrarDeMiLista(t_list* estructuraDeTrabajoConTodosLosNodos,int idNodoCaido);
	t_list*  filtrarLasPartesDelArchivoQueHayQueRemapear(t_list* estructuraDeTrabajo,vagon* eslabonDelCaido);
	t_list* borrarDeMiLista(t_list* estructuraDeTrabajoConTodosLosNodos,int idNodoCaido);
	void verificarQueEstanTodasLaspartesDelArchivo(t_list* estructuraDeTrabajo);				//TO DO!
	t_list* filtrarLasPartesDelArchivoQueHayQueRemapear(t_list* estructuraDeTrabajo,vagon* eslabonDelCaido);			//TO DO!
	void sacarDeTabla(t_list* tabla,vagon* eslabonDelCaido);
	void cambiarIdNodos(t_list* estructuraDeTrabajo);
	int destransformarIdNodo(int id);
	_Bool esElNodoQueBusco(modeloe* );
	_Bool bloqueDistintoAlQueBusco(modelob* );
	modeloe* dejarSoloLosArchivosQueSirven(modeloe* );
	_Bool listaDeBloquesVacia(modeloe* estructura);
	void armarListaDeArchivosEnVariableGlobalParaPoderFiltrar(vagon* eslabonDelCaido);
	void cambiarIdNodosParaQueNoSeRepitanEnTabla(t_list* estructuraDeTrabajo,t_list* tabla);
	int elem(t_list* lista, int elemento);
	void* obtenerIdNodos(void* eslabon);

	//Funciones para hacer Pruebas
	void imprimirLista(t_list* listaNodoConArchivos);t_list* crearListaPrueba();
	void agregarElementoPrueba(t_list* lista,int ipNodo,int idBloque, int bloqueNodo);
	void imprimirTabla(t_list* tabla);

	char* recibirCadena(int socketQueEscribe);
	void mandarCadena(int socket,char* cadena);
	fd_set listaSockets;

	void destruirElemento(void* elemento);
	void destruirEstructuraDeTrabajo(void* estructura);
	void eliminarListaEstructuraDeTrabajo(t_list* lista);
	void destruirVagon(vagon* unVagon);
	void destruirFilas(void* fila);
	void eliminarTabla(t_list* tabla);


	pthread_mutex_t lockEnvioSolicitudFS;
#endif /* MENSAJES_H_ */
