/*
 * redireccionarVersion2.c
 *
 *  Created on: 11/7/2015
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mensajesNodo.h"

//-------------------------------------------------------------------------------------------------
int escribirEnArchivo(FILE *archivo, char* rutaEntrada)
{
	printf("esta entrando a la funcion\n");
	int fd_archivo = open(rutaEntrada,O_RDONLY); //agregar al archivo de config!!
		char* mapeo;
		int tamanioArchivo = tamanio_archivo(fd_archivo);

		if( (mapeo = mmap(NULL, tamanioArchivo, PROT_READ, MAP_SHARED, fd_archivo, 0 )) == MAP_FAILED){
			//Si no se pudo ejecutar el MMAP, imprimir el error y aborta;
			printf("Error con MMAP");
			//abort();
			return -1;
		}
	if(fprintf(archivo, mapeo)==-1){
		printf("FALLO EL PRINTF, es decir fallo el programa\n");
		return -1;
	}
	munmap(mapeo,tamanioArchivo);
	close(fd_archivo);
	return 1;
}


//-------------------------------------------------------------------------------------------------

int llamada_al_programa_redireccionando_stdin_out_ordenando(char *pathPrograma, char *pathArchivoSalida, int (*escribirArchivoEntrada) (void*, void*), char* rutaEntrada, int queFuncionEs)


{
	FILE *entradaARedirigir = NULL;

	char* comandoEntero = malloc(strlen(pathPrograma)+11+strlen(pathArchivoSalida));
	/*char* rutaEntera = malloc(strlen(pathPrograma)+1+strlen("/home/utnso/Escritorio/Nodos/Nodo2/Debug/")+1);
	strcpy(rutaEntera,"/home/utnso/Escritorio/Nodos/Nodo2/Debug/");
	string_append(&rutaEntera,pathPrograma);

	printf("rutaEntera: %s", rutaEntera);*/


	sprintf(comandoEntero,"./%s | sort > %s",pathPrograma,pathArchivoSalida);

	printf("pasa esto \n");


	if(queFuncionEs){
		sprintf(comandoEntero,"./%s > %s",pathPrograma,pathArchivoSalida);
	}else{
		sprintf(comandoEntero,"./%s | sort > %s",pathPrograma,pathArchivoSalida);
	}
	entradaARedirigir = popen (comandoEntero,"w");

	if (entradaARedirigir != NULL)
	{

		escribirArchivoEntrada(entradaARedirigir,rutaEntrada);
		int resultadoDeEscribirArchivo = escribirArchivoEntrada(entradaARedirigir,rutaEntrada);
		//loguearEscrituraDeEspacioTemporal();
		pclose (entradaARedirigir);

		if(resultadoDeEscribirArchivo==-1)return -1;
		//free(comandoEntero);
	}
	else
	{
		printf("No se pudo ejecutar el programa!");
	//	loguearFalloDeEscrituraDeEspacioTemporal();

		return -1;
	}


	return 0;
}


//-------------------------------------------------------------------------------------------------

//NOTA IMPORTANTE:
///////////////////////
//MAIN PARA PROBAR. COMO ESTA DENTRO DEL PROYECTO, DEBEN COMENTAR EL MAIN DE NODO.C
/*
int main(void) {
	puts("Hola Soy un programa que redirecciona el stdin y el stdout para un cierto programa!!!\n");

	char* ruta = "/home/utnso/Escritorio/nuevo2";

	llamada_al_programa_redireccionando_stdin_out_ordenando("/home/utnso/git/tp-2015-1c-sinergia/Nodo/mapper.sh","salida.txt",(void *) escribirEnArchivo, ruta);

	puts("End\n");
	return EXIT_SUCCESS;
}*/


