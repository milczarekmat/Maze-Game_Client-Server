# List your *.h files (if you do not have them in your project then leave the variable "headers" empty):
headers = server_defs.h server_threads.h beast.h socket_server.h

# List your *.c files:
sources = server.c server_defs.c server_threads.c beast.c socket_server.c

# Specify name of your program:
executable = server

$(executable): $(headers) $(sources)
	gcc -g -Wall -pthread -pedantic $(sources) -o $(executable) -lncurses
	gcc -Wall -pedantic -lncurses -pthread client.c -o client

.PHONY: clean
clean:
	rm $(executable)
	rm client

.PHONY: client
client:
	gcc -Wall -pedantic -lncurses -pthread client.c -o client

.PHONY: check
check: $(executable)
	valgrind --leak-check=full --track-origins=yes ./$(executable)
