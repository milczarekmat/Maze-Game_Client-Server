#include "server_threads.h"

void * tick(void * arg){
    GAME *game = (GAME *)arg;
    while (true){
        for (int i=0; i<game->number_of_players; i++){
            pthread_mutex_lock(&(game->players + i)->player_mutex);
            show_players_info(game);
            pthread_mutex_unlock(&(game->players + i)->player_mutex);
        }

        usleep(400000);
        generate_map(game);
        pthread_mutex_lock(&game->players_mutex);
        for (int i=0; i<game->number_of_players; i++){
            pthread_mutex_lock(&(game->players + i)->player_mutex);
//            if ((game->players + i)->in_bush){
//                (game->players + i)->out_bush = true;
//            }
            if ((game->players + i)->bush_status > 1){
                (game->players + i)->bush_status -= 1;
            }
            (game->players + i)->already_moved = false;
            pthread_cond_signal(&(game->players + i)->move_wait);
            if ((game->players + i)->bush_status == 1){
                pthread_cond_signal(&(game->players + i)->bush_wait);
            }
            pthread_mutex_unlock(&(game->players + i)->player_mutex);
        }
        pthread_mutex_unlock(&game->players_mutex);
        pthread_mutex_lock(&game->beasts_mutex);
        for (int i=0; i<game->number_of_beasts; i++){
            BEAST *beast = game->beasts + i;
            pthread_mutex_lock(&beast->beast_mutex);
            beast->already_moved = false;
            pthread_mutex_lock(&game->map_mutex);
            // TODO przeniesc to ponizej do move_beast
            game->map[beast->y_position][beast->x_position] = '*';
            switch (beast->last_direction){
                case LEFT:
                    game->map[beast->y_position][beast->x_position + 1] = beast->last_encountered_object;
                    break;
                case RIGHT:
                    game->map[beast->y_position][beast->x_position - 1] = beast->last_encountered_object;
                    break;
                case UP:
                    game->map[beast->y_position - 1][beast->x_position] = beast->last_encountered_object;
                    break;
                case DOWN:
                    game->map[beast->y_position + 1][beast->x_position] = beast->last_encountered_object;
                    break;
                case STAY:
                    break;
            }
            pthread_mutex_unlock(&beast->beast_mutex);
            pthread_mutex_unlock(&game->map_mutex);
        }
        pthread_mutex_unlock(&game->beasts_mutex);
        (game->rounds)++;
    }
}

/*void * beast_thread(void * arg) {
    GAME *game = (GAME *) arg;
    pthread_mutex_lock(&game->beasts_mutex);
    BEAST *beast = game->beasts + game->number_of_beasts;
    pthread_mutex_unlock(&game->beasts_mutex);
    // TODO MUTEKS?
    while (true) {
        if (beast->already_moved){
            continue;
        }
        check_beast_vision(game, beast);
        if (beast->seeing_player) {
            //```
        }
        else {
            if (beast->coming_until_wall) {
                move_beast(last_direction);
                // TODO pamietac o fladze coming_until
            }
            else {
                int n = sprawdz_mozliwe_kierunki(beast_x, beast_y);
                enum direction direct
                        = losuj_mozliwy_kierunek(BEAST * beast, n, mozliwe
                kierunki...)
                (uwzglednij
                kierunek
                w
                ktorym
                szla
                do tej
                pory);
                move_beast(direct);
            }
        }
    }
}*/
