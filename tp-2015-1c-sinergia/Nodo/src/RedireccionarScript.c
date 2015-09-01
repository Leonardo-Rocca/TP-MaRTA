/*
 ============================================================================
 Name        : RedireccionarScript.c
 Author      : Fede
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "mensajesNodo.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define NUM_PIPES          2

#define PARENT_WRITE_PIPE  0
#define PARENT_READ_PIPE   1

int pipes[NUM_PIPES][2];

#define READ_FD  0
#define WRITE_FD 1

#define PARENT_READ_FD  ( pipes[PARENT_READ_PIPE][READ_FD]   )
#define PARENT_WRITE_FD ( pipes[PARENT_WRITE_PIPE][WRITE_FD] )

#define CHILD_READ_FD   ( pipes[PARENT_WRITE_PIPE][READ_FD]  )
#define CHILD_WRITE_FD  ( pipes[PARENT_READ_PIPE][WRITE_FD]  )


void redireccionarScript(int fdEntrada, int fdSalida, char* rutaScript){


	pipe(pipes[PARENT_READ_PIPE]); //donde escribe el script
	pipe(pipes[PARENT_WRITE_PIPE]); //donde lee el script

	if(!fork())
	{
		char *parametros[]={ rutaScript, "-q", 0};

		dup2(CHILD_READ_FD, STDIN_FILENO); //direcciona entrada de script
		dup2(CHILD_WRITE_FD, STDOUT_FILENO);//direcciona salida de script

		//cierro pipes que no utiliza el proceso hijo
		close(CHILD_READ_FD);
		close(CHILD_WRITE_FD);
		close(PARENT_READ_FD);
		close(PARENT_WRITE_FD);

		//ejecuto script
		execv(parametros[0], parametros);

	}
	else
	{
		char buffer[400];

		int numeroBytesEscritosPorElScript;

		int numeroBytesLeidosDeEntrada;

		int tamanioBufferEntrada = tamanioArchivo(fdEntrada);

		char bufferEntrada[tamanioBufferEntrada + 1];//el tamaño de este buffer debe ser del tamaño del archivo de entrada

		numeroBytesLeidosDeEntrada = read(fdEntrada, bufferEntrada, tamanioBufferEntrada);

		if(numeroBytesLeidosDeEntrada > 0){

			bufferEntrada[numeroBytesLeidosDeEntrada]='\n';
			write(PARENT_WRITE_FD, bufferEntrada , numeroBytesLeidosDeEntrada + 1 );

		}//habria que ver que hacer cuando no se puede leer ninguno byte y se retorna error

		close(CHILD_READ_FD);
		close(CHILD_WRITE_FD);

		//leo lo que escribe el script en el fd lectura

		numeroBytesEscritosPorElScript = read(PARENT_READ_FD, buffer, sizeof(buffer)-1);
		        if (numeroBytesEscritosPorElScript >= 0) {
		            buffer[numeroBytesEscritosPorElScript] = 0;
		            printf("%s", buffer);
		        } else {
		            printf("IO Error\n");
		        }

		write(fdSalida,buffer,numeroBytesEscritosPorElScript);

	}
}

int tamanioArchivo(int fd){
	struct stat buf ;
	fstat(fd,&buf);
	return buf.st_size;
}
