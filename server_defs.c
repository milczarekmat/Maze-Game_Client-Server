#include "server_defs.h"
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>

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
    game->players = NULL;
    game->beasts = NULL;
    pthread_mutex_init(&game->map_mutex, NULL);
    pthread_mutex_init(&game->players_mutex, NULL);
    pthread_mutex_init(&game->beasts_mutex, NULL);
    return game;
}

int spawn_player(GAME *game){
    //TODO muteks, watek obslugujacy gracza
    pthread_mutex_lock(&game->players_mutex);
    PLAYER *new_players = realloc(game->players, (game->number_of_players + 1) * sizeof(PLAYER));
    if (!new_players){
        return -1;
    }
    game->players = new_players;
    PLAYER *player = game->players + game->number_of_players;
    pthread_mutex_unlock(&game->players_mutex);

    int x, y;
    srand(time(NULL));
    pthread_mutex_lock(&game->map_mutex);
    do{
        x = rand() % WIDTH;
        y = rand() % HEIGHT;
    }
    while(game->map[y][x] != ' ');
    // TODO zmienic na spawnowanie wg id
    game->map[y][x] = '1';
    pthread_mutex_unlock(&game->map_mutex);
    // koordy przy obozie y16 x26
    // TODO ZMIENIC NA LOSOWANIE Z POWROTEM
    player->x_spawn = x;
    player->x_position = x;
    player->y_spawn = y;
    player->y_position = y;
    player->id = game->number_of_players + 1;
    player->carried = 0;
    player->brought = 0;
    player->deaths = 0;
//    player->in_bush = FALSE;
//    player->out_bush = FALSE;
    player->bush_status = 0;
    player->in_camp = FALSE;
    player->already_moved = FALSE;

    pthread_mutex_init(&player->player_mutex, NULL);
    pthread_cond_init(&player->move_wait, NULL);
    pthread_cond_init(&player->bush_wait, NULL);

    (game->number_of_players)++;
    generate_map(game);
    return 0;
}

//TODO zmienic na strukture funkcji wg spawn_player
int spawn_beast(GAME *game){
    // TODO CZY POTTRZEBNY TU MUTEKS BESTII?
    pthread_mutex_lock(&game->beasts_mutex);
    BEAST *new_beasts = realloc(game->beasts, (game->number_of_beasts + 1) * sizeof(BEAST));

    if (!new_beasts){
        return -1;
    }
    game->beasts = new_beasts;
    BEAST* beast = game->beasts + game->number_of_beasts;
    pthread_mutex_unlock(&game->beasts_mutex);

    int x, y;
    srand(time(NULL));
    pthread_mutex_lock(&game->map_mutex);
    do{
        x = rand() % WIDTH;
        y = rand() % HEIGHT;
    }
    while(game->map[y][x] != ' ');
    // TODO zmienic na mozliwosc respienia w krzakach
    game->map[y][x] = '*';
    pthread_mutex_unlock(&game->map_mutex);

    beast->x_position = x;
    beast->y_position = y;
    beast->id = game->number_of_players + 1;
    beast->already_moved = false;
    beast->seeing_player = false;
    beast->coming_until_wall = false;
    beast->last_encountered_object = ' ';

    pthread_mutex_init(&beast->beast_mutex, NULL);
    pthread_cond_init(&beast->move_wait, NULL);

    // watek bestii
    (game->number_of_beasts)++;
    generate_map(game);
    return 0;
}

void generate_map(GAME *game){
    pthread_mutex_lock(&game->map_mutex);
    start_color();
    noecho();
    // TODO ponizsze do funkcji
    init_pair(1, COLOR_GREEN, COLOR_WHITE); // kolor mapy
    init_pair(2, COLOR_WHITE, COLOR_MAGENTA); //kolor gracza
    init_pair(3, COLOR_WHITE, COLOR_BLACK); // kolor tła
    init_pair(4, COLOR_WHITE, COLOR_RED); // kolor obozu
    init_pair(5, COLOR_BLACK, COLOR_YELLOW); // kolor skarbów
    init_pair(6, COLOR_RED, COLOR_WHITE); // kolor bestii
    bkgd(COLOR_PAIR(3));

    for (int i=0; i<HEIGHT; i++){
        for (int j=0; j<WIDTH; j++){
            if (game->map[i][j] == (char)(game->number_of_players + '0')){
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
            else{
                mvaddch(i, j, game->map[i][j] | A_ALTCHARSET | COLOR_PAIR(1));
            }
            //TODO koloruj gracza wg id
        }
    }
    move(0, 0);
    refresh();
    pthread_mutex_unlock(&game->map_mutex);
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
    if ((*game)->players){
        free((*game)->players);
    }
    if ((*game)->beasts){
        free((*game)->beasts);
    }
    free_map((*game)->map, HEIGHT);
    pthread_mutex_destroy(&(*game)->map_mutex);
    pthread_mutex_destroy(&(*game)->players_mutex);
    pthread_mutex_destroy(&(*game)->beasts_mutex);
    for (int i=0; i<(*game)->number_of_players; i++){
        pthread_mutex_destroy(&((*game)->players + i)->player_mutex);
        pthread_cond_destroy(&((*game)->players + i)->move_wait);
        pthread_cond_destroy(&((*game)->players + i)->bush_wait);
    }
    for (int i=0; i<(*game)->number_of_beasts; i++){
        pthread_mutex_destroy(&((*game)->beasts + i)->beast_mutex);
    }
    free(*game);
}

void free_map(char **map, int height){
    if (!map || height<= 0){
        return;
    }

    for (int i=0; i<height; i++){
        free(*(map + i));
    }
    free(map);
}

void move_player(enum DIRECTION side, GAME* game, unsigned int id){
    PLAYER *player = game->players + id;

    pthread_mutex_lock(&player->player_mutex);
    //TODO PROBLEM Z RACE CONDITIONS
    if (player->already_moved){
        pthread_mutex_unlock(&player->player_mutex);
        return;
    }

/*    pthread_mutex_unlock(&player->player_mutex);
    pthread_mutex_lock(&player->player_mutex);*/
    if (player->bush_status > 1){
        pthread_cond_wait(&player->bush_wait, &player->player_mutex);
    }
    int player_x = player->x_position, player_y = player->y_position;
    bool player_in_camp = player->in_camp;
    unsigned int player_bush_status = player->bush_status, player_brought = player->brought, player_carried = player->carried;
    pthread_mutex_unlock(&player->player_mutex);


    // TODO zmienic poruszanie wg id
    int x, y;
    switch (side) {
        case LEFT:
            y = 0;
            x = -1;
            break;
        case RIGHT:
            y = 0;
            x = 1;
            break;
        case UP:
            y = -1;
            x = 0;
            break;
        case DOWN:
            y = 1;
            x = 0;
            break;
        default:
            x = 0;
            y = 0;
    }


    // TODO sprawdzic ponizej race conidtions
    pthread_mutex_lock(&game->map_mutex);
    if (game->map[player_y + y][player_x + x] == 'a'){
        pthread_mutex_unlock(&game->map_mutex);
        return;
    }

    if (player_bush_status >= 1){
        game->map[player_y][player_x] = '#';
    }
    else if (player_in_camp){
        game->map[player_y][player_x] = 'A';
    }
    else{
        game->map[player_y][player_x] = ' ';
    }

    switch (game->map[player_y + y][player_x + x]){
        case ' ':
//            player->in_bush = FALSE;
//            player->in_camp = FALSE;
            player_in_camp = FALSE;
            player_bush_status = 0;
            break;
        case '#':
//            player->in_bush = TRUE;
//            player->out_bush = FALSE;
            player_in_camp = FALSE;
            player_bush_status = 3;
            break;
        case 'A':
//            player->in_bush = FALSE;
//            player->in_camp = TRUE;
            player_in_camp = TRUE;
            player_bush_status = 0;
            player_brought += player_carried;
            player_carried = 0;
            break;
        case 'c':
            player_carried += 1;
            break;
        case 't':
            player_carried += 10;
            break;
        case 'T':
            player_carried += 50;
            break;
    }

    // TODO zmienic na id
    game->map[player->y_position + y][player->x_position + x] = '1';
    pthread_mutex_unlock(&game->map_mutex);

    pthread_mutex_lock(&player->player_mutex);
    player->already_moved = true;
    player->y_position += y;
    player->x_position += x;
    player->bush_status = player_bush_status;
    player->in_camp = player_in_camp;
    player->carried = player_carried;
    player->brought = player_brought;
    show_players_info(game);
    pthread_mutex_unlock(&player->player_mutex);

    // TODO muteksy dla postion przy spawnowaniu graczy itd
    generate_map(game);
}

void show_players_info(GAME *game){
    int size = 5;
    for (int i = 0; i < game->number_of_players; i++){
        int j = i + 1;
        // TODO refaktoryzacja
        move(0, WIDTH + (size * j));
        clrtoeol();
        mvprintw(0, WIDTH + (size * j), "Player ID: %d", (game->players + i)->id);
        move(2, WIDTH + (size * j));
        clrtoeol();
        mvprintw(2 , WIDTH + (size * j), "Player spawn(X/Y): %d/%d", (game->players + i)->x_spawn, (game->players + i)->y_spawn);
        move(4, WIDTH + (size * j));
        clrtoeol();
        mvprintw(4 , WIDTH + (size * j), "Current X/Y: %d/%d", (game->players + i)->x_position, (game->players + i)->y_position);
        move(6, WIDTH + (size * j));
        clrtoeol();
        mvprintw(6 , WIDTH + (size * j), "Carried: %d", (game->players + i)->carried);
        move(8, WIDTH + (size * j));
        clrtoeol();
        mvprintw(8 , WIDTH + (size * j), "Brought: %d", (game->players + i)->brought);
        move(10, WIDTH + (size * j));
        clrtoeol();
        mvprintw(10, WIDTH + (size * j), "Deaths: %d", (game->players + i)->deaths);
        move(12, WIDTH + (size * j));
        clrtoeol();
        mvprintw(12, WIDTH + (size * j), "Round number: %d", game->rounds);
        mvprintw(18 , WIDTH + (size * j), "Press q/Q to quit");
        size += 5;
    }
    move(0, 0);
    refresh();
}

void generate_element(enum TYPE type, GAME* game){
    srand(time(NULL));
    int x, y;
    pthread_mutex_lock(&game->map_mutex);
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
    pthread_mutex_unlock(&game->map_mutex);
    generate_map(game);
}

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
