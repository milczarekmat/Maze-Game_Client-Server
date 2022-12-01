#include "server_threads.h"
#include "beast.h"

void * tick(void * arg){
    GAME *game = (GAME *)arg;
    while (true){
        for (int i=0; i<game->number_of_players; i++){
            pthread_mutex_lock(&game->players[i]->player_mutex);
            show_players_info(game);
            pthread_mutex_unlock(&game->players[i]->player_mutex);
            send_player_information(game, (game->players[i]));
        }

        usleep(600000);
        generate_map(game);

        pthread_mutex_lock(&game->players_mutex);
        for (int i=0; i<game->number_of_players; i++){
            PLAYER* player = game->players[i];
            pthread_mutex_lock(&player->player_mutex);
            if (player->bush_status > 1){
                player->bush_status -= 1;
            }
            player->already_moved = false;
            pthread_mutex_unlock(&player->player_mutex);
            if (player->bush_status == 1){
                pthread_cond_signal(&player->bush_wait);
            }
        }
        pthread_mutex_unlock(&game->players_mutex);

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
    while (true) {
        pthread_mutex_lock(&beast->beast_mutex);
        if (beast->already_moved){
            pthread_cond_wait(&beast->move_wait, &beast->beast_mutex);
        }
        pthread_mutex_unlock(&beast->beast_mutex);

        check_beast_vision(game, beast);
        if (beast->seeing_player) {
            enum DIRECTION direct = check_if_chase_available(game, beast, beast->x_position, beast->y_position, beast->x_to_player, beast->y_to_player);
            if ((abs(beast->x_to_player) <= 1 && abs(beast->y_to_player) == 0) ||
            (abs(beast->y_to_player) <= 1 && abs(beast->x_to_player) == 0)){
                beast->available_kill = true;
            }
            move_beast(direct, game, beast);

        }
        else {
            int n;
            enum DIRECTION* available_directions = check_available_directions(game, beast->x_position, beast->y_position, &n);
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
    game->number_of_players++;
    pthread_mutex_unlock(&game->players_mutex);
    int *player_fd = player->file_descriptor;
    char signal_from_player;
    while (true) {
        long check = recv(*player_fd, &signal_from_player, sizeof(char), 0);
        if (check == 0){
            break;
        }

        if (signal_from_player == 'q'){
            break;
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
        send_player_information(game, player);
    }
    delete_player(game, player);
    pthread_mutex_lock(&game->players_mutex);
    game->number_of_players--;
    pthread_cond_signal(&game->connection_wait);
    pthread_mutex_unlock(&game->players_mutex);
    return NULL;
}
