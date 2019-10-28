CC=gcc
FLAGS=-Wall -O1 -Werror --std=c99 -Wno-unused-variable
TARGET=./sh
OBJS=sh.c

all: $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o $(TARGET)

run:
	PS1='$$ ' ./sh

clean:
	rm -rf $(TARGET)
