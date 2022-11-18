#include "server_threads.h"
#include "beast.h"

void * tick(void * arg){
    GAME *game = (GAME *)arg;
    while (true){
        // TODO PROBLEM PONIZEJ
        for (int i=0; i<game->number_of_players; i++){
            pthread_mutex_lock(&(game->players + i)->player_mutex);
            show_players_info(game);
            pthread_mutex_unlock(&(game->players + i)->player_mutex);
        }

        usleep(800000);
        generate_map(game);
        // TODO CZY MUTEKS PLAYERS JEST POTRZEBNY?
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
        // TODO CZY MUTEKS PONIZEJ GRY JEST POTRZEBNY?
        pthread_mutex_lock(&game->beasts_mutex);
        for (int i=0; i<game->number_of_beasts; i++){
            BEAST *beast = game->beasts[i];
            pthread_mutex_lock(&beast->beast_mutex);
            beast->already_moved = false;
            pthread_mutex_unlock(&beast->beast_mutex);
        }
        pthread_mutex_unlock(&game->beasts_mutex);
        (game->rounds)++;
    }
}

void * beast_thread(void * arg) {
    GAME *game = (GAME *) arg;
    pthread_mutex_lock(&game->beasts_mutex);
    BEAST *beast = game->beasts[game->number_of_beasts];
    (game->number_of_beasts)++;
    pthread_mutex_unlock(&game->beasts_mutex);
    // TODO MUTEKS?
    while (true) {
        //pthread_mutex_lock(&game->beasts_mutex);
        pthread_mutex_lock(&beast->beast_mutex);
        if (beast->already_moved){
            pthread_mutex_unlock(&beast->beast_mutex);
            continue;
        }
        pthread_mutex_unlock(&beast->beast_mutex);

        check_beast_vision(game, beast);
        pthread_mutex_lock(&beast->beast_mutex);
        unsigned int beast_x = beast->x_position, beast_y = beast->y_position;
        pthread_mutex_unlock(&beast->beast_mutex);
        if (beast->seeing_player) {
            pthread_mutex_lock(&game->map_mutex);
            move(22, WIDTH + (10));
            clrtoeol();
            mvprintw(22, WIDTH + (10), "Seeing player: %d", beast->seeing_player);
            pthread_mutex_unlock(&game->map_mutex);
        }
        else {
            if (beast->coming_until_wall) {
                move_beast(beast->last_direction, game, beast);
            }
            else {
                int n;
                enum DIRECTION* available_directions = check_available_directions(game, beast_x, beast_y, &n);
                enum DIRECTION direct = rand_direction_for_beast_move(n, available_directions);
                pthread_mutex_lock(&game->map_mutex);

                move(22, WIDTH + (10));
                clrtoeol();
                mvprintw(22, WIDTH + (10), "Seeing player: %d", beast->seeing_player);

                move(26, WIDTH + (10));
                clrtoeol();
                mvprintw(26, WIDTH + (10), "Next direction: %d", direct);
                pthread_mutex_unlock(&game->map_mutex);

                if (direct != STAY){
                move_beast(direct, game, beast);
                }
            }
        }
        //pthread_mutex_unlock(&game->beasts_mutex);
    }
}
