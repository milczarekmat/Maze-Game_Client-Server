#include "server_threads.h"
#include <ncurses.h>

void * tick(void * arg){
    GAME *game = (GAME *)arg;
    while (true){
        for (int i=0; i<game->number_of_players; i++){
            pthread_mutex_lock(&(game->players + i)->player_mutex);
            show_players_info(game);
            pthread_mutex_unlock(&(game->players + i)->player_mutex);
        }

        usleep(300000);
        generate_map(game); // muteks mapy
        for (int i=0; i<game->number_of_players; i++){
            // TODO wait cond for bushes
            pthread_mutex_lock(&(game->players + i)->player_mutex);
//            if ((game->players + i)->in_bush){
//                (game->players + i)->out_bush = true;
//            }
            if ((game->players + i)->bush_status > 1){
                (game->players + i)->bush_status -= 1;
            }
            (game->players + i)->already_moved = false;
            if ((game->players + i)->bush_status == 1){
                pthread_cond_signal(&(game->players + i)->move_wait);
            }
            pthread_mutex_unlock(&(game->players + i)->player_mutex);
        }
        // TODO to samo dla bestii
        (game->rounds)++;
    }

}