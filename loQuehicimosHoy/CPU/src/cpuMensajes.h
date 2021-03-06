/*
 * cpuMensajes.h
 *
 *  Created on: 1/9/2015
 *      Author: utnso
 */
#ifndef MENSAJES_H_
#define MENSAJES_H_

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

char idProceso;
int planificadorSocket;
char* puertoPlanificador;
char* ipSwap;
char* puertoAdminMemoria;

int conectarAPlanificador();

void mandarCadena(int socket,char* cadena);
char* recibirCadena(int socketQueEscribe);
#endif
