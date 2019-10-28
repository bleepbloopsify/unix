CC=gcc
FLAGS=-Wall -O1 -Werror --std=c11 -Wno-unused-variable -pthread

all: serv cli

serv: serv.c connmanager.c messages.c
	$(CC) $(FLAGS) serv.c -o serv messages.c connmanager.c

cli: cli.c
	$(CC) $(FLAGS) cli.c -o cli

clean:
	rm -rf $(TARGET)