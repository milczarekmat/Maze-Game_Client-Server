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

        usleep(1000000);
        generate_map(game);
        // TODO CZY MUTEKS PLAYERS JEST POTRZEBNY?
        pthread_mutex_lock(&game->players_mutex);
        for (int i=0; i<game->number_of_players; i++){
            // TODO wskaznik player do zrobienia
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
            pthread_cond_signal(&beast->move_wait);
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
        pthread_mutex_lock(&beast->beast_mutex);
        if (beast->already_moved){
            pthread_cond_wait(&beast->move_wait, &beast->beast_mutex);
        }
        pthread_mutex_unlock(&beast->beast_mutex);

        check_beast_vision(game, beast);
        pthread_mutex_lock(&beast->beast_mutex);
        unsigned int beast_x = beast->x_position, beast_y = beast->y_position;
        int x_to_player = beast->x_to_player, y_to_player = beast->y_to_player;
        pthread_mutex_unlock(&beast->beast_mutex);
/*        pthread_mutex_lock(&game->map_mutex);
        move(20, WIDTH + (10));
        clrtoeol();
        move(22, WIDTH + (10));
        clrtoeol();
        move(24, WIDTH + (10));
        clrtoeol();
        pthread_mutex_unlock(&game->map_mutex);*/

        if (beast->seeing_player) {
            enum DIRECTION direct = check_if_chase_available(game, beast, beast_x, beast_y, x_to_player, y_to_player);
            pthread_mutex_lock(&game->map_mutex);
            move(20, WIDTH + (10));
            clrtoeol();
            move(22, WIDTH + (10));
            clrtoeol();
            mvprintw(20, WIDTH + (10), "x_to_player: %d", beast->x_to_player);
            mvprintw(22, WIDTH + (10), "y_to_player: %d", beast->y_to_player);
            pthread_mutex_unlock(&game->map_mutex);
            // TODO POPRAWIC
            if ((abs(beast->x_to_player) <= 1 && abs(beast->y_to_player) == 0) ||
            (abs(beast->y_to_player) <= 1 && abs(beast->x_to_player) == 0)){

                mvprintw(24, WIDTH + (10), "Done flag avail");
                beast->available_kill = true;
            }
            move_beast(direct, game, beast);

        }
        else {
            int n;
            enum DIRECTION* available_directions = check_available_directions(game, beast_x, beast_y, &n);
/*            int offset_x = beast_x, offset_y = beast_y;
            int temp_x = 0, temp_y = 0;
            if (beast->last_direction == UP || beast->last_direction == DOWN){
                temp_x = 1;
            }
            else if (beast->last_direction == LEFT || beast->last_direction == RIGHT){
                temp_y = 1;
            }
            offset_adaptation(beast->last_direction, &offset_y, &offset_x);

            pthread_mutex_lock(&game->map_mutex);
            if (n >= 4 && game->map[offset_y + temp_y][offset_x + temp_x] == 'a' &&
            game->map[offset_y - temp_y][offset_x - temp_x] == 'a'){
                beast->coming_until_wall = false;
            }
            pthread_mutex_unlock(&game->map_mutex);*/

            if (beast->coming_until_wall) {
                move_beast(beast->last_direction, game, beast);
            }
            else {
                enum DIRECTION direct = rand_direction_for_beast_move(n, available_directions, beast->opposite_direction);
                 //pthread_mutex_lock(&game->map_mutex);

                 //move(22, WIDTH + (10));
                 //clrtoeol();
                 //mvprintw(22, WIDTH + (10), "Opposite direct: %d", beast->opposite_direction);

                 //move(24, WIDTH + (10));
                 //clrtoeol();
                 //mvprintw(24, WIDTH + (10), "Avail kill %d", beast->available_kill);
                 //pthread_mutex_unlock(&game->map_mutex);

                //if (direct != STAY){
                move_beast(direct, game, beast);
                //}
            }
        }
    }
}
