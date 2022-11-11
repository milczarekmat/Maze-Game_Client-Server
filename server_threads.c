#include "server_threads.h"


void * tick(void * arg){
    GAME *game = (GAME *)arg;
    while (true){
        sleep(1);
        generate_map(game);
        show_players_info(game);
        for (int i=0; i<game->number_of_players; i++){
            (game->players + i)->already_moved = false;
        }
        (game->rounds)++;
        // TODO to samo dla bestii
    }

}