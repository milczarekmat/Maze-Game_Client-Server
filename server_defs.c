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
    return game;
}

int spawn_player(GAME *game){
    PLAYER *new_players = realloc(game->players, (game->number_of_players + 1) * sizeof(PLAYER));
    if (!new_players){
        return -1;
    }
    game->players = new_players;
    PLAYER *player = game->players + game->number_of_players;

    int x, y;
    srand(time(NULL));
    do{
        x = rand() % WIDTH;
        y = rand() % HEIGHT;
    }
    while(game->map[y][x] != ' ');
    // TODO zmienic na spawnowanie wg id
    game->map[y][x] = '1';
    (player)->x_spawn = x;
    (player)->x_position = x;
    (player)->y_spawn = y;
    (player)->y_position = y;
    (player)->id = game->number_of_players + 1;
    (player)->carried = 0;
    (player)->brought = 0;
    (player)->deaths = 0;
    (player)->in_bush = FALSE;
    (player)->in_camp = FALSE;
    (player)->already_moved = FALSE;

    pthread_mutex_init(&(game->players + game->number_of_players)->player_mutex, NULL);

    (game->number_of_players)++;
    generate_map(game);
    return 0;
}

//TODO zmienic na strukture funkcji wg spawn_player
int spawn_beast(BEAST **beast, char **map, pthread_t* thread){
    *beast = malloc(sizeof(PLAYER));

    if (!*beast){
        return -1;
    }

    //muteks
    int x, y;
    srand(time(NULL));
    do{
        x = rand() % WIDTH;
        y = rand() % HEIGHT;
    }
    while(map[y][x] != ' ');
    // TODO zmienic na spawnowanie wg id
    map[y][x] = '*';
    (*beast)->x_position = x;
    (*beast)->y_position = y;
    (*beast)->in_bush = FALSE;
    (*beast)->in_camp = FALSE;

    //game->;
    // watek bestii
    //generate_map(WIDTH, HEIGHT, map);
    return 0;

}

void generate_map(GAME *game){

    start_color();
    noecho();
    // TODO ponizsze do funkcji
    init_pair(1, COLOR_GREEN, COLOR_WHITE); // kolor mapy
    init_pair(2, COLOR_WHITE, COLOR_MAGENTA); //kolor gracza
    init_pair(3, COLOR_WHITE, COLOR_BLACK); // kolor tła
    init_pair(4, COLOR_WHITE, COLOR_RED); // kolor obozu
    init_pair(5, COLOR_BLACK, COLOR_YELLOW); // kolor skarbów
    bkgd(COLOR_PAIR(3));

    pthread_mutex_lock(&game->map_mutex);
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
            else{
                mvaddch(i, j, game->map[i][j] | A_ALTCHARSET | COLOR_PAIR(1));
            }
            //TODO koloruj gracza wg id
        }
    }
    pthread_mutex_unlock(&game->map_mutex);
    move(0, 0);
    refresh();
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
    for (int i=0; i<(*game)->number_of_players; i++){
        pthread_mutex_destroy(&((*game)->players + i)->player_mutex);
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
    if (player->already_moved == true){
        pthread_mutex_unlock(&player->player_mutex);
        return;
    }
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


    // TODO sprawdzic ponizej
    pthread_mutex_lock(&game->map_mutex);
    if (game->map[player->y_position + y][player->x_position + x] == 'a'){
        pthread_mutex_unlock(&game->map_mutex);
        return;
    }

    if (player->in_bush){
        game->map[player->y_position][player->x_position] = '#';
    }
    else if (player->in_camp){
        game->map[player->y_position][player->x_position] = 'A';
    }
    else{
        game->map[player->y_position][player->x_position] = ' ';
    }

    //player->in_bush = FALSE;
    //player->in_camp = FALSE;
    switch (game->map[player->y_position + y][player->x_position + x]){
        case ' ':
            player->in_bush = FALSE;
            player->in_camp = FALSE;
            break;
        case '#':
            player->in_bush = TRUE;
            player->in_camp = FALSE;
            break;
        case 'A':
            player->in_bush = FALSE;
            player->in_camp = TRUE;
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
    }

    // TODO zmienic na id
    game->map[player->y_position + y][player->x_position + x] = '1';
    pthread_mutex_unlock(&game->map_mutex);

    pthread_mutex_lock(&player->player_mutex);
    player->already_moved = true;
    pthread_mutex_unlock(&player->player_mutex);

    player->y_position += y;
    player->x_position += x;
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
        //mvprintw(14, WIDTH + (size * j), "Already moved: %d", (game->players + i)->already_moved);
        mvprintw(16 , WIDTH + (size * j), "Press q/Q to quit");
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
