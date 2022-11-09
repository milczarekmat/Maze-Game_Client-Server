#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include "server_defs.h"

//temp
//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void tick(int *flag, int *round_num){
   // while(1){
        *flag = 0;
        usleep(350000);
        *flag = 1;
        (*round_num)++;
    //}
}

// TODO ZWALNIANIE STRUKTURY GRY - FUNCKJA,inicjalizacja struktury gry, wprowadzenie ticków serwera
int main() {
    //temp
    int round_num = 0;
    int flag_main = 0;

    //

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
    game->players = NULL;
    game->beasts = NULL;

    initscr();
    int rows, cols;
    getmaxyx(stdscr,rows,cols);

    if (rows < HEIGHT || cols < WIDTH){
        free(game);
        main_error(SIZE_OF_CONSOLE);
    }

    GAME *new_player = realloc(game->players, (game->number_of_players + 1) * sizeof(GAME));
    if (!new_player){
        //TODO zwalnianie pamieci
    }
    //TODO OGARNAC DALEJ
    PLAYER **players = calloc(2, sizeof(PLAYER *));

    if (!players){
        free(game);
        main_error(ALLOCATION);
    }

    int check_alloc = spawn_player(players, game->map);

    if (check_alloc){
        free(game);
        main_error(ALLOCATION);
    }

    show_players_info(players);
    keypad(stdscr, TRUE);
    while(1){
        int ch = getch();
        switch (ch) {
            case 'q':
            case 'Q':
                free(*players);
                free(players);
                free_map(game->map, HEIGHT);
                free(game);
                endwin();
                return 0;
                // TODO PRZEKAZYWANIE ODPOWIEDNIEGO GRACZA
            case KEY_UP:
                move_player(UP, *players, game->map);
                break;
            case KEY_DOWN:
                move_player(DOWN, *players, game->map);
                break;
            case KEY_LEFT:
                move_player(LEFT, *players, game->map);
                break;
            case KEY_RIGHT:
                move_player(RIGHT, *players, game->map);
                break;
            case 'c':
                generate_element(COIN, game->map);
                break;
            case 't':
                generate_element(SMALL_TREASURE, game->map);
                break;
            case 'T':
                generate_element(TREASURE, game->map);
                break;
        }
        erase();
        mvprintw(14, WIDTH + 5, "Round: %d", round_num);
        tick(&flag_main, &round_num);
        generate_map(WIDTH, HEIGHT, game->map);
        show_players_info(players);
    }
    //pthread_mutex_destroy(&mutex);
}
