server: server.c server_defs.c
	gcc -g -lncurses -pthread server.c server_defs.c server_threads.c -o server

clean:
	rm *.o server