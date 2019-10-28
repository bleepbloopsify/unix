CC=gcc
FLAGS=-Wall -O1 -Werror --std=c11 -Wno-unused-variable

all: serv cli

serv: serv.c
	$(CC) $(FLAGS) serv.c -o serv

cli: cli.c
	$(CC) $(FLAGS) cli.c -o cli

clean:
	rm -rf $(TARGET)
