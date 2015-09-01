/*
 ============================================================================
 Name        : Prueba.c
 Author      : Fede
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void apareoDeArchivos(int cantidadArchivos , char* archivos[], char* rutaEntrada);

/*int main(){
	char* ruta1 = "/home/utnso/git/tp-2015-1c-sinergia/ApareoSC/ApareoSinCombiner/file.txt";
	char* ruta2 = "/home/utnso/git/tp-2015-1c-sinergia/ApareoSC/ApareoSinCombiner/file2.txt";
	char* ruta3 = "/home/utnso/git/tp-2015-1c-sinergia/ApareoSC/ApareoSinCombiner/file3.txt";
	char* ruta4 = "/home/utnso/git/tp-2015-1c-sinergia/ApareoSC/ApareoSinCombiner/file4.txt";

	char* archivos[4];

	archivos[0] = ruta1;
	archivos[1] = ruta2;
	archivos[2] = ruta3;
	archivos[3] = ruta4;

	char* salida = "/home/utnso/Escritorio/12345";

	apareoDeArchivos(4,archivos,salida);

	printf("End\n");

	return 0;
}*/

void apareoDeArchivos(int cantidadArchivos , char* archivos[], char* rutaEntrada){
	//---------------------------------------------Inicializaciones-------------------------------------------------------------------------

	FILE * file[200];
	int j = 0;
	int k = 0;
	int comparacion = 0;
	char menor[200]; //Estamos usando arrays para sacar la info del archivo (lo cual es malo x su tamaño predefinido)
	memset(menor,0,200); //Pongo el arreglo en 0
	int pos = 0;
	int contador = 0; //De archivos cerrados
	int flag = 1;

	FILE* archivoApareado = fopen(rutaEntrada,"w");

	struct informacion{
			char info[200];
		}informacion[cantidadArchivos];

	//---------------------------------------------Para que comience a funcionar debe hacer un ciclo con todos los archivos-----------------

	for(;j<cantidadArchivos;j++){
		file[j] = fopen (archivos[j],"r");
		fgets(informacion[j].info,200,file[j]);
		if (flag){
			strcpy(menor,informacion[j].info); //Es la inicializacion de menor
			flag = 0;
			}
		comparacion = strcmp(menor, informacion[j].info);
		if(comparacion >= 0){
			strcpy(menor, informacion[j].info);
			pos = j;
		}
	}

	//---------------------------------------------Cuando quedan 2 o mas archivos x analizar--------------------------------------------------

	while(contador < (cantidadArchivos - 1)){
	int i = 0;
	for(;i<cantidadArchivos;i++){
		if(file[i] != 0){ //Para no trabajar sobre archivos cerrados
		comparacion = strcmp(menor, informacion[i].info); //Compara los strings (-1 -> 1ro menor, 1 -> 2do menor, 0 -> iguales)
			if(comparacion >= 0){
				strcpy(menor, informacion[i].info);
				pos = i;
			}
		}
	}
	fgets(informacion[pos].info,200,file[pos]); //Avanza sobre el archivo con el menor dato
	printf("%s",menor); //Imprime el menor
	fwrite(&menor,sizeof(char),strlen(menor),archivoApareado);
	memset(menor,0,200); //Para poner el arreglo 0
	if(feof(file[pos])){
		fclose(file[pos]);
		file[pos] = 0; //Para saber cuando se cierra un archivo
		contador++; //Cuenta la cantidad de archivos leidos completamente
		for(i = 0;i<cantidadArchivos;i++){ //Para obtener un nuevo valor si un archivo se termino pero todavia quedan 2 o mas
				if(file[i] != 0){
					if(contador == (cantidadArchivos - 1)) break; //Ya que si es el ultimo archivo, no quiero que lea
					fgets(informacion[i].info,200,file[i]);
					strcpy(menor, informacion[i].info);
					break; //Una vez que obtiene un valor sale
				}
		}
	}else{
			strcpy(menor, informacion[pos].info); //Setea un nuevo menor para la proxima comparacion
		}
	}

	//---------------------------------------------Para cuando queda un unico archivo x analizar--------------------------------------------------

	for(;k<cantidadArchivos;k++){
		if(file[k] != 0){
			while(!feof(file[k])){
				printf("%s",informacion[j].info);
				fwrite(&informacion[k].info,sizeof(char),strlen(informacion[k].info),archivoApareado);
				fgets(informacion[k].info,200,file[k]);
			}fclose(file[k]);
		}
	}
}
