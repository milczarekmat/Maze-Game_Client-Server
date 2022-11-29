#include "server_threads.h"
#include "beast.h"

void * tick(void * arg){
    GAME *game = (GAME *)arg;
    while (true){
        // TODO PROBLEM PONIZEJ
        for (int i=0; i<game->number_of_players; i++){
            pthread_mutex_lock(&game->players[i]->player_mutex);
            show_players_info(game);
            pthread_mutex_unlock(&game->players[i]->player_mutex);
            //send_player_information(game, (game->players + i));
        }

        usleep(1000000);
        generate_map(game);

        refresh();
        // TODO CZY MUTEKS PLAYERS JEST POTRZEBNY?
        pthread_mutex_lock(&game->players_mutex);
        for (int i=0; i<game->number_of_players; i++){
/*            move(24 + i, WIDTH + (10));
            clrtoeol();
            mvprintw(24 + i, WIDTH + (10), "Done");*/
            // TODO wskaznik player do zrobienia
            PLAYER* player = game->players[i];
            pthread_mutex_lock(&player->player_mutex);
//            if ((game->players + i)->in_bush){
//                (game->players + i)->out_bush = true;
//            }
            if (player->bush_status > 1){
                player->bush_status -= 1;
            }

            //todo do funkcji copy!
            player->already_moved = false;
/*            SEND_DATA data;
            data.x = player->x_position;
            data.y = player->y_position;
            data.carried = player->carried;
            data.brought = player->brought;

            data.game_round = game->rounds;
            for (int i = -2, k = 0; i <= 2; i++, k++) {
                for (int j = -2, l = 0; j <= 2; j++, l++) {
                    if (check_if_border_y_exceeded(player->y_position + i)
                        || check_if_border_x_exceeded(player->x_position + j)) {
                        data.player_map[k][l] = 'a';
                    } else {
                        data.player_map[k][l] = game->map[player->y_position + i][player->x_position + j];
                    }
                }
            }
            long check = send(*player->file_descriptor, &data, sizeof(data), 0);*/
            //send_player_information(game, (game->players + i));

            // TODO BYLA ZMIANA KOLEJNOSCI UNLOCK I SIGNAL
            pthread_mutex_unlock(&player->player_mutex);
            if (player->bush_status == 1){
                pthread_cond_signal(&player->bush_wait);
            }
        }
        pthread_mutex_unlock(&game->players_mutex);
        // TODO CZY MUTEKS PONIZEJ GRY JEST POTRZEBNY?
        pthread_mutex_lock(&game->beasts_mutex);
        for (int i=0; i<game->number_of_beasts; i++){
            BEAST *beast = game->beasts[i];
            pthread_mutex_lock(&beast->beast_mutex);
            beast->already_moved = false;
            // TODO BYLA ZMIANA KOLEJNOSCI 2 LINIJEK PONIZE
            pthread_mutex_unlock(&beast->beast_mutex);
            pthread_cond_signal(&beast->move_wait);
        }
        pthread_mutex_unlock(&game->beasts_mutex);
        pthread_mutex_lock(&game->main_mutex);
        (game->rounds)++;
        pthread_mutex_unlock(&game->main_mutex);
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
        if (beast->seeing_player) {
            enum DIRECTION direct = check_if_chase_available(game, beast, beast_x, beast_y, x_to_player, y_to_player);
/*            pthread_mutex_lock(&game->main_mutex);
            pthread_mutex_unlock(&game->main_mutex);*/
            // TODO POPRAWIC
            if ((abs(beast->x_to_player) <= 1 && abs(beast->y_to_player) == 0) ||
            (abs(beast->y_to_player) <= 1 && abs(beast->x_to_player) == 0)){
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
                move_beast(direct, game, beast);
            }
        }
/*        for (int i=0; i<game->number_of_players; i++){
            send_player_information(game, (game->players + i));
        }*/
    }
}

void * player_thread(void * arg){
    GAME *game = (GAME *) arg;
    pthread_mutex_lock(&game->players_mutex);
    PLAYER* player = game->players[game->number_of_players];
    //int counter = game->number_of_players;
    (game->number_of_players)++;
    pthread_mutex_unlock(&game->players_mutex);
    int *player_fd = player->file_descriptor;
    SEND_DATA data;
    char signal_from_player;
    while (true) {
/*        pthread_mutex_lock(&game->main_mutex);
        mvprintw(25+ counter, WIDTH + counter*(10), "player id: %d", player->id);
        refresh();
        pthread_mutex_unlock(&game->main_mutex);*/
        //pthread_mutex_lock(&game->main_mutex);
        //pthread_mutex_lock(&player->player_mutex);
        memset(&data, 0, sizeof(struct send_data_t));
        data.x = player->x_position;
        data.y = player->y_position;
        data.carried = player->carried;
        data.brought = player->brought;
        data.id = player->id;

        data.game_round = game->rounds;
        for (int i = -2, k = 0; i <= 2; i++, k++) {
            for (int j = -2, l = 0; j <= 2; j++, l++) {
                if (check_if_border_y_exceeded(player->y_position + i)
                    || check_if_border_x_exceeded(player->x_position + j)) {
                    data.player_map[k][l] = 'a';
                } else {
                    data.player_map[k][l] = game->map[player->y_position + i][player->x_position + j];
                }
            }
        }
        long check = send(*player_fd, &data, sizeof(data), 0);
/*        if (check == -1) {
            endwin();
            free_game(&game);
            exit(9);
        }*/
        //pthread_mutex_unlock(&player->player_mutex);
        //pthread_mutex_unlock(&game->main_mutex);

        check = recv(*player_fd, &signal_from_player, sizeof(char), 0);
        if (signal_from_player == 'q'){
            mvprintw(22, WIDTH + (10), "quited");
            refresh();
            break;
            //pthread_cancel(*game->players_threads + player->id - 1);
        }
        else if (signal_from_player == 'w'){

            move_player(UP, game, player->id);
        }
        else if (signal_from_player == 's'){
            move_player(DOWN, game, player->id);
        }
        else if (signal_from_player == 'a'){
            move_player(LEFT, game, player->id);
        }
        else if (signal_from_player == 'd'){
            move_player(RIGHT, game, player->id);
        }
        //todo switch
    }
    game->number_of_players--;
    return NULL;
}





/*
void * getch_thread(void * arg){
    GAME *game = (GAME *) arg;

    while (true){
        nodelay(stdscr, FALSE);
        int ch = getch();
        pthread_cond_signal()
    }
}
*/
