/*
 * mensajesNodo.h
 *
 *  Created on: 8/6/2015
 *      Author: utnso
 */

#ifndef MENSAJESNODO_H_
#define MENSAJESNODO_H_

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
#include <commons/collections/dictionary.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netdb.h>
#include <stdbool.h>
#include <math.h>
#include <commons/log.h>

/*
char* puertoFileSystem= "8888";
char* ipFileSystem = "127.0.0.1";
*/
char* puertoFileSystem;
char* ipFileSystem;
int idNodo;
char* ipNodo;
char* puertoNodo;
char* tamanioNodo;
int tamanioDeEspacioDeDatos;
char* nodoNuevo;
char* dirTemp;

t_list* socketsConectados;

typedef struct{
	int nroBloque;
	char* contenidoBloque;
	uint32_t contenidoBloque_long;
}t_Package;

typedef	struct{
		int idNodo;
	    char* ipNodo;
	    char* puertoNodo;
}nodo;

typedef struct{
	nodo nodo;
	t_list* listaNombres; //De char*
}estructuraNN;


void cambiarCondicionANodoViejo();
void guardarParametrosDeConfiguracion( char* parametro, char* configDelParametro);
void formatearArchivoDeConfig();
void formatearArchivoDeLog();
void enviarRespuestaDelMapAlJob(char unaRespuesta, int socketQueEscribe);
void loguearEscrituraEnBloque(int indiceDelBloque);
void loguearLecturaDeEspacioTemporal();
void loguearEscrituraDeEspacioTemporal();
void loguearFalloDeEscrituraDeEspacioTemporal();
void loguearLecturaDeBloques(int indiceDelBloque);
void logearOrdenReduce(char* nombre);
void logearOrdenMap(char* nombre);
void aceptacionDeProceso(char* nombreProceso, int socket);
void loguearDesconexionDeProceso(char* nombreProceso,int nroSocket);
void loguearConexionConProceso(char* cadena,int seLogroConectar, int socket);
int conectarAFileSystem();
void* manejoConexionesNodosYJobs(void*);
void *manejoHiloNodo(void *);
void *manejoHiloJob(void *);
int recibirMensajeDe(int);
int conectarANodo();
char recibirChar(int);
int tamanio_archivo(int);
char* mapearAMemoriaElEspacioDeDatos(char*);
//struct package* deserializarEnvioDeBloqueANodo(char *);
int recibirYdeserializarBloques(t_Package*,int);
void guardarContenidoEnEspacioTemporalDelNodo(FILE*,char*,int,int);
char* conseguirUnBloquePedido(int bloque);
int tamanioArchivo(int fd);
void redireccionarScript(int fdEntrada, int fdSalida, char* rutaScript);
void cargarArchivoDeConfiguracion();
char* obtenerDatoLimpioDelDiccionario(t_dictionary* diccionario, char* dato);
void armarArchivoParaMapear(int bloque, char* ruta);
char* generarRuta();

int llamada_al_programa_redireccionando_stdin_out_ordenando(char*, char*, int (*escribirArchivoEntrada) (void*, void*), char*, int);
int escribirEnArchivo(FILE*, char*);
void apareoDeArchivos(int, char* archivos[], char*);
char ejecutarReduce(char* rutaScriptReduce,char* archivoDondeLoGuardo,t_list* listaNodoNombres);
char ejecutarMap(char* rutaScriptMap,char* archivoDondeLoGuardo, int bloque, char* rutaEntrada);
void recibirScript(int socketQueEscribe);
void recibirOrdenMap(int socketQueEscribe);
int enviar(int s, char *buf, int len);
int recibir(int s, char *buf, int len);
void conseguirBloqueYEnviar(int socket);
void recibirBloque(int socket,char*ruta);
char* pedirBloqueAOtroNodo(nodo nodo, char* nombre);
t_list* deserializarOrdenDeReduce(char* mensajeSerializado);
void recibirOrdenReduce(int socketQueEscribe);
void* manejoConexionesConFS(void* parametro);
void conseguirYEnviarLoQuePideElFS(int socket_FS);
char* generarNombreRandom();
t_list*  recibirEstructurasDelNodo(int);
char* recibirCadena(int socketQueEscribe);
void conseguirArchivoFinal(int socket_FS);
static void estructuraN_destroy(estructuraNN *self);
static void nombres_destroy(char *self);
void conseguirBloqueYEnviarAlFS(int socket_FS);

#endif /* MENSAJESNODO_H_ */
