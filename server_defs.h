#ifndef GRA_PROJEKT_SERVER_DEFS_H
#define GRA_PROJEKT_SERVER_DEFS_H

#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>
#include <time.h>
#include <ncurses.h>

#define HEIGHT 25
#define WIDTH 45

enum DIRECTION{
    STAY = 0,
    LEFT = 1,
    RIGHT = 2,
    DOWN = 3,
    UP = 4
};

enum TYPE{
    COIN,
    SMALL_TREASURE,
    TREASURE
};

enum ERROR{
    FILE_OPEN = 1,
    ALLOCATION,
    SIZE_OF_CONSOLE,
};

struct game_t{
    char** map;
    struct player_t* players;
    struct beast_t* beasts;
    pthread_t* beasts_threads;
    unsigned int number_of_players;
    unsigned int number_of_beasts;
    unsigned int rounds;
    pthread_t tick_thread;
    pthread_mutex_t map_mutex;
    pthread_mutex_t players_mutex;
    pthread_mutex_t beasts_mutex;
    // TODO players mutex
};

struct player_t{
    unsigned char id;
    bool already_moved;
//    bool in_bush;
//    bool out_bush;
    bool in_camp;
    unsigned short bush_status;
    int x_spawn;
    int y_spawn;
    unsigned int carried;
    unsigned int brought;
    unsigned int deaths;
    int x_position;
    int y_position;
    pthread_mutex_t player_mutex;
    pthread_cond_t move_wait;
    pthread_cond_t bush_wait;
};

struct beast_t{
    bool already_moved;
    bool seeing_player;
    bool coming_until_wall;
    int x_position;
    int y_position;
    char last_encountered_object;
    int x_to_player;
    int y_to_player;
    enum DIRECTION last_direction;
    pthread_mutex_t beast_mutex;
};

typedef struct player_t PLAYER;
typedef struct beast_t BEAST;
typedef struct game_t GAME;

GAME * create_game();
char ** load_map(char *filename, int *err);
void generate_map(GAME *game);
void show_players_info(GAME *game);
int spawn_player(GAME *game);
void move_player(enum DIRECTION side, GAME* game, unsigned int id);
void offset_adaptation(enum DIRECTION direction, int* offset_y, int* offset_x);

void generate_element(enum TYPE type, GAME* game);
void main_error(enum ERROR err);
void free_map(char **map, int height);
void free_game(GAME **game);
bool check_if_border_x_exceeded(unsigned int x);
bool check_if_border_y_exceeded(unsigned int y);

#endif
