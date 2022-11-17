# List your *.h files (if you do not have them in your project then leave the variable "headers" empty):
headers = server_defs.h server_threads.h beast.h

# List your *.c files:
sources = server.c server_defs.c server_threads.c beast.c

# Specify name of your program:
executable = server

$(executable): $(headers) $(sources)
	gcc -g -Wall -pthread -lncurses -pedantic $(sources) -o $(executable)

.PHONY: clean
clean:
	rm $(executable)

.PHONY: check
check: $(executable)
	valgrind --leak-check=full --track-origins=yes ./$(executable)