/*
 ============================================================================
 Name        : jugando.c
 Author      : Javier Zolotarchuk
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commons/txt.h>
#include <commons/string.h>
#include <stdbool.h>


void conseguiRutaArchivos(t_dictionary* undiccionario);

int main(void) {


t_dictionary* diccionario;
diccionario = dictionary_create();
char* dato;
char* clave;
char* textoLeido;
FILE* archivo;
archivo = fopen("/home/utnso/workspace/extraer datos de archivo de configuracion/src/texto","r");

while (feof(archivo) == 0){
fgets(textoLeido,50,archivo);



clave = string_split(textoLeido,",")[0];
dato = string_split(textoLeido,",")[1];
dictionary_put(diccionario,clave,dato);



}
fclose(archivo);


conseguiRutaArchivos(diccionario);
/*
char* info;
info= (char*)dictionary_get(diccionario,"ip");
printf("%s",info);

*/
printf(dictionary_get(diccionario,"ip"));	//cada dato tiene que ser cargado en una variable en ves de imprimirse, ej: ipMarta = dictionary_get(diccionario,"ip");
printf(dictionary_get(diccionario,"puerto"));
printf(dictionary_get(diccionario,"mapper"));
printf(dictionary_get(diccionario,"reducer"));
printf(dictionary_get(diccionario,"combiner"));
printf(dictionary_get(diccionario,"resultado"));

}

void conseguiRutaArchivos(t_dictionary* diccionario) {
	int x = 0;
	int i = 1;
	bool existe;


	while (x == 0) {

	char *unaPalabra = string_new();
		 string_append(&unaPalabra, "archivo");
		 string_append(&unaPalabra, string_itoa(i));

		 existe = dictionary_has_key(diccionario,unaPalabra);

		if (existe == 1) {
			printf(dictionary_get(diccionario,unaPalabra));   //cada ruta de archivo tiene que ser cargada en una lista en ves de imprimirse.
			i++;
		}else{
			x=1;
		}
	}

}



