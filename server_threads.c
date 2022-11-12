#include "server_threads.h"

void * tick(void * arg){
    GAME *game = (GAME *)arg;
    while (true){
        usleep(700000);
        generate_map(game);
        show_players_info(game);
        for (int i=0; i<game->number_of_players; i++){
            pthread_mutex_lock(&(game->players + i)->player_mutex);
            (game->players + i)->already_moved = false;
            pthread_mutex_unlock(&(game->players + i)->player_mutex);
        }
        // TODO to samo dla bestii
        (game->rounds)++;
    }

}