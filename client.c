#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <ncurses.h>
#include <pthread.h>
#include <ctype.h>
#include <sys/un.h>
#define PORT 8080
#define PORT1 8081
#define BACKLOG 5
#define SERVER_PATH "/tmp/server"

void * event_handler(void * arg);

struct received_t{
    int x;
    int y;
    char player_map[5][5];
    unsigned int game_round;
    unsigned int carried;
    unsigned int brought;
    unsigned char id;
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


int main(int argc, char** argv){
    SA_IN server_address;
    int option = 1;
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1){
        perror("Error with creating socket\n");
        return 1;
    }
    //setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    //setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    //server_address.sin_addr.s_addr = htonl(INADDR_ANY);
//    server_address.sin_port = 0;
    if (argc == 1){
        server_address.sin_port = htons(PORT);
    }
    else{
        server_address.sin_port = htons(PORT1);
    }

/*    if (bind(socket_fd, (SA*)&server_address, sizeof(server_address)) == -1){
        endwin();
        perror("Error with binding socket\n");
        exit(1);
    }*/
/*    struct sockaddr_un serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sun_family = AF_UNIX;
    strcpy(serveraddr.sun_path, SERVER_PATH);*/

    //connect(socket_fd, (struct sockaddr *)&serveraddr, SUN_LEN(&serveraddr))
    if (connect(socket_fd, (SA*)&server_address, sizeof(server_address)) == -1) {
        perror("Connection with the server failed...\n");
        return 2;
    }
    else{
        printf("Connected to the server..\n");
    }

/*    char server_sign;
    long connection_check = recv(socket_fd, &server_sign, sizeof(char), 0);
    if (connection_check == 0){
        printf("Pełny serwer\n");
        close(socket_fd);
        exit(1);
    }*/


    struct socket_and_signal socket_and_signal;
    socket_and_signal.socket_fd = socket_fd;
    socket_and_signal.signal = malloc(sizeof(char));

    if (!socket_and_signal.signal){
        endwin();
        exit(2);
    }
    *socket_and_signal.signal = 'x';

    initscr();
    mvprintw(10, 10, "Socket fd: %d", socket_fd);
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
/*        printw("Waiting for mess\n");
        refresh();*/
        long check = recv(socket_fd, &player_data, sizeof(struct received_t), 0);
        //printw("Got mess\n");

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

/*        pthread_mutex_lock(&main_mutex);
        long check_quit = send(socket_and_signal.socket_fd, socket_and_signal.signal, sizeof(char), 0);
        if (*socket_and_signal.signal == 'q'){
            close(socket_and_signal.socket_fd);
            endwin();
            pthread_mutex_destroy(&main_mutex);
            free(socket_and_signal.signal);
            printf("Closed connection on client\n");
            exit(0);
        }
        *socket_and_signal.signal = 'x';
        pthread_mutex_unlock(&main_mutex);*/
    }
    while (true);
}


void * event_handler(void * arg) {
    struct socket_and_signal *socket_and_signal = (struct socket_and_signal *) arg;
    //scrollok(stdscr, TRUE);
    keypad(stdscr, TRUE);
    noecho();
    while (true) {
        int ch = getch();
        pthread_mutex_lock(&main_mutex);
        switch (ch) {
            case 'q':
                *socket_and_signal->signal = 'q';
                send(socket_and_signal->socket_fd, socket_and_signal->signal, sizeof(char), 0);
                //close(socket_and_signal->socket_fd);
                pthread_mutex_unlock(&main_mutex);
                endwin();
                pthread_mutex_destroy(&main_mutex);
                free(socket_and_signal->signal);

                printf("Closed connection on client\n");
                exit(0);

                //pthread_exit();
                break;
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
    start_color();
    noecho();
    // TODO ponizsze do funkcji
    init_pair(1, COLOR_GREEN, COLOR_WHITE); // kolor mapy
    init_pair(2, COLOR_WHITE, COLOR_MAGENTA); //kolor gracza
    init_pair(3, COLOR_WHITE, COLOR_BLACK); // kolor tła
    init_pair(4, COLOR_WHITE, COLOR_RED); // kolor obozu
    init_pair(5, COLOR_BLACK, COLOR_YELLOW); // kolor skarbów
    init_pair(6, COLOR_RED, COLOR_WHITE); // kolor bestii
    init_pair(7, COLOR_WHITE, COLOR_YELLOW); // kolor upuszczonych skarbów
    bkgd(COLOR_PAIR(3));

    for (int i=0; i<5; i++){
        for (int j=0; j<5; j++){
/*            if (map[i][j] == (char)(game->number_of_players + '0')){
                mvaddch(i, j, map[i][j] | A_ALTCHARSET | COLOR_PAIR(2));
            }*/
            if (isdigit(map[i][j])){
                mvaddch(i, j, map[i][j] | A_ALTCHARSET | COLOR_PAIR(2));
            }
            else if (map[i][j] == 'A'){
                mvaddch(i, j, map[i][j] | COLOR_PAIR(4));
            }
            else if (map[i][j] == 'c' || map[i][j] == 't' || map[i][j] == 'T'){
                mvaddch(i, j, map[i][j] | COLOR_PAIR(5));
            }
            else if (map[i][j] == '*'){
                mvaddch(i, j, map[i][j] | COLOR_PAIR(6));
            }
            else if (map[i][j] == 'D'){
                mvaddch(i, j, map[i][j] | COLOR_PAIR(7));
            }
/*            else if (map[i][j] == 'b'){
                mvaddch(i, j, map[i][j]  | COLOR_PAIR(8));
            }*/
            else{
                mvaddch(i, j, map[i][j] | A_ALTCHARSET | COLOR_PAIR(1));
            }
            //TODO koloruj gracza wg id
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
    mvprintw(0, 20, "Player ID: %d", data.id);
    move(2, 20);
    clrtoeol();
    mvprintw(2 , 20, "Current X/Y: %d/%d", data.x, data.y);
    move(2, 20);
    clrtoeol();
    mvprintw(2 , 20, "Current X/Y: %d/%d", data.x, data.y);
/*
    move(6, WIDTH + (size * j));
    clrtoeol();
    mvprintw(6 , WIDTH + (size * j), "Carried: %d", (game->players + i)->carried);
    move(8, WIDTH + (size * j));
    clrtoeol();
    mvprintw(8 , WIDTH + (size * j), "Brought: %d", (game->players + i)->brought);
    move(10, WIDTH + (size * j));
    clrtoeol();
    mvprintw(10, WIDTH + (size * j), "Deaths: %d", (game->players + i)->deaths);
    move(12, WIDTH + (size * j));
    clrtoeol();
    mvprintw(12, WIDTH + (size * j), "Round number: %d", game->rounds);
    mvprintw(18 , WIDTH + (size * j), "Press q/Q to quit");
*/

    move(0, 0);
    refresh();
    pthread_mutex_unlock(&main_mutex);
}