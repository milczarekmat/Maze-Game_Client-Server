#ifndef GRA_PROJEKT_SERVER_DEFS_H
#define GRA_PROJEKT_SERVER_DEFS_H

#include <stdbool.h>
#define HEIGHT 25
#define WIDTH 45

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

int spawn_player(PLAYER **ptr, char **map);
char ** load_map(char *filename, int *err);
void free_map(char **map, int height);
void generate_map(int width, int height, char **map);
void show_players_info(PLAYER **players);
void move_player(enum DIRECTION side, PLAYER *player, char **map);
void generate_element(enum TYPE type, char **map);
void main_error(enum ERROR err);

#endif
