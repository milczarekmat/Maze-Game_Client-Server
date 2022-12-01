#include "socket_server.h"

#define PORT 8080
#define BACKLOG 10

SA_IN server_address;
SA_IN client_address;
void init_server_socket(GAME *game){
    int option = 1;
    game->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(game->socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if (game->socket_fd == -1) {
        endwin();
        perror("Failed to create server socket\n");
        free_game(&game);
        exit(1);
    }
    memset(&server_address, 0, sizeof(server_address));
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

    pthread_create(&game->listener, NULL, &listener, game);
}

void * listener(void* arg){
    GAME *game = (GAME *)arg;

    int client_socket, address_size = sizeof(SA_IN);
    while(true) {
        pthread_mutex_lock(&game->players_mutex);
        if (game->number_of_players >= 2){
            //char full_server_signal = 'f';
//            long check = send(client_socket, &full_server_signal, sizeof(char), 0);
            //shutdown(client_socket, SHUT_RDWR);
            //close(client_socket);
            //mvprintw(28, WIDTH+5, "Done");
            pthread_cond_wait(&game->connection_wait, &game->players_mutex);
            //init_server_socket(game);
        }
        pthread_mutex_unlock(&game->players_mutex);

        client_socket = accept(game->socket_fd, (SA *) &client_address, (socklen_t *) &address_size);
        if (client_socket == -1) {
            perror("Accept failed\n");
            continue;
        }

/*        else{
            char ready_server_signal = 'r';
            long check = send(client_socket, &ready_server_signal, sizeof(char), 0);
        }*/

        spawn_player(game, &client_socket);
    }
}
