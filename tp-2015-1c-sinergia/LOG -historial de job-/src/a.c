/*
 ============================================================================
 Name        : LOG
 Author      : Javier Zolotarchuk
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>

void rutinaFinalizoCorrectamente(int resultado);

int main(void) {


int resultado=0; //valor Dammy, en realidad va a ser provisto por el job


rutinaFinalizoCorrectamente(resultado);

}


void rutinaFinalizoCorrectamente(int resultado){
if (resultado == 0){

	t_log_level nivel;
	t_log* archivoDeLog;
	nivel = LOG_LEVEL_INFO;
	archivoDeLog = log_create("/home/utnso/Escritorio/log.txt", "LaMartus", 0, nivel);
	log_info(archivoDeLog,"salio ok map/reduce");

}else{

	t_log_level nivel;
	t_log* archivoDeLog;
	nivel = LOG_LEVEL_ERROR;
	archivoDeLog = log_create("/home/utnso/Escritorio/log.txt", "LaMartus", 0, nivel);
	log_error(archivoDeLog,"fallo map/reduce");
}
}




