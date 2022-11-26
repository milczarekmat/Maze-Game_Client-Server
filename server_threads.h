#ifndef GRA_PROJEKT_SERVER_THREADS_H
#define GRA_PROJEKT_SERVER_THREADS_H
#include "server_defs.h"
#include <stdbool.h>
#include <unistd.h>
#include "socket_server.h"

void * tick(void * arg);
void * beast_thread(void * arg);
void * player_thread(void * arg);

#endif
