/*
 * mensaje.h
 *
 *  Created on: 3/6/2015
 *      Author: utnso
 */

#ifndef MENSAJE_H_
#define MENSAJE_H_

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
#include <commons/log.h>



char* puertoFS;
char* cantidadDeNodos;

pthread_mutex_t lockEnvioBloque;
t_list* socketsConectados;
int tamanioChanta;
int flagChanta;
int idNodoChanta;
int idNodoBloquePedido1;
int idNodoBloquePedido2;

	struct directorios{
		int id;
		char nombre[50];
		int dirPadre;
	};
	struct directorios directorio[1024];

	struct archivos{
		char* nombre;
		int tamanio;
		int dirPadre;
		int estado;
		int bloquesDeCadaArchivo;
		struct bloques *bloque;
		struct archivos *archivo;
	};

	struct archivos* ptrArchivo;
	struct archivos* aux_ptrArchivo;

	struct bloques{
		int numBloque;
		int copiasDeCadaBloque;
		struct copias *copia;
		struct bloques *bloque;
	};


	struct copias{
		int nroCopia;
		struct nodoPos *unNodo;
		struct copias *copia;
	};


	struct nodoPos{
		struct nodos* unNodo;
		int idNodo;
		int nroBloqueNodo; //id del bloque en el nodo. Es decir donde se va a guardar el bloque dentro del nodo.
	};

	struct nodos{
		int idNodo;
		int estado; //1 estado activo, 0 estado inactivo
		int nroSocket;
		int cantidadDeBloques;
		t_bitarray *bloques;
		char* ipNodo;
		char* puerto;
	};


typedef struct{
	int nroBloque;
	char* bloque;
	uint32_t bloque_long;
	uint32_t total_size;
}t_Package;

typedef struct{
	int nroSocketNodo;
	char* ipNodo;
	char* puertoNodo;
}creacionNodo;


#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

	//FUNCIONES ---------------------------------------------------



	void loguearCopiasEnviadas(int numero,int nroCopia,int idNodo, int nroBloqueNodo);
	char* obtenerDatoLimpioDelDiccionario(t_dictionary* diccionario, char* dato);
	void cargarArchivoDeConfiguracion();
	void loguearErrores(char* palabrita);
	void loguearComentarios(char* palabrita);
	int espacioDisponibleFS();
	void loguearEspacioDisponible(int espacioDisponible);
	void formatearArchivoDeLog();
	void loguearDesconexionDeProceso(char* nombreProceso);
	void aceptacionDeProceso(char* nombreProceso, int socket);
	int distribuirArchivoEnBloques(int, struct archivos *);
	void rellenarBloque(int, char **);
	int tamanio_archivo(int);
	void* manejoConsola(void);
	void cargarUnBloque(int, struct bloques*);
	void armarYEnviarBloqueANodo(char *,int,struct bloques*);
	void armaEstructurasYEnviaCopias(int, struct bloques *, char*);
	void copiarArchivoLocalAlMDFS(char *);
	int obtenerPosLibreDeTablaDeDirectorios(struct directorios []);
	void crearDirectorio(char *,char*);
	void crearDirectorios(char *);
	void inicializarTablaDeDirectorios();
	void eliminarDirectorio(char *);
	void eliminarUnDirectorio(int);
	void eliminarLosDirectoriosHijosDe(int);
	void renombrarDirectorio(char*, char*);
	void renombrarUnDirectorio(char* ,int );
	void moverDirectorioA(char*, char*);
	void moverUnDirectorioA(int, int);
	void cargarEstructuras(int, struct bloques *);
	void verBloquesQueComponenAUnArchivo(char *);
	char** desglosarRuta(char*);
	int obtenerUltimaPosicionDeUnArray(char**);
	t_list* convertirAListaDeCommonsArchivos(struct archivos *);
	void convertirAListaDeCommonsBloques(t_list*,struct bloques*);
	void convertirAListaDeCommonsCopias(t_list* lista, struct copias* copia);
	int obtenerIdDelUltimoDirectorioDe(char *);
	void verificarRuta(int*, char* );
	char* sacaElArchivoALaRuta(char *);
	struct archivos* encontrarAlArchivo(char*, char*);
	void renombrarUnArchivo(char*, char*);
	void eliminarUnArchivo(char* );
	void moverUnArchivo(char*, char*);
	bool escribio(void* );
	int recibirMensajeDe(int);
	char recibirChar(int);
	void* manejoConexiones(void*);
	void cargarDatosDeBloques(struct nodos*,int);
	char* serializarEnvioDeBloqueANodo(t_Package*);
	struct nodoPos* obtenerDestinoDeBloque(int);
	void* cantidadDeBloquesLibres(void*);
	void enviaCopiaANodo(struct copias*, char* );
	void *manejoHiloNodo(void *socket_desc);
	void *manejoHiloMarta(void *socketCli);
	void fill_package(t_Package *, char *, int);
	void renombrarUnArchivo(char*, char* );
	void martaPideBloquesDeArchivos(int);
	t_list* deserializarPedidoDeMarta(char *);
	void* obtenerEstructuraDeArchivo(void*);
	char* serializarEstructurasDeArchivos(t_list*);
	int seEncuentraElBloqueEnElNodo(int, t_list*);
	int enviar(int, char*, int);
	int recibir(int, char*, int);
	struct nodos* obtenerNodo(int);
	void recibirArchivoFinal(int);
	void pedirBloque(int bloqueABuscar,int socketNodo,char* rutaLinux);
	void copiarArchivoDelMDFSAlFSLocal(char *rutaMDFS, char *rutaLinux);
	int calcularMD5ArchivoLocal(char* rutaArchivo, char** resultado);
	void recibirBloqueDelNodo(int socketQueEscribe);
	int ubicacionDelNodo(int);
	int seReconectoNodo(char*);
	bool nodoActivoYConLugar(void *elemento);
	int cantidadDeNodosActivosYConLugar();
	int sePuedeDistribuirElArchivo(int tamanioArchivo);
	int nodosTienenLugarSuficiente(int tamanioArchivo);
	int cantBloquesLibres(t_bitarray* bloques, int cantBloques);
	bool tieneElArchivoDisponible(void* elemento);
	int yaExisteElDirectorio(char* nombreDirectorio, char* rutaDelPadre);
	int cantCopiasDisponiblesDeUnBloque(struct bloques* unBloque);
	int cantCopiasDisponiblesDeUnArchivo(struct archivos* unArchivo);
	void reestablecerDistribucion(struct archivos* archivo);
	void notificaEliminacionAlNodo(int nroSocket,int nroBloqueNodo);
	bool nodosActivos(void *elemento);
	int enviar2(int s, char *buf, int len);

	void formatearMDFS();
	void verBloques();
	void borrarBloques();
	void copiarBloques();
	void verBloquesQueComponenAUnArchivo(char *);
	void persistirArchivo(FILE*,struct archivos*);
	void persistirBloques(FILE*,struct bloques*);
	void persistirCopias(FILE*,struct copias*);
	void persistirNodoPos(FILE *, struct nodoPos*);
	int persistirArchivoPosta(struct archivos*);
	void verBloquesQueComponenAUnArchivo(char *);
	struct archivos* despersistirArchivoPosta();
	void borrarBloquesQueComponenAUnArchivo(char*,int[]);
	int esta(int, int[]);
	void copiarBloquesQueComponenAUnArchivo(char*, int[]);
	void persistirDirectorios();
	void despersistirDirectorios();
	void despersistirArchivos();


	void mostrarDirectorios(); // para probar

#endif /* MENSAJE_H_ */
