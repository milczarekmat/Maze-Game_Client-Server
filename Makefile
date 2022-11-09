server: server.c server_defs.c
	gcc -g -lncurses server.c server_defs.c -o server

clean:
	rm *.o server