#ifndef MENSAJESJOB_H_
#define MENSAJESJOB_H_

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
#include <commons/txt.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netdb.h>
#include <stdbool.h>
#include <commons/log.h>

typedef	struct nodo{
		int idNodo;
	    char* ipNodo;
	    char* puertoNodo;
}nodo;

typedef	struct bloquePrimaJob {
		int idBloqueNodo; //id del bloque en el nodo. Es decir donde se va a guardar el bloque dentro del nodo.
		char* nombreEnDondeGuardarlo; //tmb sirve como nombre de donde sacarlo
}bloqueJob;

typedef struct{
		struct nodo nodo;
		t_list* bloques; //de struct bloquePrima
		char* nombreDelReduce;
}estructura;

typedef struct{
	estructura* unaEstructura;
	int nroSocket;
}envioMap;

pthread_t hiloConexionNodo;

//ENCABEZADOS

void loguearCreacionDeHiloReduce(char* unaIP);
void loguearCabecerasDeMensajes(char operacion);
void loguearFinalizacionDeHiloScript(char* maporeduce);
void formatearArchivoDeLog();
void loguearCreacionDeHilosMapper(char* unNumero);
void loguearConexionConMarta(int unNumero,char* ipMarta,char* puertoMarta);
int conectarConNodo(char* ip, char* puerto);
void* hacerLaburarAlNodo(estructura* unaEstructura);
void mandarALaburarALosNodos(t_list* );
int conectarAMarta();
void pedirAMartaInfoParaTrabajar();
void cargarArchivoDeConfiguracion();
void conseguiRutaArchivos(t_dictionary* diccionario);
void mandarCadena(int socket,char* cadena);
char* recibirCadena(int socketQueEscribe);
t_list* escucharOrdenesDeMarta(char);
char* obtenerDatoLimpioDelDiccionario(t_dictionary* diccionario, char* dato);
int enviar(int, char *, int);
int recibir(int, char *, int);
void escucharBloques(char id,estructura* estructuraDeTrabajo);
//void enviarAMartaResultadoOperacion(estructura* nodo);
void enviarAMartaResultadoOperacion(envioMap*, char);
void enviarOperacionMapANodos(int socketNodo, estructura* unNodoConSusBloques);
char* serializarEnvioDeOperacionMap(estructura* unNodoConSusBloques, int* longitudMensaje);
void envioDeScriptMap(int socketNodo);
int tamanio_archivo(int fd);
void* hacerLaburarAlNodoPorBloque(envioMap* unEnvio);
//char* serializarEnvioDeReduce(t_list* listaDeNodosAReducir, int socketNodo, int* longitudMensaje);
int mandarAReducirALosNodos(t_list* listaDeNodosAReducir);
void enviarAMartaResultadoOperacionReduce(t_list* listaConNodosAMapear, int nroSocket,char resultado);
char recibirResultadoDelNodo(int socketNodo);
void envioDeScriptReduce(int socketNodo);
void envioDeReduceAlNodo(t_list* listaDeNodosAReducir, int socketNodo);
static void nodos_destroy(estructura *self);
static void bloquejob_destroy(bloqueJob *self);
void* operacionReduce(t_list*);

//replanificar
int cambiarVariableGlobal(int id);
void manejarReplanificacionDeMap();
int elem(t_list* lista, int elemento);
int elem2(t_list* lista, int elemento);
void agregarAListaGlobal(int id);

#endif
