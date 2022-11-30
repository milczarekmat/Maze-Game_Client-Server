#include "server_defs.h"

GAME * create_game(){
    GAME *game = malloc(sizeof(GAME));
    if (!game){
        main_error(ALLOCATION);
    }
    int err;
    game->map = load_map("map", &err);
    if (err){
        free(game);
        main_error(err);
    }
    game->number_of_beasts = 0;
    game->number_of_players = 0;
    game->rounds = 1;
    game->number_of_dropped_treasures = 0;
    game->dropped_treasures = NULL;
    pthread_mutex_init(&game->main_mutex, NULL);
    pthread_mutex_init(&game->players_mutex, NULL);
    pthread_mutex_init(&game->beasts_mutex, NULL);
    pthread_mutex_init(&game->treasures_mutex, NULL);
    pthread_cond_init(&game->char_wait, NULL);
    return game;
}

void init_colors(){
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_WHITE); // kolor mapy
    init_pair(2, COLOR_WHITE, COLOR_MAGENTA); //kolor gracza
    init_pair(3, COLOR_WHITE, COLOR_BLACK); // kolor tła
    init_pair(4, COLOR_WHITE, COLOR_RED); // kolor obozu
    init_pair(5, COLOR_BLACK, COLOR_YELLOW); // kolor skarbów
    init_pair(6, COLOR_RED, COLOR_WHITE); // kolor bestii
    init_pair(7, COLOR_WHITE, COLOR_YELLOW); // kolor upuszczonych skarbów
    bkgd(COLOR_PAIR(3));
}

int spawn_player(GAME *game, int* file_descriptor){
    //TODO muteks, watek obslugujacy gracza
    pthread_mutex_lock(&game->players_mutex);
/*    PLAYER *new_players = realloc(game->players, (game->number_of_players + 1) * sizeof(PLAYER));
    if (!new_players){
        //todo zwalniac pamiec juz w funkcji zamiast zwracac kod bledu
        return -1;
    }
    game->players = new_players;
    PLAYER *player = game->players + game->number_of_players;*/
    game->players[game->number_of_players] = malloc(sizeof(PLAYER));
    PLAYER *player = game->players[game->number_of_players];
    pthread_mutex_unlock(&game->players_mutex);

    int x, y;
    srand(time(NULL));
    pthread_mutex_lock(&game->main_mutex);
    do{
        x = rand() % WIDTH;
        y = rand() % HEIGHT;
    }
    while(game->map[y][x] != ' ');
    player->id = game->number_of_players + 1;
    game->map[8][33] = player->id + 48;
    pthread_mutex_unlock(&game->main_mutex);
    // koordy przy obozie y16 x26
    // y8 x33 srodek
    player->x_spawn = 33;
    player->x_position = 33;
    player->y_spawn = 8;
    player->y_position = 8;
    player->file_descriptor = file_descriptor;
    player->carried = 0;
    player->brought = 0;
    player->deaths = 0;
    player->in_bush = false;
    player->bush_status = 0;
    player->in_camp = false;
    player->already_moved = false;
    player->object_to_save = 0;

    pthread_mutex_init(&player->player_mutex, NULL);
    pthread_cond_init(&player->bush_wait, NULL);

    if (file_descriptor){
        pthread_create(game->players_threads + game->number_of_players, NULL, &player_thread, game);
    }
    else{
        game->number_of_players++;
    }
    
    // TODO DODAC MUTEKS PLAYERS?
    //pthread_mutex_lock(&game->players_mutex);
    //(game->number_of_players)++;
    //pthread_mutex_unlock(&game->players_mutex);
    generate_map(game);
    return 0;
}

void generate_map(GAME *game){
    pthread_mutex_lock(&game->main_mutex);
    for (int i=0; i<HEIGHT; i++){
        for (int j=0; j<WIDTH; j++){
            if (isdigit(game->map[i][j])){
                mvaddch(i, j, game->map[i][j] | A_ALTCHARSET | COLOR_PAIR(2));
            }
            else if (game->map[i][j] == 'A'){
                mvaddch(i, j, game->map[i][j] | COLOR_PAIR(4));
            }
            else if (game->map[i][j] == 'c' || game->map[i][j] == 't' || game->map[i][j] == 'T'){
                mvaddch(i, j, game->map[i][j] | COLOR_PAIR(5));
            }
            else if (game->map[i][j] == '*'){
                mvaddch(i, j, game->map[i][j] | COLOR_PAIR(6));
            }
            else if (game->map[i][j] == 'D'){
                mvaddch(i, j, game->map[i][j] | COLOR_PAIR(7));
            }
            else{
                mvaddch(i, j, game->map[i][j] | A_ALTCHARSET | COLOR_PAIR(1));
            }
        }
    }
    move(0, 0);
    refresh();
    pthread_mutex_unlock(&game->main_mutex);
}

void show_basic_info(GAME *game){
    init_pair(8, COLOR_RED, COLOR_BLACK); // kolor bestii legenda
    init_pair(9, COLOR_GREEN, COLOR_BLACK); // kolor krzaka legenda
    pthread_mutex_lock(&game->main_mutex);
    mvprintw(0, WIDTH+5, "Server's PID: %d", getpid());

    mvprintw(5 , WIDTH+5, "Player ID's");
    mvprintw(6 , WIDTH+5, "Current X/Y");
    mvprintw(7 , WIDTH+5, "Deaths");
    mvprintw(8, WIDTH+5, "Coins");
    mvprintw(9 , WIDTH+7, "Carried");
    mvprintw(10 , WIDTH+7, "Brought");

    mvprintw(13 , WIDTH+5, "Legend:");
    attron(COLOR_PAIR(2));
    mvaddch(15 , WIDTH+6, '1');
    mvaddch(15 , WIDTH+7, '2');
    mvaddch(15 , WIDTH+8, '3');
    mvaddch(15 , WIDTH+9, '4');
    attroff(COLOR_PAIR(2));
    mvaddch(16 , WIDTH+6, 'a' | A_ALTCHARSET | COLOR_PAIR(1));
    mvaddch(17 , WIDTH+6, '#' | COLOR_PAIR(9));
    mvaddch(18 , WIDTH+6, '*' | COLOR_PAIR(8));
    mvaddch(19 , WIDTH+6, 'c' | COLOR_PAIR(5));
    mvaddch(20 , WIDTH+6, 'C' | COLOR_PAIR(5));
    mvaddch(21 , WIDTH+6, 'T' | COLOR_PAIR(5));
    mvaddch(22 , WIDTH+6, 'D' | COLOR_PAIR(7));
    mvaddch(23 , WIDTH+6, 'A' | COLOR_PAIR(4));

    mvprintw(15 , WIDTH+10, "- players");
    mvprintw(16 , WIDTH+10, "- wall");
    mvprintw(17 , WIDTH+10, "- bushes (slow down)");
    mvprintw(18 , WIDTH+10, "- wild beast");
    mvprintw(19 , WIDTH+10, "- one coin");
    mvprintw(20 , WIDTH+10, "- treasure (10 coins)");
    mvprintw(21 , WIDTH+10, "- large treasure (50 coins)");
    mvprintw(22 , WIDTH+10, "- dropped treasure");
    mvprintw(23 , WIDTH+10, "- campsite");
    mvprintw(25, WIDTH+5, "Press q/Q to quit");

    move(0, 0);
    refresh();
    pthread_mutex_unlock(&game->main_mutex);
}

void show_players_info(GAME *game){
    int size = 30;
    pthread_mutex_lock(&game->main_mutex);
    move(2, WIDTH + 5);
    clrtoeol();
    mvprintw(2 , WIDTH+5, "Round number: %d", game->rounds);
    move(3, WIDTH + 5);
    clrtoeol();
    mvprintw(3 , WIDTH+5, "Number of players: %d", game->number_of_players);

    for (int i = 0, j = i + 1; i < game->number_of_players; i++){
        move(5, WIDTH + size);
        clrtoeol();
        mvprintw(5, WIDTH + (size * j), "%d", game->players[i]->id);
        move(6, WIDTH + size);
        clrtoeol();
        mvprintw(6 , WIDTH + (size * j), "%d/%d", game->players[i]->x_position, game->players[i]->y_position);
        move(7, WIDTH + size);
        clrtoeol();
        mvprintw(7 , WIDTH + (size * j), "%d", game->players[i]->deaths);
        move(9, WIDTH + size);
        clrtoeol();
        mvprintw(9 , WIDTH + (size * j), "%d", game->players[i]->carried);
        move(10, WIDTH + size);
        clrtoeol();
        mvprintw(10 , WIDTH + (size * j), "%d", game->players[i]->brought);
        size += 15;
    }
    move(0, 0);
    refresh();
    pthread_mutex_unlock(&game->main_mutex);
}

char ** load_map(char *filename, int *err){
    FILE *fp = fopen(filename, "r");

    if (!fp){
        *err = 1;
        return NULL;
    }

    char **map = calloc(HEIGHT, sizeof(char *));

    if (!map){
        *err = 2;
        fclose(fp);
        return NULL;
    }

    for (int i=0; i<HEIGHT; i++){
        *(map + i) = calloc(WIDTH, sizeof(char));

        if (!*(map + i)){
            for (int j=0; j<i; j++){
                free(*(map + j));
            }
            free(map);
            *err = 2;
            fclose(fp);
            return NULL;
        }
    }

    for (int i=0; i<HEIGHT; i++){
        for (int j=0; j<WIDTH; j++){
            *(*(map + i) + j) = fgetc(fp);
            fgetc(fp);
        }
    }

    *err = 0;
    return map;
}

void free_game(GAME **game){
    pthread_mutex_lock(&(*game)->beasts_mutex);
    for (int i=0; i<(*game)->number_of_beasts; i++){
        pthread_cancel(((*game)->beasts_threads[i]));
    }
    for (int i=0; i<(*game)->number_of_beasts; i++){
        pthread_mutex_destroy(&((*game)->beasts[i])->beast_mutex);
    }
    pthread_mutex_unlock(&(*game)->beasts_mutex);

    pthread_cancel((*game)->tick_thread);

    pthread_mutex_lock(&(*game)->main_mutex);
    free_map((*game)->map, HEIGHT);
    pthread_mutex_unlock(&(*game)->main_mutex);

    pthread_mutex_lock(&(*game)->players_mutex);
    for (int i=0; i<(*game)->number_of_players; i++){
        pthread_mutex_destroy(&((*game)->players[i])->player_mutex);
        pthread_cond_destroy(&((*game)->players[i])->bush_wait);
    }
    pthread_mutex_unlock(&(*game)->players_mutex);

    //TODO zwalnianie treasures i ich mutexow
    pthread_mutex_lock(&(*game)->treasures_mutex);
    for (int i=0; i<(*game)->number_of_dropped_treasures; i++){
        pthread_mutex_destroy(&(*game)->dropped_treasures[i]->treasure_mutex);
        free((*game)->dropped_treasures[i]);
    }
    pthread_mutex_unlock(&(*game)->treasures_mutex);
    free((*game)->dropped_treasures);

    pthread_mutex_destroy(&(*game)->main_mutex);
    pthread_mutex_destroy(&(*game)->treasures_mutex);
    pthread_mutex_destroy(&(*game)->players_mutex);
    pthread_mutex_destroy(&(*game)->beasts_mutex);
    pthread_mutex_destroy(&(*game)->treasures_mutex);

//    if ((*game)->players){
//        free((*game)->players);
//    }
/*    if ((*game)->beasts){
        free((*game)->beasts);
    }
    if ((*game)->beasts_threads){
        free((*game)->beasts_threads);
    }*/
    free(*game);
}

void free_map(char **map, int height){
    for (int i=0; i<height; i++){
        free(*(map + i));
    }
    free(map);
}

void offset_adaptation(enum DIRECTION direction, int* offset_y, int* offset_x){
    switch (direction){
        case LEFT:
            *offset_x -= 1;
            break;
        case RIGHT:
            *offset_x += 1;
            break;
        case UP:
            *offset_y -= 1;
            break;
        case DOWN:
            *offset_y += 1;
            break;
        default:
            break;
    }
}

void move_player(enum DIRECTION side, GAME* game, unsigned int id){
    PLAYER *player = game->players[id - 1];

    pthread_mutex_lock(&player->player_mutex);
    if (player->already_moved){
        pthread_mutex_unlock(&player->player_mutex);
        return;
    }

    if (player->bush_status > 1){
        pthread_cond_wait(&player->bush_wait, &player->player_mutex);
    }

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

    pthread_mutex_lock(&game->main_mutex);
    if (game->map[player->y_position + y][player->x_position + x] == 'a'){
        pthread_mutex_unlock(&game->main_mutex);
        pthread_mutex_unlock(&player->player_mutex);
        return;
    }

    if (player->object_to_save){
        game->map[player->y_position][player->x_position] = player->object_to_save;
        player->object_to_save = 0;
    }
    else {
        if (player->bush_status == 1) {
            game->map[player->y_position][player->x_position] = '#';
        }
        else if (player->in_camp) {
            game->map[player->y_position][player->x_position] = 'A';
        }
        else {
            game->map[player->y_position][player->x_position] = ' ';
        }
    }

    switch (game->map[player->y_position + y][player->x_position + x]){

        case ' ':
            player->in_camp = false;
            player->in_bush = false;
            player->bush_status = 0;
            break;
        case '#':
            player->in_camp = false;
            player->in_bush = true;
            player->bush_status = 3;
            break;
        case 'A':
            player->in_camp = true;
            player->bush_status = 0;
            player->in_bush = false;
            player->brought += player->carried;
            player->carried = 0;
            break;
        case 'c':
            player->carried += 1;
            break;
        case 't':
            player->carried += 10;
            break;
        case 'T':
            player->carried += 50;
            break;
        case '*':
            pthread_mutex_unlock(&game->main_mutex);
            pthread_mutex_unlock(&player->player_mutex);
            kill_player(game, player);
            return;
        case 'D':
            player->object_to_save = get_dropped_treasure(game, player, player->x_position + x, player->y_position + y);
            break;
    }

    game->map[player->y_position + y][player->x_position + x] = player->id + 48;
    pthread_mutex_unlock(&game->main_mutex);

    player->already_moved = true;
    player->y_position += y;
    player->x_position += x;
    show_players_info(game);
    pthread_mutex_unlock(&player->player_mutex);
    generate_map(game);
}

void generate_element(enum TYPE type, GAME* game){
    srand(time(NULL));
    int x, y;
    pthread_mutex_lock(&game->main_mutex);
    do{
        x = rand() % WIDTH;
        y = rand() % HEIGHT;
    }
    while (game->map[y][x] != ' ');

    if (type == COIN){
        game->map[y][x] = 'c';
    }
    else if (type == SMALL_TREASURE){
        game->map[y][x] = 't';
    }
    else if (type == TREASURE){
        game->map[y][x] = 'T';
    }
    pthread_mutex_unlock(&game->main_mutex);
    generate_map(game);
}

//TODO dorobic zwalnianie pamieci
void main_error(enum ERROR err){
    clear();
    endwin();
    if (err == FILE_OPEN){
        printf("Failed to open resources with map\n");
        exit(1);
    }
    else if (err == ALLOCATION){
        printf("Failed to allocate memory\n");
        exit(2);
    }
    else{
        printf("Too small size of console to play!\n");
        exit(3);
    }
}

bool check_if_border_y_exceeded(unsigned int y){
    return y >= HEIGHT || y < 0 ? true : false;
}

bool check_if_border_x_exceeded(unsigned int x){
    return x >= WIDTH || x < 0 ? true : false;
}

void add_dropped_treasure(GAME *game, char object_to_save, unsigned int carried_by_player,
                          unsigned int x, unsigned int y){
    DROPPED_TREASURE *dropped_treas = NULL;
    pthread_mutex_lock(&game->treasures_mutex);
    for (unsigned int i=0; i<game->number_of_dropped_treasures; i++){
        DROPPED_TREASURE* temp = game->dropped_treasures[i];
        if (temp->x == x && temp->y == y){
            dropped_treas = temp;
            break;
        }
    }
    pthread_mutex_unlock(&game->treasures_mutex);

    if (dropped_treas){
        //pthread_mutex_lock(&dropped_treas->treasure_mutex);
        dropped_treas->value += carried_by_player;
        //pthread_mutex_unlock(&dropped_treas->treasure_mutex);
    }
    else{
        pthread_mutex_lock(&game->treasures_mutex);
        DROPPED_TREASURE ** new_treasures = realloc(game->dropped_treasures,
                                                    (game->number_of_dropped_treasures+1)*sizeof(DROPPED_TREASURE*));
        if (!new_treasures){
            // TODO wykorzystac main_error
            endwin();
            free_game(&game);
            perror("Failed to allocate memory for dropped_treasures");
            exit(4);
        }
        game->dropped_treasures = new_treasures;
		pthread_mutex_unlock(&game->treasures_mutex);
        
        game->dropped_treasures[game->number_of_dropped_treasures] = malloc(sizeof(DROPPED_TREASURE));
        game->dropped_treasures[game->number_of_dropped_treasures]->value = carried_by_player;
        game->dropped_treasures[game->number_of_dropped_treasures]->x = x;
        game->dropped_treasures[game->number_of_dropped_treasures]->y = y;
        game->dropped_treasures[game->number_of_dropped_treasures]->last_object = object_to_save;
        pthread_mutex_init(&game->dropped_treasures[game->number_of_dropped_treasures]->treasure_mutex, NULL);
        game->number_of_dropped_treasures++;
    }
}

char get_dropped_treasure(GAME* game, PLAYER*player, unsigned int player_x, unsigned int player_y){
    DROPPED_TREASURE *dropped_treas = NULL;
    pthread_mutex_lock(&game->treasures_mutex);
    for (unsigned int i=0; i<game->number_of_dropped_treasures; i++){
        DROPPED_TREASURE* temp = game->dropped_treasures[i];
        if (temp->x == player_x && temp->y == player_y ){
            dropped_treas = temp;
            break;
        }
    }
	pthread_mutex_unlock(&game->treasures_mutex);
    char object_to_save;
    // TODO czy potrzebny tu muteks treas?
    //pthread_mutex_lock(&dropped_treas->treasure_mutex);
    player->carried += dropped_treas->value;
    object_to_save = dropped_treas->last_object;
    //pthread_mutex_unlock(&dropped_treas->treasure_mutex);
    return object_to_save;
}

void send_player_information(GAME* game, PLAYER* player){
    if (!player->file_descriptor){
        return;
    }
    SEND_DATA data;
    data.x = player->x_position;
    data.y = player->y_position;
    data.carried = player->carried;
    data.brought = player->brought;
    data.id = player->id;
    data.deaths = player->deaths;

    data.game_round = game->rounds;
    pthread_mutex_lock(&game->main_mutex);
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
    long check = send(*player->file_descriptor, &data, sizeof(data), 0);

    if (check == -1){
        //todo
    }
    pthread_mutex_unlock(&game->main_mutex);
}

void delete_player(GAME* game, PLAYER* player){
    pthread_mutex_lock(&player->player_mutex);
    pthread_mutex_lock(&game->main_mutex);
    if (player->object_to_save){
        game->map[player->y_position][player->x_position] = player->object_to_save;
    }
    else {
        if (player->bush_status >= 1) {
            game->map[player->y_position][player->x_position] = '#';
        }
        else if (player->in_camp) {
            game->map[player->y_position][player->x_position] = 'A';
        }
        else {
            game->map[player->y_position][player->x_position] = ' ';
        }
    }
    pthread_mutex_unlock(&player->player_mutex);
    pthread_mutex_unlock(&game->main_mutex);
    free(player);
}
