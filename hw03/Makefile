CC = gcc
FLAGS=-Werror -Wall -Wno-unused-variable -std=c99 -O1
OBJS=du.c
TARGET=du

all: $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o $(TARGET)

clean:
	rm -rf $(TARGET) $(DISTTARGET)
