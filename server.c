#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include "server_defs.h"

//temp
//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*void tick(int *flag, int *round_num){
   // while(1){
        *flag = 0;
        usleep(350000);
        *flag = 1;
        (*round_num)++;
    //}
}*/

// TODO ALOKACJA STRUKTURY GRY I TABLIC GRACZY ORAZ BESTII - FUNKCJE, wprowadzenie ticków serwera
int main() {
/*
    //temp
    int round_num = 0;
    int flag_main = 0;

    //
*/
    int err, rows, cols;;
    GAME *game = malloc(sizeof(GAME));
    if (!game){
        main_error(ALLOCATION);
    }

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
    getmaxyx(stdscr,rows,cols);
    if (rows < HEIGHT || cols < WIDTH){
        free_game(&game);
        main_error(SIZE_OF_CONSOLE);
    }

    int check_alloc = spawn_player(game);

    if (check_alloc){
        free_game(&game);
        main_error(ALLOCATION);
    }

    show_players_info(game);
    keypad(stdscr, TRUE);
    while(1){
        int ch = getch();
        switch (ch) {
            case 'q':
            case 'Q':
                free_game(&game);
                endwin();
                return 0;
                // TODO PRZEKAZYWANIE ODPOWIEDNIEGO GRACZA
            case KEY_UP:
                move_player(UP, game->players, game->map);
                break;
            case KEY_DOWN:
                move_player(DOWN, game->players, game->map);
                break;
            case KEY_LEFT:
                move_player(LEFT, game->players, game->map);
                break;
            case KEY_RIGHT:
                move_player(RIGHT, game->players, game->map);
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
        //mvprintw(14, WIDTH + 5, "Round: %d", round_num);
        //tick(&flag_main, &round_num);
        generate_map(game);
        show_players_info(game);
    }
    //pthread_mutex_destroy(&mutex);
}
