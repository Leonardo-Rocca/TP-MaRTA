/*
 ============================================================================
 Name        : Apareo.c
 Author      : Fede
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mensajesNodo.h"

void apareoDeArchivos(int cantidadArchivos , char* archivos[], char* rutaEntrada){
	//---------------------------------------------Inicializaciones-------------------------------------------------------------------------

	FILE * file[cantidadArchivos];
	int j = 0;
	int k = 0;
	int comparacion = 0;
	char* menor = malloc(500); //Estamos usando arrays para sacar la info del archivo (lo cual es malo x su tamaño predefinido)
	memset(menor,0,500); //Pongo el arreglo en 0
	int pos = 0;
	int contador = 0; //De archivos cerrados
	int flag = 1;

	FILE* archivoApareado = fopen(rutaEntrada,"a");

	char* info[cantidadArchivos];

	//---------------------------------------------Para que comience a funcionar debe hacer un ciclo con todos los archivos-----------------

	for(;j<cantidadArchivos;j++){
		file[j] = fopen (archivos[j],"r");
		info[j] = malloc(500);
		fgets(info[j],500,file[j]);
		if (flag){
			strcpy(menor,info[j]); //Es la inicializacion de menor
			flag = 0;
			}
		comparacion = strcmp(menor, info[j]);
		if(comparacion >= 0){
			strcpy(menor, info[j]);
			pos = j;
		}
	}

	//---------------------------------------------Cuando quedan 2 o mas archivos x analizar--------------------------------------------------

	while(contador < (cantidadArchivos - 1)){
	int i = 0;
	for(;i<cantidadArchivos;i++){
		if(file[i] != 0){ //Para no trabajar sobre archivos cerrados
		comparacion = strcmp(menor, info[i]); //Compara los strings (-1 -> 1ro menor, 1 -> 2do menor, 0 -> iguales)
			if(comparacion >= 0){
				strcpy(menor, info[i]);
				pos = i;
			}
		}
	}
	fgets(info[pos],500,file[pos]); //Avanza sobre el archivo con el menor dato
	//printf("%s",menor); //Imprime el menor
	//fwrite(menor,sizeof(char),500,archivoApareado);
	fprintf(archivoApareado,menor);
	memset(menor,0,500); //Para poner el arreglo 0
	if(feof(file[pos])){
		fclose(file[pos]);
		free(info[pos]);
		file[pos] = 0; //Para saber cuando se cierra un archivo
		contador++; //Cuenta la cantidad de archivos leidos completamente
		for(i = 0;i<cantidadArchivos;i++){ //Para obtener un nuevo valor si un archivo se termino pero todavia quedan 2 o mas
				if(file[i] != 0){
					if(contador == (cantidadArchivos - 1)) break; //Ya que si es el ultimo archivo, no quiero que lea
					fgets(info[i],500,file[i]);
					strcpy(menor, info[i]);
					break; //Una vez que obtiene un valor sale
				}
		}
	}else{
			strcpy(menor, info[pos]); //Setea un nuevo menor para la proxima comparacion
		}
	}

	//---------------------------------------------Para cuando queda un unico archivo x analizar--------------------------------------------------

	for(;k<cantidadArchivos;k++){
		if(file[k] != 0){
			while(!feof(file[k])){
				//printf("%s\n",info[k]);
				//fwrite(info[k],sizeof(char),500,archivoApareado);
				fprintf(archivoApareado,info[k]);
				fgets(info[k],500,file[k]);
			}fclose(file[k]);
			free(info[k]);
		}
	}
	free(menor);
	fclose(archivoApareado);
}
