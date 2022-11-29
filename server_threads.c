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
        }

        usleep(600000);
        generate_map(game);

        // TODO CZY MUTEKS PLAYERS JEST POTRZEBNY?
        pthread_mutex_lock(&game->players_mutex);
        for (int i=0; i<game->number_of_players; i++){
            // TODO wskaznik player do zrobienia
            PLAYER* player = game->players[i];
            pthread_mutex_lock(&player->player_mutex);
            if (player->bush_status > 1){
                player->bush_status -= 1;
            }
            player->already_moved = false;
            send_player_information(game, (game->players[i]));

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
            if ((abs(beast->x_to_player) <= 1 && abs(beast->y_to_player) == 0) ||
            (abs(beast->y_to_player) <= 1 && abs(beast->x_to_player) == 0)){
                beast->available_kill = true;
            }
            move_beast(direct, game, beast);

        }
        else {
            int n;
            enum DIRECTION* available_directions = check_available_directions(game, beast_x, beast_y, &n);
            if (beast->coming_until_wall) {
                move_beast(beast->last_direction, game, beast);
            }
            else {
                enum DIRECTION direct = rand_direction_for_beast_move(n, available_directions, beast->opposite_direction);
                move_beast(direct, game, beast);
            }
        }
        for (int i=0; i<game->number_of_players; i++){
            send_player_information(game, (game->players[i]));
        }
    }
}

void * player_thread(void * arg){
    GAME *game = (GAME *) arg;
    pthread_mutex_lock(&game->players_mutex);
    PLAYER* player = game->players[game->number_of_players];
    (game->number_of_players)++;
    pthread_mutex_unlock(&game->players_mutex);
    int *player_fd = player->file_descriptor;
    char signal_from_player;
    while (true) {
        long check = recv(*player_fd, &signal_from_player, sizeof(char), 0);
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
        // BRAK MUTEKSU MAIN I PLAYER W SEND
        send_player_information(game, player);
    }
    game->number_of_players--;
    return NULL;
}
