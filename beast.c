#include "beast.h"

int spawn_beast(GAME *game){
    pthread_mutex_lock(&game->beasts_mutex);
    game->beasts[game->number_of_beasts] = malloc(sizeof(BEAST));
    if (!game->beasts[game->number_of_beasts]){
        pthread_mutex_unlock(&game->beasts_mutex);
        main_error(ALLOCATION, &game);
    }
    BEAST *beast = game->beasts[game->number_of_beasts];
    pthread_mutex_unlock(&game->beasts_mutex);

    int x, y;
    srand(time(NULL));
    pthread_mutex_lock(&game->main_mutex);
    do{
        x = rand() % WIDTH;
        y = rand() % HEIGHT;
    }
    while(game->map[y][x] != ' ');
    game->map[y][x] = '*';
    pthread_mutex_unlock(&game->main_mutex);
    beast->x_position = x;
    beast->y_position = y;
    beast->x_to_player = x;
    beast->y_to_player = y;
    beast->already_moved = false;
    beast->seeing_player = false;
    beast->coming_until_wall = false;
    beast->available_kill = false;
    beast->last_encountered_object = ' ';

    pthread_mutex_init(&beast->beast_mutex, NULL);
    pthread_cond_init(&beast->move_wait, NULL);

    pthread_create(game->beasts_threads + game->number_of_beasts, NULL, beast_thread, game);
    generate_map(game);
    return 0;
}

void move_beast(enum DIRECTION side, GAME* game, BEAST *beast){
    int x = 0, y = 0;
    switch (side) {
        case LEFT:
            offset_adaptation(LEFT, NULL, &x);
            beast->opposite_direction = RIGHT;
            break;
        case RIGHT:
            offset_adaptation(RIGHT, NULL, &x);
            beast->opposite_direction = LEFT;
            break;
        case UP:
            offset_adaptation(UP, &y, NULL);
            beast->opposite_direction = DOWN;
            break;
        case DOWN:
            offset_adaptation(DOWN, &y, NULL);
            beast->opposite_direction = UP;
            break;
        default:
            beast->opposite_direction = STAY;
            return;
    }

    pthread_mutex_lock(&game->main_mutex);

    if (game->map[beast->y_position + y][beast->x_position + x] == 'a'
    || game->map[beast->y_position + y][beast->x_position + x] == 'A'){
        pthread_mutex_unlock(&game->main_mutex);
        beast->coming_until_wall = false;
        return;
    }

    char object_to_save;
    unsigned int dropped_treasure;
    bool break_flag = false;
    if (beast->available_kill) {
            pthread_mutex_lock(&game->players_mutex);
            for (enum DIRECTION direct = 1; direct<=4; direct++) {
                int offset_x = 0, offset_y = 0 ;
                offset_adaptation(direct, &offset_y, &offset_x);
                for (int i = 0; i < game->number_of_players; i++) {
                PLAYER *player = game->players[i];
                if (player->id + 48 == game->map[beast->y_position + offset_y]
                        [beast->x_position + offset_x]) {
                    int kill_place_x = player->x_position, kill_place_y = player->y_position;
                    bool in_camp = player->in_camp;
                    dropped_treasure = kill_player(game, player);
                    pthread_mutex_lock(&player->player_mutex);
                    if (dropped_treasure > 0) {
                        object_to_save = 'D';
                        if (player->in_bush){
                            add_dropped_treasure(game, '#', dropped_treasure, kill_place_x, kill_place_y);
                        }
                        else{
                            add_dropped_treasure(game, ' ', dropped_treasure, kill_place_x, kill_place_y);
                        }
                    }
                    else {
                        object_to_save = ' ';
                        if (player->in_bush){
                            object_to_save = '#';
                        }
                        else if (in_camp){
                            object_to_save = 'A';
                        }
                    }
                    pthread_mutex_unlock(&player->player_mutex);
                    break_flag = true;
                    break;
                }
                }
                if (break_flag){
                    break;
                }
            }
            pthread_mutex_unlock(&game->players_mutex);
    }
    else{
        object_to_save = game->map[beast->y_position + y][beast->x_position + x];
    }

    game->map[beast->y_position + y][beast->x_position + x] = '*';
    if (beast->last_encountered_object != '*'){
        game->map[beast->y_position][beast->x_position] = beast->last_encountered_object;
    }
    pthread_mutex_unlock(&game->main_mutex);

    pthread_mutex_lock(&beast->beast_mutex);
    beast->already_moved = true;
    pthread_mutex_unlock(&beast->beast_mutex);
    beast->last_encountered_object = object_to_save;
    beast->x_position += x;
    beast->y_position += y;
    beast->last_direction = side;
    beast->coming_until_wall = true;
    beast->available_kill = false;
    generate_map(game);
}

void check_beast_vision(GAME *game, BEAST *beast){
    int x = beast->x_position, y = beast->y_position;
    beast->seeing_player = 0;
    beast->y_to_player = 0;
    beast->x_to_player = 0;

    bool** walls = calloc(5, sizeof(bool *));

    if (!walls){
        free_game(&game);
        main_error(ALLOCATION, &game);
    }

    for (int i=0; i<5; i++){
        *(walls + i) = calloc(5, sizeof(bool));

        if (!*(walls + i)){
            for (int j=0; j<i; j++){
                free(*(walls + j));
            }
            free(walls);
            main_error(ALLOCATION, &game);
        }
    }
    enum DIRECTION direct;

    pthread_mutex_lock(&game->main_mutex);
    // poziom pierwszy przeszukiwan
    for (direct=1; direct<=4; direct++){
        check_fields_for_player_occurrence(game, beast, walls, x, y, 1, direct, STAY);

        if (beast->seeing_player) {
            pthread_mutex_unlock(&game->main_mutex);
            return;
        }
    }
    // poziom drugi przeszukiwan
    for (direct=1; direct<=2; direct++){
        for (enum DIRECTION addit=3; addit<=4; addit++){
            check_fields_for_player_occurrence(game, beast, walls, x, y, 2, direct, addit);
            if (beast->seeing_player) {
                pthread_mutex_unlock(&game->main_mutex);
                return;
            }
        }
    }
    // poziom trzeci przeszukiwan
    for (direct=1; direct<=4; direct++) {
        int wall_x = 2, wall_y = 2;
        offset_adaptation(direct, &wall_y, &wall_x);
        if (walls[wall_y][wall_x]){
            continue; //sytuacja gdy, w pionie lub poziomie o jedno pole wystepuje sciana
        }
        check_fields_for_player_occurrence(game, beast, walls, x, y, 3, direct, STAY);
        if (beast->seeing_player) {
            pthread_mutex_unlock(&game->main_mutex);
            return;
        }
    }
    // poziom czwarty przeszukiwan
    for (direct=1; direct<=2; direct++) {
        for (enum DIRECTION addit=3; addit<=4; addit++){
            int wall_x = 2, wall_y = 2, offset_y = 0, offset_x = 0;
            bool wall_flag = false;
            offset_adaptation(direct, &wall_y, &wall_x);
            offset_adaptation(addit, &wall_y, &wall_x);
            offset_adaptation(direct, &offset_y, &offset_x);
            offset_adaptation(addit, &offset_y, &offset_x);
            if (walls[wall_y][wall_x]){
                continue;
            }
            check_fields_for_player_occurrence(game, beast, walls, x + offset_x, y + offset_y, 4, direct, STAY);
            int wall_x_addit = wall_x, wall_y_addit = wall_y;
            offset_adaptation(addit, &wall_y_addit, &wall_x_addit);
            offset_adaptation(direct, &wall_y, &wall_x);
            if (beast->seeing_player){
                if (walls[wall_y - offset_y][wall_x - offset_x]){
                    wall_flag = true;
                }
                if (!wall_flag){
                    beast->y_to_player += offset_y;
                    beast->x_to_player += offset_x;
                    pthread_mutex_unlock(&game->main_mutex);
                    return;
                }
            }
            beast->seeing_player = false;
            wall_flag = false;
            check_fields_for_player_occurrence(game, beast, walls, x + offset_x, y + offset_y, 4, addit, STAY);
            if (beast->seeing_player){
                if (walls[wall_y_addit - offset_y][wall_x_addit - offset_x]){
                    wall_flag = true;
                }
                if (!wall_flag){
                    beast->y_to_player += offset_y;
                    beast->x_to_player += offset_x;
                    pthread_mutex_unlock(&game->main_mutex);
                    return;
                }
            }
            beast->seeing_player = false;
        }
    }
    // poziom piaty przeszukiwan
    for (direct=1; direct<=2; direct++) {
        for (enum DIRECTION addit = 3; addit <= 4; addit++) {
            int wall_x = 2, wall_y = 2, offset_y = 0, offset_x = 0;
            offset_adaptation(direct, &wall_y, &wall_x);
            offset_adaptation(addit, &wall_y, &wall_x);
            offset_adaptation(direct, &offset_y, &offset_x);
            offset_adaptation(addit, &offset_y, &offset_x);
            if (walls[wall_y][wall_x]) {
                continue;
            }
            check_fields_for_player_occurrence(game, beast, walls, x + offset_x, y + offset_y, 2, direct, addit);
            if (beast->seeing_player){
                beast->y_to_player += offset_y;
                beast->x_to_player += offset_x;
                pthread_mutex_unlock(&game->main_mutex);
                return;
            }

        }
    }
    pthread_mutex_unlock(&game->main_mutex);

    for (int i=0; i<5; i++){
        free(*(walls + i));
    }
    free(walls);

    beast->seeing_player = false;
    beast->x_to_player = 0;
    beast->y_to_player = 0;
}

void founded_player(BEAST* beast, int x, int y){
    beast->seeing_player = true;
    beast-> x_to_player = x;
    beast-> y_to_player = y;
}

void check_fields_for_player_occurrence(GAME *game, BEAST *beast, bool** walls, int x, int y,
                                        unsigned int depth_of_search, enum DIRECTION direction, enum DIRECTION additional_direction)
{
    int offset_y = 0, offset_x = 0, wall_x = 2, wall_y = 2;
    switch (depth_of_search) {
        case 1:
            offset_adaptation(direction, &offset_y, &offset_x);
            offset_adaptation(direction, &wall_y, &wall_x);
            break;
        case 2:
        case 5:
            offset_adaptation(direction, &offset_y, &offset_x);
            offset_adaptation(additional_direction, &offset_y, &offset_x);
            offset_adaptation(direction, &wall_y, &wall_x);
            offset_adaptation(additional_direction, &wall_y, &wall_x);
            break;
        case 3:
            for (int i=0; i<2; i++){
                offset_adaptation(direction, &offset_y, &offset_x);
                offset_adaptation(direction, &wall_y, &wall_x);
            }
            break;
        case 4:
            offset_adaptation(direction, &offset_y, &offset_x);
            if (check_if_border_x_exceeded(x + offset_x) ||
                check_if_border_y_exceeded(y + offset_y)){
                return;
            }
            if (isdigit(game->map[y + offset_y][x + offset_x])){
                founded_player(beast, offset_x, offset_y);
            }
            break;
    }

    if (check_if_border_x_exceeded(x + offset_x) ||
        check_if_border_y_exceeded(y + offset_y)){
        return;
    }
    if (isdigit(game->map[y + offset_y][x + offset_x])){
        founded_player(beast, offset_x, offset_y);
        return;
    }
    if (game->map[y + offset_y][x + offset_x] == 'a'){
        walls[wall_y][wall_x] = true;
    }
    else{
        walls[wall_y][wall_x] = false;
    }
}

enum DIRECTION * check_available_directions(GAME *game, unsigned int x, unsigned int y, int* n){
    int counter = 1; //mozliwy stay
    enum DIRECTION* avail_directs = calloc(1, sizeof(enum DIRECTION));
    if (!avail_directs){
        main_error(ALLOCATION, &game);
    }
    pthread_mutex_lock(&game->main_mutex);
    for (int direct=1; direct<=4; direct++){
        int x_offset = x, y_offset = y;
        offset_adaptation(direct, &y_offset, &x_offset);
        if (game->map[y_offset][x_offset] != 'a'){
            enum DIRECTION* new_avail_directs = realloc(avail_directs, sizeof(enum DIRECTION) * (counter + 1));
            if (!new_avail_directs){
                free(avail_directs);
                main_error(ALLOCATION, &game);
            }
            avail_directs = new_avail_directs;
            avail_directs[counter] = direct;
            counter++;
        }
    }

    pthread_mutex_unlock(&game->main_mutex);
    *n = counter;
    return avail_directs;
}

enum DIRECTION rand_direction_for_beast_move(int n, enum DIRECTION* avail, enum DIRECTION opposite){
    srand(time(NULL));
    int x = rand() % n;
    enum DIRECTION direct = avail[x];
    unsigned int while_counter = 0;
    while ((direct == opposite || direct == STAY) && while_counter++ < 4){
        x = rand() % n;
        direct = avail[x];
    }
    free(avail);
    return direct;
}

enum DIRECTION check_if_chase_available(GAME *game, BEAST *beast, unsigned int x, unsigned int y, int x_to_player, int y_to_player){
    enum DIRECTION direct_x = STAY, direct_y = STAY;
    int offset_y = 0, offset_x = 0;
    if (y_to_player < 0){
        offset_adaptation(UP, &offset_y, NULL);
        direct_y = UP;
    }
    else if (y_to_player > 0){
        offset_adaptation(DOWN, &offset_y, NULL);
        direct_y = DOWN;
    }

    if (x_to_player < 0){
        offset_adaptation(LEFT, NULL, &offset_x);
        direct_x = LEFT;
    }
    else if (x_to_player > 0){
        offset_adaptation(RIGHT, NULL, &offset_x);
        direct_x = RIGHT;
    }

    if (abs(x_to_player) > abs(y_to_player)){
        pthread_mutex_lock(&game->main_mutex);
        if (game->map[y][x + offset_x] != 'a'){
            pthread_mutex_unlock(&game->main_mutex);
            return direct_x;
        }
        else{
            if (game->map[y + offset_y][x] != 'a'){
                pthread_mutex_unlock(&game->main_mutex);
                return direct_y;
            }
            pthread_mutex_unlock(&game->main_mutex);
            return STAY;
        }
    }
    else if (abs(y_to_player) > abs(x_to_player)){
        pthread_mutex_lock(&game->main_mutex);
        if (game->map[y + offset_y][x] != 'a'){
            pthread_mutex_unlock(&game->main_mutex);
            return direct_y;
        }
        else{
            if (game->map[y][x + offset_x] != 'a'){
                pthread_mutex_unlock(&game->main_mutex);
                return direct_x;
            }
            pthread_mutex_unlock(&game->main_mutex);
            return STAY;
        }
    }
    else{
        srand(time(NULL));
        if (rand() % 2){ //najpierw x
            pthread_mutex_lock(&game->main_mutex);
            if (game->map[y][x + offset_x] != 'a'){
                pthread_mutex_unlock(&game->main_mutex);
                return direct_x;
            }
            else{
                if (game->map[y + offset_y][x] != 'a') {
                    pthread_mutex_unlock(&game->main_mutex);
                    return direct_y;
                }
                pthread_mutex_unlock(&game->main_mutex);
                return STAY;
            }
        }
        else{ //najpierw y
            pthread_mutex_lock(&game->main_mutex);
            if (game->map[y + offset_y][x] != 'a'){
                pthread_mutex_unlock(&game->main_mutex);
                return direct_y;
            }
            else{
                if (game->map[y][x + offset_x] != 'a'){
                    pthread_mutex_unlock(&game->main_mutex);
                    return direct_x;
                }
                pthread_mutex_unlock(&game->main_mutex);
                return STAY;
            }
        }
    }
}

unsigned int kill_player(GAME *game, PLAYER *player){
    pthread_mutex_lock(&player->player_mutex);
    unsigned int carried = player->carried;
    player->carried = 0;
    player->x_position = player->x_spawn;
    player->y_position = player->y_spawn;
    player->bush_status = 0;
    player->in_camp = false;
    player->already_moved = false;
    player->deaths++;
    game->map[player->y_spawn][player->x_spawn] = player->id + 48;
    pthread_mutex_unlock(&player->player_mutex);
    return carried;
}
