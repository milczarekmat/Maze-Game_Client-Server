#ifndef GRA_PROJEKT_BEAST_H
#define GRA_PROJEKT_BEAST_H
#include "server_defs.h"

int spawn_beast(GAME *game);
//enum DIRECTION * check_available_directions(GAME *game, unsigned int x, unsigned int y, int* n);
void check_beast_vision(GAME *game, BEAST *beast);
void offset_adaptation(enum DIRECTION direction, int* offset_y, int* offset_x);
void founded_player(BEAST* beast, int x, int y);
// TODO zmienic na przekazywanie samej mapy bez gry?
void check_fields_for_player_occurrence(GAME *game, BEAST *beast, bool** walls, int x, int y,
                                        unsigned int depth_of_search, enum DIRECTION direction, enum DIRECTION additional_direction);
#endif
