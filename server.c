#include <ncurses.h>
#include <unistd.h>
#include "server_defs.h"
#include "server_threads.h"

//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// TODO zwolnic mutex playera, ogarnac wylaczanie watkow, respienie coinow od razu bez tickow (mutex na generate_map?), zmiana spawn beast
int main() {
    GAME* game = create_game();

    int rows, cols;
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

    generate_map(game);
    show_players_info(game);
    pthread_create(&game->tick_thread, NULL, &tick, game);
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
                move_player(UP, game, 0);
                break;
            case KEY_DOWN:
                move_player(DOWN, game, 0);
                break;
            case KEY_LEFT:
                move_player(LEFT, game, 0);
                break;
            case KEY_RIGHT:
                move_player(RIGHT, game, 0);
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
        //erase();
        //generate_map(game);
        //show_players_info(game);
    }
    //pthread_mutex_destroy(&mutex);
}
