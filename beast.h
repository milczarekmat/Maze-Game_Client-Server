#ifndef GRA_PROJEKT_BEAST_H
#define GRA_PROJEKT_BEAST_H
#include "server_defs.h"
#include "server_threads.h"

int spawn_beast(GAME *game);
void move_beast(enum DIRECTION side, GAME* game, BEAST *beast);
enum DIRECTION * check_available_directions(GAME *game, unsigned int x, unsigned int y, int* n);

// TODO uwzglednij przeciwny kierunek w ktorym szla do tej pory w losowaniu
enum DIRECTION rand_direction_for_beast_move(int n, enum DIRECTION* avail);
void check_beast_vision(GAME *game, BEAST *beast);
void founded_player(BEAST* beast, int x, int y);

// TODO zmienic na przekazywanie samej mapy bez gry?
void check_fields_for_player_occurrence(GAME *game, BEAST *beast, bool** walls, int x, int y,
                                        unsigned int depth_of_search, enum DIRECTION direction, enum DIRECTION additional_direction);
#endif
