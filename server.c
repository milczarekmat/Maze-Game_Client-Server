#include <ncurses.h>
#include <unistd.h>
#include "socket_server.h"
#include "server_defs.h"
#include "server_threads.h"
#include "beast.h"

// TODO kolizja graczy, znikanie obozu po przejsciu bestii, komunikat o pelnym serwerze, crashowanie klienta jesli coin jest obok krzaka lub podwojne krzaki, zwalnianie dropped treasures i listener, sprobowac usunac muteks pojedynczej bestii (oprocz beast->already_moved), bestia na skrzyzowaniach, jezeli nie ma wolnego miesjca na mapie zakonczyc generowanie elementu, ogarnac wylaczanie watkow, muteks dla spawnowania gracza, zmiana spawn beast
int main() {
    GAME* game = create_game();

    int rows, cols;
    initscr();
    getmaxyx(stdscr,rows,cols);
    if (rows < HEIGHT || cols < WIDTH){
        free_game(&game);
        main_error(SIZE_OF_CONSOLE);
    }

/*    int check_alloc = spawn_player(game, NULL);
    if (check_alloc){
        free_game(&game);
        main_error(ALLOCATION);
    }*/

    noecho();
    init_colors();
    generate_map(game);
    show_basic_info(game);
    spawn_player(game, NULL);
    show_players_info(game);
    pthread_create(&game->tick_thread, NULL, &tick, game);
    keypad(stdscr, TRUE);
    cbreak();
    init_server_socket(game);
    while(true){
        //pthread_mutex_lock(&(game->map_mutex));
        //nodelay(stdscr, TRUE);
        int ch = getch();
/*        if (ch == ERR){
            move(20, WIDTH + (10));
            clrtoeol();
            mvprintw(20, WIDTH + (10), "ERR");
        }*/
        //pthread_mutex_unlock(&(game->map_mutex));
        switch (ch) {
            case 'q':
            case 'Q':
                close(game->socket_fd);
                endwin();
                free_game(&game);
                return 0;
                // TODO PRZEKAZYWANIE ODPOWIEDNIEGO GRACZA
            case KEY_UP:
                move_player(UP, game, 1);
                break;
            case KEY_DOWN:
                move_player(DOWN, game, 1);
                break;
            case KEY_LEFT:
                move_player(LEFT, game, 1);
                break;
            case KEY_RIGHT:
                move_player(RIGHT, game, 1);
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
