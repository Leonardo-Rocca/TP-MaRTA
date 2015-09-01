/*
 ============================================================================
 Name        : Prueba2.c
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>

#include "mensajesMarta.h"

/*
	typedef struct nodoDos{
		int idNodo;
	    int ipNodo;    //hacer q el FS me la de -------
	    int puertoNodo;    //hacer q el FS me la de -------
	}modelon;

	typedef struct bloquePrimaDos{
		int idBloqueNodo; //id del bloque en el nodo. Es decir donde se va a guardar el bloque dentro del nodo.
		int idBloqueArchivo;
//		struct archivo idArchivo;
	}modelob;

	typedef struct{
		struct nodoDos nodo;
		t_list* bloques; //de bloquePrima   struct..
	}modeloe;

//t_list* planificarNodos(t_list* listaNodoConArchivos,int combiner);
t_list* planificarNodosConCombiner(t_list* listaNodoConArchivos);
t_list* planificarNodosSinCombiner(t_list* listaNodoConArchivos);
_Bool comparatorMenorAMayor (modeloe* estructuraDeTrabajo1, modeloe* estructuraDeTrabajo2);
_Bool comparatorMayorAMenor (modeloe* estructuraDeTrabajo1, modeloe* estructuraDeTrabajo2);
int cantidadDeBloquesArchivo(t_list* listaNodoConArchivos);
int posicionBloqueArchivoEnEstructura(modeloe estructuraDeTrabajo,int idBloqueArchivo); //Devuelve -1 si no la encuentra.
int posicionListaAuxiliarContieneNodo(t_list* listaAuxiliar,modeloe estructuraDeTrabajo); //Devuelve -1 si no la encuentra.


struct archivo{
		char nombre[40];
		int tamanio;
	};

typedef	struct nodo{
		int idNodo;
	    int ipNodo;    //hacer q el FS me la de -------
	    int puertoNodo;    //hacer q el FS me la de -------
	};

	typedef	struct bloquePrima {
		int idBloqueNodo; //id del bloque en el nodo. Es decir donde se va a guardar el bloque dentro del nodo.
		int idBloqueArchivo;
		struct archivo idArchivo;
	}modelob;

	typedef struct{
		struct nodo nodo;
		t_list* bloques; //de bloquePrima   struct..
	}modeloe;

	struct ruta{
		char* rutaArchrivos;
	};

	struct nodoDeDondeSacarloYArchivoTemporal{
		struct nodo nodoDeDondeSacarlo;
	    char* archivoTemporal;
	};


	typedef struct{
		int combiner;
		char* rutaDestinoDeResultado;
		uint32_t long_RutaDestino;
		char* rutaDeArchivos;
		t_list* rutasDeArchivos;
	}t_Package;*/

int mainTest(){
	//Datos de prueba
t_list* listaNodoConArchivos=crearListaPrueba();

	//Imprimir datos



	//Ejecutar planificacion (con o sin combiner)

	t_list* resultado;// = list_create();

	resultado = planificarNodos(listaNodoConArchivos,1);

	//Imprimir resultado

	modeloe* res;
	struct bloquePrima* resb;
	int cantN = 0;int cantB = 0;int i = 0;int j = 0;
	cantN = list_size(resultado);

	printf("Resultado\n\n");

	while(cantN>i){
	res = list_get(resultado,i);
	printf("Nodo:%d\n",res->nodo.idNodo);
	cantB = list_size(res->bloques);
	j = 0;
		while(cantB>j){
			resb = list_get(res->bloques,j);
	//		printf("BloqueArchivo:%d\n",resb->idBloqueArchivo);
			j++;
		}
	i++;
	printf("\n");
	}

printf("Finish");
return 0;
}

t_list* planificarNodos(t_list* listaNodoConArchivos,int combiner){
	t_list* resultado;// = list_create();

	if(combiner == 1){
		resultado = planificarNodosConCombiner(listaNodoConArchivos);
		return resultado;
	}else if(combiner == 0){
		resultado = planificarNodosSinCombiner(listaNodoConArchivos);
		return resultado;
	}
	eliminarListaEstructuraDeTrabajo(listaNodoConArchivos);
	return resultado;
};

t_list* planificarNodosSinCombiner(t_list* listaNodoConArchivos){
	t_list* listaAuxiliar = list_create();
	list_sort(listaNodoConArchivos,*comparatorMayorAMenor);
	t_list* bloquesAuxiliares = list_create();
	modeloe* estructuraAuxiliar = list_get(listaNodoConArchivos,0);
	list_add_all(bloquesAuxiliares,estructuraAuxiliar->bloques);
	if(list_size(bloquesAuxiliares) == cantidadDeBloquesArchivo(listaNodoConArchivos)){
		list_add(listaAuxiliar,estructuraAuxiliar);
		return listaAuxiliar;
	}
	int i = 1;
	list_sort(listaNodoConArchivos,*comparatorMayorAMenor);
	while(list_size(bloquesAuxiliares)>=i){
		int j = 1;
		while(list_size(listaNodoConArchivos)>j){
			modeloe* estructuraDeTrabajo = list_get(listaNodoConArchivos,j);
			int posicion = posicionBloqueArchivoEnEstructura(*estructuraDeTrabajo,i);
			if(posicion != -1){
				list_remove(estructuraDeTrabajo->bloques,posicion);
			}
			j++;
		}
		i++;
	}
	listaAuxiliar = planificarNodosConCombiner(listaNodoConArchivos);
	//FALTA borralistas: bloquesAUxiliares(),
	return listaAuxiliar;
}

t_list* planificarNodosConCombiner(t_list* listaNodoConArchivos){
	t_list * listaAuxiliar = list_create();
	list_sort(listaNodoConArchivos,*comparatorMenorAMayor);
	int i = 1;
	while(cantidadDeBloquesArchivo(listaNodoConArchivos)>=i){
		int e = 0;
		while(list_size(listaNodoConArchivos)>e){
			modeloe* estructuraDeTrabajo;
			estructuraDeTrabajo = list_get(listaNodoConArchivos,e);
			int b = 0;
			while(list_size(estructuraDeTrabajo->bloques)>b){
				int posicion = posicionBloqueArchivoEnEstructura(*estructuraDeTrabajo,i);
				if(posicion != -1){
					modeloe* estructuraAuxiliarPrima = malloc(sizeof(modeloe));
					int posicionAux = posicionListaAuxiliarContieneNodo(listaAuxiliar, *estructuraDeTrabajo);
					if(posicionAux != -1){
						modeloe* estructuraAuxiliar = malloc(sizeof(modeloe));
						estructuraAuxiliar = list_get(listaAuxiliar,posicionAux);
						list_add(estructuraAuxiliar->bloques,list_get(estructuraDeTrabajo->bloques,posicion));
						list_replace(listaAuxiliar,posicionAux,estructuraAuxiliar);
					}else{
					modelob* bloqueAuxiliar = malloc(sizeof(modelob));
					estructuraAuxiliarPrima->bloques = list_create();
					bloqueAuxiliar = list_get(estructuraDeTrabajo->bloques,posicion);
					estructuraAuxiliarPrima->nodo = estructuraDeTrabajo->nodo;
					list_add(estructuraAuxiliarPrima->bloques,bloqueAuxiliar);
					modeloe* nodoC = malloc(sizeof(modeloe));
					nodoC->nodo=estructuraAuxiliarPrima->nodo;
					nodoC->bloques = estructuraAuxiliarPrima->bloques;
					list_add(listaAuxiliar,nodoC);
					}
				list_remove(listaNodoConArchivos,e);
				list_add(listaNodoConArchivos,estructuraDeTrabajo);
				e = list_size(listaNodoConArchivos);
				break;
				}
			b++;
			}
		e++;
		}
	i++;
	}
	return listaAuxiliar;
}

_Bool comparatorMenorAMayor (modeloe* estructuraDeTrabajo1, modeloe* estructuraDeTrabajo2){
	int cant1 = list_size(estructuraDeTrabajo1->bloques);
	int cant2 = list_size(estructuraDeTrabajo2->bloques);
	return (cant1 < cant2);
}

_Bool comparatorMayorAMenor (modeloe* estructuraDeTrabajo1, modeloe* estructuraDeTrabajo2){
	int cant1 = list_size(estructuraDeTrabajo1->bloques);
	int cant2 = list_size(estructuraDeTrabajo2->bloques);
	return (cant1 > cant2);
}

int cantidadDeBloquesArchivo(t_list* listaNodoConArchivos){
	int maximo = 0;
	int cantEstructuras = list_size(listaNodoConArchivos);
	int i=0;
	while(i<cantEstructuras){
		modeloe* nodoC = list_get(listaNodoConArchivos,i);
		int cantBloques = list_size(nodoC->bloques);
		int j=0;
		while(j<cantBloques){
			modelob* unBloque = list_get(nodoC->bloques,j);
			if(unBloque->idBloqueArchivo > maximo){
				maximo = unBloque->idBloqueArchivo;
			}
			j++;
		}
		i++;
	}

	return maximo;

}

int posicionBloqueArchivoEnEstructura(modeloe estructuraDeTrabajo,int idBloqueArchivo){
	int i = 0;
	while(list_size(estructuraDeTrabajo.bloques)>i){
		modelob* bloque;
		bloque = list_get(estructuraDeTrabajo.bloques,i);
		if(idBloqueArchivo == bloque->idBloqueArchivo){
			return i;
		}else{
			i++;
		}
	}
	return -1;
}

int posicionListaAuxiliarContieneNodo(t_list* listaAuxiliar,modeloe estructuraDeTrabajo){
	if(list_size(listaAuxiliar) != 0){
		int i = 0;
		while(list_size(listaAuxiliar)>i){
			modeloe* estructura;
			estructura = list_get(listaAuxiliar,i);
			if(estructura->nodo.idNodo == estructuraDeTrabajo.nodo.idNodo){
				return i;
			}
			i++;
		}
		return -1;
	}else{
		return -1;
	}
}


///////////FUNCIONES para Probar///////////
/* no sirve
t_list* crearListaDePrueba(){
	t_list * listaNodoConArchivos = list_create();

	modelob b1;
	b1.idBloqueArchivo = 1;
	modelob b2;
	b2.idBloqueArchivo = 2;
	modelob b3;
	b3.idBloqueArchivo = 3;
	modelob b4;
	b4.idBloqueArchivo = 4;
	modelob b5;
	b5.idBloqueArchivo = 5;
	modelob b6;
	b6.idBloqueArchivo = 6;

	modelon n1;
	n1.idNodo = 1;
	modeloe e1;
	e1.nodo = n1;
	e1.bloques = list_create();
	list_add(e1.bloques,&b1);
	list_add(e1.bloques,&b2);
	list_add(listaNodoConArchivos,&e1);

	modelon n2;
	n2.idNodo = 2;
	modeloe e2;
	e2.nodo = n2;
	e2.bloques = list_create();
	list_add(e2.bloques,&b2);
	list_add(e2.bloques,&b3);
	list_add(listaNodoConArchivos,&e2);

	modelon n3;
	n3.idNodo = 3;
	modeloe e3;
	e3.nodo = n3;
	e3.bloques = list_create();
	list_add(e3.bloques,&b4);
	list_add(listaNodoConArchivos,&e3);

	modelon n4;
	n4.idNodo = 4;
	modeloe e4;
	e4.nodo = n4;
	e4.bloques = list_create();
	list_add(e4.bloques,&b1);
	list_add(e4.bloques,&b2);
	list_add(e4.bloques,&b3);
	list_add(e4.bloques,&b4);
	list_add(listaNodoConArchivos,&e4);

	modelon n5;
	n5.idNodo = 5;
	modeloe e5;
	e5.nodo = n5;
	e5.bloques = list_create();
	list_add(e5.bloques,&b5);
	list_add(e5.bloques,&b6);
	list_add(listaNodoConArchivos,&e5);

	modelon n6;
	n6.idNodo = 6;
	modeloe e6;
	e6.nodo = n6;
	e6.bloques = list_create();
	list_add(e6.bloques,&b6);
	list_add(listaNodoConArchivos,&e6);
	return listaNodoConArchivos;

}
*/
void imprimirLista(t_list* listaNodoConArchivos){
	modeloe* dat;
		struct bloquePrima* datb;
		int cantN, cantB, i = 0, j = 0;
		cantN = list_size(listaNodoConArchivos);

		printf("Datos\n\n");

		while(cantN>i){

		dat = list_get(listaNodoConArchivos,i);
		printf("Nodo:%d\n",dat->nodo.idNodo);
		cantB = list_size(dat->bloques);
		j = 0;
			while(cantB>j){
				datb = list_get(dat->bloques,j);
				printf("BloqueArchivo:%d\n",datb->idBloqueArchivo);
				j++;
			}
			i++;
			printf("\n");
		}
}

t_list* crearListaPrueba(){
	t_list* lista=list_create();
	agregarElementoPrueba(lista,7,1,0);
	agregarElementoPrueba(lista,678,2,0);
	agregarElementoPrueba(lista,666,3,0);

	return lista;
}

void agregarElementoPrueba(t_list* lista,int idNodo,int idBloque, int bloqueNodo){
	modeloe* nodoE = malloc(sizeof(modeloe));
	nodoE->nodo.idNodo = idNodo;
	nodoE->nodo.ipNodo="ip.dammy";
	nodoE->nodo.puertoNodo="puertoDammy";
	nodoE->bloques = list_create();

	modelob* bloque = malloc(sizeof(modelob));
	bloque->idBloqueArchivo=idBloque;
	bloque->idBloqueNodo = bloqueNodo;
	bloque->idArchivo.ruta ="ruta";

	list_add(nodoE->bloques,bloque);
	list_add(lista,nodoE);
}
void imprimirTabla(t_list* tabla){
	struct eslabonDeSubTabla* dat;
	t_list* fila=list_get(tabla,0);
	int k =0;
	printf("Datos de la tabla ;) \n\n");
	int cantidadDeFilas = list_size(tabla);

	for(k;cantidadDeFilas>k;){
		printf("fila :%d\n",k);
		 archivoMap* datb;
		int cantN, cantB, i = 0, j = 0;

		cantN = list_size(fila);


		while(cantN>i){

		dat = list_get(fila,i);
		printf("Nodo:%d\n",dat->nodo.idNodo);
		cantB = list_size(dat->nombreDelMapp);
		j = 0;
		printf("Estado del nodo :%d\n",dat->estado);
			while(cantB>j){
				datb = list_get(dat->nombreDelMapp,j);
				printf("Nombre Archivo temporal(del map) :%s\n",datb->nombreEnElArchivoTemporal);
				printf("Estado :%d\n",datb->estado);
				j++;
			}
			i++;
			printf("Nombre Archivo temporal(del reduce) :%s\n",dat->nombreEnElArchivoTemporal);
			printf("\n");
		}
		k++;fila=list_get(tabla,k);

	}
}

void destruirElemento(void* elemento){
	void* a=NULL;
	free(elemento);
	//printf("destrui elemento \n");
	return;
}
void destruirEstructuraDeTrabajo(void* estructura){
 modeloe* estructuraDeTrabajo = (modeloe*)estructura;
 list_destroy_and_destroy_elements(estructuraDeTrabajo->bloques,(void*)destruirElemento);
 free(estructura);

}
void eliminarListaEstructuraDeTrabajo(t_list* lista){
	list_destroy_and_destroy_elements(lista,(void*)destruirEstructuraDeTrabajo);
	printf("elimine lista \n");
}
void destruirVagon(vagon* unVagon){
 t_list* lista = unVagon->nombreDelMapp;
 list_destroy_and_destroy_elements(lista,(void*)destruirElemento);
 free(unVagon);
}
void destruirFilas(void* fila){
 (t_list*)fila;
 list_destroy_and_destroy_elements(fila,(void*)destruirVagon);
}
void eliminarTabla(t_list* tabla){
	list_destroy_and_destroy_elements(tabla,(void*)destruirFilas);
	printf("elimine tabla \n");
}

