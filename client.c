#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ncurses.h>
#include <pthread.h>
#include <ctype.h>
#define PORT 8080
#define BACKLOG 5

void init_colors();
void * event_handler(void * arg);

struct received_t{
    int x;
    int y;
    char player_map[5][5];
    unsigned int game_round;
    unsigned int carried;
    unsigned int brought;
    unsigned char id;
    unsigned int deaths;
};

struct socket_and_signal{
    int socket_fd;
    char* signal;
};

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

pthread_t event_thread;
pthread_mutex_t main_mutex = PTHREAD_MUTEX_INITIALIZER;

void generate_map(char map[5][5]);
void generate_player_info(struct received_t data);

int main(){
    SA_IN server_address;
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1){
        perror("Error with creating socket\n");
        return 1;
    }
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(PORT);

    if (connect(socket_fd, (SA*)&server_address, sizeof(server_address)) == -1) {
        perror("Connection with the server failed...\n");
        return 2;
    }

    struct socket_and_signal socket_and_signal;
    socket_and_signal.socket_fd = socket_fd;
    socket_and_signal.signal = malloc(sizeof(char));

    if (!socket_and_signal.signal){
        endwin();
        exit(2);
    }
    *socket_and_signal.signal = 'x';

    initscr();
    noecho();
    init_colors();
    pthread_create(&event_thread, NULL, &event_handler, &socket_and_signal);
    struct received_t player_data;
    long connection_check = recv(socket_fd, &player_data, sizeof(struct received_t), 0);
    if (connection_check == 0){
        endwin();
        printf("Pełny serwer\n");
        close(socket_fd);
        pthread_mutex_destroy(&main_mutex);
        exit(1);
    }

    generate_map(player_data.player_map);
    generate_player_info(player_data);

    do{
        long check = recv(socket_fd, &player_data, sizeof(struct received_t), 0);

        if (check == 0){
            close(socket_fd);
            endwin();
            pthread_mutex_destroy(&main_mutex);
            free(socket_and_signal.signal);
            printf("Server exited\n");
            exit(1);
        }
        if (check == sizeof(struct received_t)){
            generate_map(player_data.player_map);
            generate_player_info(player_data);
        }
    }
    while (true);
}


void * event_handler(void * arg) {
    struct socket_and_signal *socket_and_signal = (struct socket_and_signal *) arg;
    keypad(stdscr, TRUE);
    while (true) {
        int ch = getch();
        pthread_mutex_lock(&main_mutex);
        switch (ch) {
            case 'q':
                *socket_and_signal->signal = 'q';
                send(socket_and_signal->socket_fd, socket_and_signal->signal, sizeof(char), 0);
                pthread_mutex_unlock(&main_mutex);
                endwin();
                pthread_mutex_destroy(&main_mutex);
                free(socket_and_signal->signal);
                close(socket_and_signal->socket_fd);
                printf("Closed connection on client\n");
                exit(0);
            case KEY_UP:
                *socket_and_signal->signal = 'w';
                send(socket_and_signal->socket_fd, socket_and_signal->signal, sizeof(char), 0);
                break;
            case KEY_DOWN:
                *socket_and_signal->signal = 's';
                send(socket_and_signal->socket_fd, socket_and_signal->signal, sizeof(char), 0);
                break;
            case KEY_LEFT:
                *socket_and_signal->signal = 'a';
                send(socket_and_signal->socket_fd, socket_and_signal->signal, sizeof(char), 0);
                break;
            case KEY_RIGHT:
                *socket_and_signal->signal = 'd';
                send(socket_and_signal->socket_fd, socket_and_signal->signal, sizeof(char), 0);
                break;

        }
        pthread_mutex_unlock(&main_mutex);
    }
}

void generate_map(char map[5][5]){
    pthread_mutex_lock(&main_mutex);
    int offset_y = 1;
    int offset_x = 4;
    for (int i=0; i<5; i++){
        for (int j=0; j<5; j++){
            if (isdigit(map[i][j])){
                mvaddch(i+offset_y, j+offset_x, map[i][j] | A_ALTCHARSET | COLOR_PAIR(2));
            }
            else if (map[i][j] == 'A'){
                mvaddch(i+offset_y, j+offset_x, map[i][j] | COLOR_PAIR(4));
            }
            else if (map[i][j] == 'c' || map[i][j] == 't' || map[i][j] == 'T'){
                mvaddch(i+offset_y, j+offset_x, map[i][j] | COLOR_PAIR(5));
            }
            else if (map[i][j] == '*'){
                mvaddch(i+offset_y, j+offset_x, map[i][j] | COLOR_PAIR(6));
            }
            else if (map[i][j] == 'D'){
                mvaddch(i+offset_y, j+offset_x, map[i][j] | COLOR_PAIR(7));
            }
            else{
                mvaddch(i+offset_y, j+offset_x, map[i][j] | A_ALTCHARSET | COLOR_PAIR(1));
            }
        }
    }
    move(10, 10);
    refresh();
    pthread_mutex_unlock(&main_mutex);
}

void generate_player_info(struct received_t data){
    pthread_mutex_lock(&main_mutex);
    move(0, 20);
    clrtoeol();
    mvprintw(0, 14, "Player ID: %d", data.id);
    move(1, 20);
    clrtoeol();
    mvprintw(1 , 14, "Round: %d", data.game_round);
    move(3, 20);
    clrtoeol();
    mvprintw(3 , 14, "Current X/Y: %d/%d", data.x, data.y);
    move(4, 20);
    clrtoeol();
    mvprintw(4 , 14, "Carried: %d", data.carried);
    move(5, 20);
    mvprintw(5 , 14, "Brought: %d", data.brought);
    move(6, 20);
    mvprintw(6 , 14, "Deaths: %d", data.deaths);
    move(10, 10);
    printw("Press q/Q to quit");
    refresh();
    pthread_mutex_unlock(&main_mutex);
}

void init_colors()
{
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_WHITE); // kolor mapy
    init_pair(2, COLOR_WHITE, COLOR_MAGENTA); //kolor gracza
    init_pair(3, COLOR_WHITE, COLOR_BLACK); // kolor tła
    init_pair(4, COLOR_WHITE, COLOR_RED); // kolor obozu
    init_pair(5, COLOR_BLACK, COLOR_YELLOW); // kolor skarbów
    init_pair(6, COLOR_RED, COLOR_WHITE); // kolor bestii
    init_pair(7, COLOR_WHITE, COLOR_YELLOW); // kolor upuszczonych skarbów
    bkgd(COLOR_PAIR(3));
}