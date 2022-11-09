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

int main() {
    //temp
    int round_num = 0;
    int flag_main = 0;


    int err;
    char **map = load_map("map", &err);
    initscr();
    if (err){
        main_error(err);
    }

    int rows, cols;
    getmaxyx(stdscr,rows,cols);

    if (rows < HEIGHT || cols < WIDTH){
        main_error(SIZE_OF_CONSOLE);
    }

    PLAYER **players = calloc(2, sizeof(PLAYER *));

    if (!players){
        main_error(ALLOCATION);
    }

    int check_alloc = spawn_player(players, map);

    if (check_alloc){
        main_error(ALLOCATION);
    }

    show_players_info(players);
    keypad(stdscr, TRUE);
    while(1){
        int ch = getch();
        switch (ch) {
            case 'q':
            case 'Q':
                free(players);
                free_map(map, HEIGHT);
                endwin();
                return 0;
                // TODO PRZEKAZYWANIE ODPOWIEDNIEGO GRACZA
            case KEY_UP:
                move_player(UP, *players, map);
                break;
            case KEY_DOWN:
                move_player(DOWN, *players, map);
                break;
            case KEY_LEFT:
                move_player(LEFT, *players, map);
                break;
            case KEY_RIGHT:
                move_player(RIGHT, *players, map);
                break;
            case 'c':
                generate_element(COIN, map);
                break;
            case 't':
                generate_element(SMALL_TREASURE, map);
                break;
            case 'T':
                generate_element(TREASURE, map);
                break;
        }
        erase();
        mvprintw(14, WIDTH + 5, "Round: %d", round_num);
        tick(&flag_main, &round_num);
        generate_map(WIDTH, HEIGHT, map);
        show_players_info(players);
    }
    //pthread_mutex_destroy(&mutex);
}
