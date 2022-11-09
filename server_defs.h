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
};

struct player_t{
    unsigned char id;
    bool in_bush;
    bool in_camp;
    int x_spawn;
    int y_spawn;
    unsigned int carried;
    unsigned int brought;
    unsigned int deaths;
    int x_position;
    int y_position;
};

struct beast_t{
    unsigned char id;
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

int spawn_player(PLAYER **ptr, char **map);
int spawn_beast(BEAST **beast, char **map, pthread_t* thread);
char ** load_map(char *filename, int *err);
void free_map(char **map, int height);
void generate_map(int width, int height, char **map);
void show_players_info(PLAYER **players);
void move_player(enum DIRECTION side, PLAYER *player, char **map);
void generate_element(enum TYPE type, char **map);
void main_error(enum ERROR err);

// THREADS
void * beast_thread(void * arg);


#endif
