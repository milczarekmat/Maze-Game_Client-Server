#ifndef GRA_PROJEKT_SERVER_DEFS_H
#define GRA_PROJEKT_SERVER_DEFS_H

#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>
#include <time.h>
#include <ncurses.h>
#include "server_threads.h"

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
    int socket_fd;
    //TODO wykorzystac flage (true po zrespieniu pierwszego gracza)
    bool started_game;
    struct player_t* players;
    //struct beast_t* beasts;
    //pthread_t* beasts_threads;
    struct beast_t* beasts [10];
    struct dropped_treasure_t** dropped_treasures;
    pthread_t beasts_threads[10];
    pthread_t players_threads[4];
    unsigned int number_of_players;
    unsigned int number_of_beasts;
    unsigned int rounds;
    unsigned int number_of_dropped_treasures;
    pthread_t tick_thread;
    pthread_t listener;
    pthread_mutex_t main_mutex;
    pthread_mutex_t players_mutex;
    pthread_mutex_t beasts_mutex;
    pthread_mutex_t treasures_mutex;
    pthread_cond_t char_wait;
};

struct player_t{
    unsigned char id;
    int* file_descriptor;
    bool already_moved;
    bool in_bush;
    bool in_camp;
    unsigned short bush_status;
    int x_spawn;
    int y_spawn;
    unsigned int carried;
    unsigned int brought;
    unsigned int deaths;
    int x_position;
    int y_position;
    char object_to_save;
    pthread_mutex_t player_mutex;
    pthread_cond_t bush_wait;
};

struct beast_t{
    bool already_moved;
    bool seeing_player;
    bool coming_until_wall;
    bool available_kill;
    int x_position;
    int y_position;
    char last_encountered_object;
    int x_to_player;
    int y_to_player;
    enum DIRECTION last_direction;
    enum DIRECTION opposite_direction;
    pthread_mutex_t beast_mutex;
    pthread_cond_t move_wait;
};

struct dropped_treasure_t{
    unsigned int value;
    unsigned int x;
    unsigned int y;
    char last_object;
    pthread_mutex_t treasure_mutex;
};

struct send_data_t{
    int x;
    int y;
    char player_map[5][5];
    unsigned int game_round;
    unsigned int carried;
    unsigned int brought;
};

typedef struct player_t PLAYER;
typedef struct beast_t BEAST;
typedef struct game_t GAME;
typedef struct dropped_treasure_t DROPPED_TREASURE;
typedef struct send_data_t SEND_DATA;

GAME * create_game();
char ** load_map(char *filename, int *err);
void generate_map(GAME *game);
void show_players_info(GAME *game);
int spawn_player(GAME *game, int* file_descriptor);
void move_player(enum DIRECTION side, GAME* game, unsigned int id);
void offset_adaptation(enum DIRECTION direction, int* offset_y, int* offset_x);

void generate_element(enum TYPE type, GAME* game);
void main_error(enum ERROR err);
void free_map(char **map, int height);
void free_game(GAME **game);
bool check_if_border_x_exceeded(unsigned int x);
bool check_if_border_y_exceeded(unsigned int y);

unsigned int kill_player(GAME* game, PLAYER* player);
void add_dropped_treasure(GAME* game, char object_to_save, unsigned int carried_by_player,
                          unsigned int x, unsigned int y);
char get_dropped_treasure(GAME* game, PLAYER*player, unsigned int player_x, unsigned int player_y);

#endif
