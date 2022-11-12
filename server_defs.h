#ifndef GRA_PROJEKT_SERVER_DEFS_H
#define GRA_PROJEKT_SERVER_DEFS_H

#include <stdbool.h>
#include <pthread.h>

#define HEIGHT 25
#define WIDTH 45

struct game_t{
    char **map;
    struct player_t *players;
    struct beast_t *beasts;
    unsigned int number_of_players;
    unsigned int number_of_beasts;
    unsigned int rounds;
    pthread_t tick_thread;
    pthread_mutex_t map_mutex;
};

struct player_t{
    unsigned char id;
    bool already_moved;
    bool in_bush;
    bool in_camp;
    int x_spawn;
    int y_spawn;
    unsigned int carried;
    unsigned int brought;
    unsigned int deaths;
    int x_position;
    int y_position;
    pthread_mutex_t player_mutex;
};

struct beast_t{
    unsigned char id;
    bool already_moved;
    bool in_bush;
    bool in_camp;
    int x_position;
    int y_position;
};

enum DIRECTION{
    LEFT = 1,
    RIGHT,
    DOWN,
    UP
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

typedef struct player_t PLAYER;
typedef struct beast_t BEAST;
typedef struct game_t GAME;

GAME * create_game();
int spawn_player(GAME *game);
int spawn_beast(BEAST **beast, char **map, pthread_t* thread);
char ** load_map(char *filename, int *err);
void free_map(char **map, int height);
void free_game(GAME **game);
void generate_map(GAME *game);
void show_players_info(GAME *game);
void move_player(enum DIRECTION side, GAME* game, unsigned int id);
void generate_element(enum TYPE type, char **map);
void main_error(enum ERROR err);


#endif
