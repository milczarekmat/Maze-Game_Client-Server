#include "socket_server.h"

#define PORT 8080
#define BACKLOG 10
#define POOL 4

SA_IN server_address;
SA_IN client_address;
void init_server_socket(GAME *game){

    //struct sockaddr_un serveraddr;
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
    //server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
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

    game->connections = create_queue();
    for (int i=0; i<POOL; i++){
        pthread_create(game->players_threads + i, NULL, &player_thread, game);
    }

    int client_socket, address_size = sizeof(SA_IN);
    while(true) {
        //accept(game->socket_fd, NULL, NULL)
        client_socket = accept(game->socket_fd, (SA *) &client_address, (socklen_t *) &address_size);
        if (client_socket == -1) {
            perror("Accept failed\n");
            continue;
        }
        pthread_mutex_lock(&game->main_mutex);
        mvprintw(24 + game->number_of_players, WIDTH + (10), "Connection accepted from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
        refresh();
        pthread_mutex_unlock(&game->main_mutex);
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

        pthread_mutex_lock(&game->server_mutex);

        enqueue(&client_socket, game->connections);
        pthread_cond_signal(&game->server_wait);

        pthread_mutex_unlock(&game->server_mutex);

        //spawn_player(game, &client_socket);
    }
}
