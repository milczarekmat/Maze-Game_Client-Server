#include <ncurses.h>
#include <unistd.h>
#include <ctype.h>
#include "server_defs.h"
#include "server_threads.h"
#include "beast.h"

// TODO naprawic blad z watkami, ogarnac mutex w warunku (yt), jezeli nie ma wolnego miesjca na mapie zakonczyc generowanie elementu, ogarnac wylaczanie watkow, muteks dla spawnowania gracza, zmiana spawn beast, zmienic bush_status, zmienic sprawdzenie rows i cols dla statystyk graczy
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
    cbreak();
    while(1){
        int ch = getch();
/*        pthread_mutex_lock(&game->players->player_mutex);
            if
        pthread_mutex_unlock(&game->players->player_mutex);*/

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
                generate_element(COIN, game);
                break;
            case 't':
                generate_element(SMALL_TREASURE, game);
                break;
            case 'T':
                generate_element(TREASURE, game);
                break;
            case 'b':
            case 'B':
                spawn_beast(game);
                break;
        }
    }
}
