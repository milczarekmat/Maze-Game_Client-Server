#include "beast.h"

//TODO zmienic na strukture funkcji wg spawn_player
int spawn_beast(GAME *game){
    // TODO CZY POTTRZEBNY TU MUTEKS BESTII?
    pthread_mutex_lock(&game->beasts_mutex);
    game->beasts[game->number_of_beasts] = malloc(sizeof(BEAST));
    BEAST *beast = game->beasts[game->number_of_beasts];

//    pthread_mutex_lock(&(game->beasts + game->number_of_beasts)->beast_mutex);
/*    BEAST *new_beasts = realloc(game->beasts, (game->number_of_beasts + 1) * sizeof(BEAST));

    if (!new_beasts){
        perror("Failed to allocate memory for beast struct");
        free_game(&game);
        exit(4);
    }
    game->beasts = new_beasts;
    BEAST* beast = game->beasts + game->number_of_beasts;
    pthread_t* new_beasts_threads = realloc(game->beasts_threads, sizeof(pthread_t) * (game->number_of_beasts + 1));
    if (!new_beasts_threads){
        perror("Failed to allocate memory for beast threads");
        free_game(&game);
        exit(4);
    }
    game->beasts_threads = new_beasts_threads;*/

    pthread_mutex_unlock(&game->beasts_mutex);
//    pthread_mutex_lock(&(game->beasts + game->number_of_beasts)->beast_mutex);
    int x, y;
    srand(time(NULL));
    pthread_mutex_lock(&game->map_mutex);
    do{
        x = rand() % WIDTH;
        y = rand() % HEIGHT;
    }
    while(game->map[y][x] != ' ');
    // TODO zmienic na mozliwosc respienia w krzakach?
    game->map[y][x] = '*';
    pthread_mutex_unlock(&game->map_mutex);

    beast->x_position = x;
    beast->y_position = y;
    beast->x_to_player = 0;
    beast->y_to_player = 0;
    beast->already_moved = false;
    beast->seeing_player = false;
    beast->coming_until_wall = false;
    beast->last_encountered_object = ' ';

    pthread_mutex_init(&beast->beast_mutex, NULL);

    pthread_create(game->beasts_threads + game->number_of_beasts, NULL, beast_thread, game);
    generate_map(game);
    return 0;
}

void move_beast(enum DIRECTION side, GAME* game, BEAST *beast){
    int x = 0, y = 0;
    switch (side) {
        case LEFT:
            offset_adaptation(LEFT, NULL, &x);
            break;
        case RIGHT:
            offset_adaptation(RIGHT, NULL, &x);
            break;
        case UP:
            offset_adaptation(UP, &y, NULL);
            break;
        case DOWN:
            offset_adaptation(DOWN, &y, NULL);
            break;
        default:
            x = 0;
            y = 0;
    }

    pthread_mutex_lock(&game->map_mutex);
    if (game->map[beast->y_position + y][beast->x_position + x] == 'a'){
        pthread_mutex_unlock(&game->map_mutex);
        //TODO MUTEKS BEAST?
        beast->coming_until_wall = false;
        return;
    }
    char object_to_save = game->map[beast->y_position + y][beast->x_position + x];
    // TODO CO JESLI OBJECT JEST GRACZEM
    // TODO MUTEKS BEAST?
    game->map[beast->y_position + y][beast->x_position + x] = '*';
    game->map[beast->y_position][beast->x_position] = beast->last_encountered_object;
    pthread_mutex_unlock(&game->map_mutex);

    pthread_mutex_lock(&beast->beast_mutex);
    beast->already_moved = true;
    beast->last_encountered_object = object_to_save;
    beast->x_position += x;
    beast->y_position += y;
    beast->last_direction = side;
    beast->coming_until_wall = true;
    pthread_mutex_unlock(&beast->beast_mutex);
    generate_map(game);
}

void check_beast_vision(GAME *game, BEAST *beast){
    // TODO ZASTANOWIC SIE CZY MUTEKSY BESTII SA TU POTRZEBNE
    pthread_mutex_lock(&beast->beast_mutex);
    int x = beast->x_position, y = beast->y_position;
    beast->seeing_player = 0;
    beast->y_to_player = 0;
    beast->x_to_player = 0;
    pthread_mutex_unlock(&beast->beast_mutex);

    bool** walls = calloc(5, sizeof(bool *));

    if (!walls){
        free_game(&game);
        perror("Failed to allocate memory for walls_tab");
        exit(3);
    }

    for (int i=0; i<5; i++){
        *(walls + i) = calloc(5, sizeof(bool));

        if (!*(walls + i)){
            for (int j=0; j<i; j++){
                free(*(walls + j));
            }
            free(walls);
            free_game(&game);
            perror("Failed to allocate memory for walls arr");
            exit(3);
        }
    }
    // TODO to na gorze do funkcji

    char ** beast_view = calloc(5, sizeof(char *));
    if (!beast_view){
        perror("Failed to allocate memory for beast_map arr");
        free(walls);
        free_game(&game);
        exit(3);
    }
    for (int i=0; i<5; i++){
        *(beast_view + i) = calloc(5, sizeof(char));

        if (!*(beast_view + i)){
            for (int j=0; j<i; j++){
                free(*(beast_view + j));
            }
            free(beast_view);
            free(walls);
            free_game(&game);
            perror("Failed to allocate memory for beast_map arr");
            exit(3);
        }
    }

    /*pthread_mutex_lock(&game->map_mutex);
    for (int i=-2, k=0; i<=2; i++, k++){
        for (int j=-2, l=0; j<=2; j++, l++){
            if (check_if_border_x_exceeded(x + i) ||
                check_if_border_y_exceeded(y + j)){
                continue;
            }
            beast_view[k][l] = game->map[x + i][y + j];
        }
    }
    pthread_mutex_unlock(&game->map_mutex);*/

    enum DIRECTION direct;
    // TODO OPISAC INSTRUKCJE Z PLIKIEM PNG

    pthread_mutex_lock(&game->map_mutex);
    // poziom pierwszy przeszukiwan
    for (direct=1; direct<=4; direct++){
        check_fields_for_player_occurrence(game, beast, walls, x, y, 1, direct, STAY);
        // TODO race conditions?
        //TODO MUTEKS DLA BESTII!!! I PONIZEJ
        pthread_mutex_lock(&beast->beast_mutex);
        if (beast->seeing_player) {
            pthread_mutex_unlock(&game->map_mutex);
            pthread_mutex_unlock(&beast->beast_mutex);
            return;
        }
        pthread_mutex_unlock(&beast->beast_mutex);
    }
    // poziom drugi przeszukiwan
    for (direct=1; direct<=2; direct++){
        for (enum DIRECTION addit=3; addit<=4; addit++){
            check_fields_for_player_occurrence(game, beast, walls, x, y, 2, direct, addit);
            // TODO race conditions?
            pthread_mutex_lock(&beast->beast_mutex);
            if (beast->seeing_player) {
                pthread_mutex_unlock(&game->map_mutex);
                pthread_mutex_unlock(&beast->beast_mutex);
                return;
            }
            pthread_mutex_unlock(&beast->beast_mutex);
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
        // TODO race conditions?
        pthread_mutex_lock(&beast->beast_mutex);
        if (beast->seeing_player) {
            pthread_mutex_unlock(&game->map_mutex);
            pthread_mutex_unlock(&beast->beast_mutex);
            return;
        }
        pthread_mutex_unlock(&beast->beast_mutex);
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
            // TODO MUTEKS REKURSYWNY beast
            pthread_mutex_lock(&beast->beast_mutex);
            if (beast->seeing_player){
                if (walls[wall_y - offset_y][wall_x - offset_x]){
                    wall_flag = true;
                }
                if (!wall_flag){
                    beast->y_to_player += offset_y;
                    beast->x_to_player += offset_x;
                    pthread_mutex_unlock(&game->map_mutex);
                    pthread_mutex_unlock(&beast->beast_mutex);
                    return;
                }
            }
            beast->seeing_player = false;
            wall_flag = false;
            pthread_mutex_unlock(&beast->beast_mutex);
            check_fields_for_player_occurrence(game, beast, walls, x + offset_x, y + offset_y, 4, addit, STAY);
            pthread_mutex_lock(&beast->beast_mutex);
            if (beast->seeing_player){
                if (walls[wall_y_addit - offset_y][wall_x_addit - offset_x]){
                    wall_flag = true;
                }
                if (!wall_flag){
                    beast->y_to_player += offset_y;
                    beast->x_to_player += offset_x;
                    pthread_mutex_unlock(&game->map_mutex);
                    pthread_mutex_unlock(&beast->beast_mutex);
                    return;
                }
            }
            beast->seeing_player = false;
            //wall_flag = false;
            pthread_mutex_unlock(&beast->beast_mutex);
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
            pthread_mutex_lock(&beast->beast_mutex);
            if (beast->seeing_player){
                beast->y_to_player += offset_y;
                beast->x_to_player += offset_x;
                pthread_mutex_unlock(&game->map_mutex);
                pthread_mutex_unlock(&beast->beast_mutex);
                return;
            }
            pthread_mutex_unlock(&beast->beast_mutex);
        }
    }
    pthread_mutex_unlock(&game->map_mutex);


    for (int i=0; i<5; i++){
        free(*(walls + i));
        free(*(beast_view + i));
    }
    free(walls);
    free(beast_view);
}

void founded_player(BEAST* beast, int x, int y){
    pthread_mutex_lock(&beast->beast_mutex);
    beast->seeing_player = true;
    beast-> x_to_player = x;
    beast-> y_to_player = y;
    pthread_mutex_unlock(&beast->beast_mutex);
}

void check_fields_for_player_occurrence(GAME *game, BEAST *beast, bool** walls, int x, int y,
                                        unsigned int depth_of_search, enum DIRECTION direction, enum DIRECTION additional_direction)
{
    int offset_y = 0, offset_x = 0, wall_x = 2, wall_y = 2;
    // TODO KOMENTARZE OPISUJACE DZIALANIE
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
        free_game(&game);
        perror("Failed to allocate enums array");
        exit(3);
    }

    pthread_mutex_lock(&game->map_mutex);
    for (int direct=1; direct<=4; direct++){
        int x_offset = x, y_offset = y;
        offset_adaptation(direct, &y_offset, &x_offset);
        if (game->map[y_offset][x_offset] != 'a'){
            enum DIRECTION* new_avail_directs = realloc(avail_directs, sizeof(enum DIRECTION) * (counter + 1));
            if (!new_avail_directs){
                free_game(&game);
                free(avail_directs);
                perror("Failed to reallocate enums array");
                exit(3);
            }
            avail_directs = new_avail_directs;
            avail_directs[counter] = direct;
            counter++;
        }
    }

    pthread_mutex_unlock(&game->map_mutex);
    *n = counter;
    return avail_directs;
}

enum DIRECTION rand_direction_for_beast_move(int n, enum DIRECTION* avail){
    int x = rand() % n;
    enum DIRECTION direct = avail[x];
    if (direct == STAY){
        x = rand() % n;
        direct = avail[x];
    }
    free(avail);
    return direct;
}
