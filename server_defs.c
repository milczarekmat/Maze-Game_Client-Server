#include "server_defs.h"
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>

// TODO pomyslec o tych zmiennych globalnych
int player_counter = 0;
int beast_counter = 0;

// TODO zmienic na zwracanie wskaznika i zmienna err dla bestii i gracza
int spawn_player(PLAYER **ptr, char **map){
    //*ptr = malloc(sizeof(PLAYER));

    //if (!*ptr){
        //return -1;
    //}

    int x, y;
    srand(time(NULL));
    do{
        x = rand() % WIDTH;
        y = rand() % HEIGHT;
    }
    while(map[y][x] != ' ');
    // TODO zmienic na spawnowanie wg id
    map[y][x] = '1';
    (*ptr)->x_spawn = x;
    (*ptr)->x_position = x;
    (*ptr)->y_spawn = y;
    (*ptr)->y_position = y;
    (*ptr)->id = player_counter + 1;
    (*ptr)->carried = 0;
    (*ptr)->brought = 0;
    (*ptr)->deaths = 0;
    (*ptr)->in_bush = FALSE;
    (*ptr)->in_camp = FALSE;

    player_counter++;
    generate_map(WIDTH, HEIGHT, map);
    return 0;
}

int spawn_beast(BEAST **beast, char **map, pthread_t* thread){
    *beast = malloc(sizeof(PLAYER));

    if (!*beast){
        return -1;
    }

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

    beast_counter++;
    // watek bestii
    generate_map(WIDTH, HEIGHT, map);
    return 0;

}

void generate_map(int width, int height, char **map){

    start_color();
    noecho();
    init_pair(1, COLOR_GREEN, COLOR_WHITE); // kolor mapy
    init_pair(2, COLOR_WHITE, COLOR_MAGENTA); //kolor gracza
    init_pair(3, COLOR_WHITE, COLOR_BLACK); // kolor tła
    init_pair(4, COLOR_WHITE, COLOR_RED); // kolor obozu
    init_pair(5, COLOR_BLACK, COLOR_YELLOW); // kolor skarbów
    bkgd(COLOR_PAIR(3));

    for (int i=0; i<height; i++){
        for (int j=0; j<width; j++){
            if (map[i][j] == (char)(player_counter + '0')){
                mvaddch(i, j, map[i][j] | A_ALTCHARSET | COLOR_PAIR(2));
            }
            else if (map[i][j] == 'A'){
                mvaddch(i, j, map[i][j] | COLOR_PAIR(4));
            }
            else if (map[i][j] == 'c' || map[i][j] == 't' || map[i][j] == 'T'){
                mvaddch(i, j, map[i][j] | COLOR_PAIR(5));
            }
            else{
                mvaddch(i, j, map[i][j] | A_ALTCHARSET | COLOR_PAIR(1));
            }
            //TODO koloruj gracza wg id
        }
    }
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

void move_player(enum DIRECTION side, PLAYER *player, char **map){
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
    if (map[player->y_position + y][player->x_position + x] == 'a'){
        return;
    }

    if (player->in_bush){
        map[player->y_position][player->x_position] = '#';
    }
    else if (player->in_camp){
        map[player->y_position][player->x_position] = 'A';
    }
    else{
        map[player->y_position][player->x_position] = ' ';
    }

    player->in_bush = FALSE;
    player->in_camp = FALSE;
    switch (map[player->y_position + y][player->x_position + x]){
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

    map[player->y_position + y][player->x_position + x] = '1';
    player->y_position += y;
    player->x_position += x;
}

void show_players_info(PLAYER **players){
    int size = 5;
    while (*players){
        mvprintw(0, WIDTH + size, "Player ID: %d", (*players)->id);
        mvprintw(2 , WIDTH + size, "Player spawn(X/Y): %d/%d", (*players)->x_spawn, (*players)->y_spawn);
        mvprintw(4 , WIDTH + size, "Current X/Y: %d/%d", (*players)->x_position, (*players)->y_position);
        mvprintw(6 , WIDTH + size, "Carried: %d", (*players)->carried);
        mvprintw(8 , WIDTH + size, "Brought: %d", (*players)->brought);
        mvprintw(10, WIDTH + size, "Deaths: %d", (*players)->deaths);
        mvprintw(12 , WIDTH + size, "Press q/Q to quit");
        size += 5;
        players++;
    }
    move(0, 0);
    refresh();
}

void generate_element(enum TYPE type, char **map){
    srand(time(NULL));
    int x, y;
    do{
        x = rand() % WIDTH;
        y = rand() % HEIGHT;
    }
    while (map[y][x] != ' ');

    if (type == COIN){
        map[y][x] = 'c';
    }
    else if (type == SMALL_TREASURE){
        map[y][x] = 't';
    }
    else if (type == TREASURE){
        map[y][x] = 'T';
    }
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
