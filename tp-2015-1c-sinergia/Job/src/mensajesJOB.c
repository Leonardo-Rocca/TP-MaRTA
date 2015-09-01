/*
 * mensajes.c
 *
 *  Created on: 26/6/2015
 *      Author: utnso
 */

#include "mensajesJOB.h"
/*

void guardarParametrosDeConfiguracion(char* resultado, char* resultadoFinal, char* parametro, char dato[50], FILE* archivo){


		char* preResultado;
		char* coma;
		coma = string_new();
		preResultado = string_new();

		coma = ",";

		string_append(&preResultado,parametro);
		string_append(&preResultado,coma);  //le pone la coma al parametro por EJ puerto => puerto,

		string_append(&resultado,preResultado);
		string_append(&resultado,dato);						//concatena lo del paso anterior con el dato ingresado por EJ puerto, => puerto,4500


		string_append(&resultadoFinal,"\n");
		string_append(&resultadoFinal,resultado);       //le agrega el \n al final para que printf escriba y avance a la linea siguiente y no siga escribiendo todo pegado

		fprintf(archivo, "%s", resultadoFinal);

}

int cargarArchivoDeConfiguracion(void) {

	char dato[50];
	//char* dato;
	char* resultado;
	char* resultadoFinal;
	char* parametro;
	FILE* archivo;

	resultado = string_new();
	resultadoFinal = string_new();

	//archivo = fopen("/home/utnso/workspace/guardar datos en archivo de configuracion/src/textoConfiguracion","a");

	archivo = fopen("/home/utnso/archConfigJob","a"); //si no existe lo crea solo.
	fprintf(archivo, "comienzo,comienzo"); //IMPORTANTE!! para que haya una 1ra linea de texto cuando se crea el archivo ya que sino la primera cargada de datos sale erronea.


	printf("por favor ingrese la direccion de IP de MARTA: ");
		parametro= "ip";
		scanf("%s",&dato);
		guardarParametrosDeConfiguracion(resultado,resultadoFinal,parametro,dato,archivo);

	printf("por favor ingrese el puerto de MARTA: ");
		parametro= "puerto";
		scanf("%s",&dato);
		guardarParametrosDeConfiguracion(resultado,resultadoFinal,parametro,dato,archivo);


fclose(archivo);

return 0;
}


*/

