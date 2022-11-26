#include "socket_server.h"
//#include "queue_server.h"

#define PORT 8080
#define BACKLOG 10
#define POOL 4

void init_server_socket(GAME *game){
    SA_IN server_address;
    int option = 1;
    game->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(game->socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if (game->socket_fd == -1) {
        endwin();
        perror("Failed to create server socket\n");
        free_game(&game);
        exit(1);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(PORT);
    if (bind(game->socket_fd, (SA*)&server_address, sizeof(server_address)) == -1){
        endwin();
        perror("Error with binding socket\n");
        free_game(&game);
        exit(1);
    }
    if ((listen(game->socket_fd, BACKLOG)) == -1) {
        endwin();
        perror("Listen failed\n");
        free_game(&game);
        exit(1);
    }
/*    else{
        mvprintw(20, WIDTH + (11), "list");
    }*/
    pthread_create(&game->listener, NULL, &listener, game);
}

void * listener(void* arg){
    GAME *game = (GAME *)arg;
    SA_IN client_address;
    int client_socket, address_size = sizeof(SA_IN);
    while(true) {
        client_socket = accept(game->socket_fd, (SA *) &client_address, (socklen_t *) &address_size);
        if (client_socket == -1) {
            perror("Accept failed\n");
            continue;
        }
        bool flag = false;
        pthread_mutex_lock(&game->players_mutex);
        if (game->number_of_players >= 4){
            //printw("Full server\n");
            shutdown(client_socket, SHUT_RDWR);
            close(client_socket);
            //printw("Closed connection\n");
            refresh();
            flag = true;
            //pthread_mutex_unlock(&clients_m);
            //continue;
        }
        pthread_mutex_unlock(&game->players_mutex);

        if (flag){
            continue;
        }

        spawn_player(game, &client_socket);
    }
}
