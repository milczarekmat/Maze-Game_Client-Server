#ifndef GRA_PROJEKT_SOCKET_SERVER_H
#define GRA_PROJEKT_SOCKET_SERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <ncurses.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "server_defs.h"
//#include "queue.h"

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;
void init_server_socket();
void * listener(void* arg);

#endif
