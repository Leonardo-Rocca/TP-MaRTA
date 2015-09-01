/*
 * AdminSwapMensajes.h
 *
 *  Created on: 1/9/2015
 *      Author: utnso
 */

#ifndef MENSAJES_H_
#define MENSAJES_H_

#include <time.h>
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


t_list* socketsConectados;

void* manejoConexionesConCPUs(void* parametro);
int recibirMensajeDe(int);
void *manejoHiloAdminMemoria(void *socketCli);

void mandarCadena(int socket,char* cadena);
char* recibirCadena(int socketQueEscribe);

#endif /* MENSAJES_H_ */

