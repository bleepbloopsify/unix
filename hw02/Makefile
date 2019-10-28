CC = gcc
FLAGS=-Werror -Wall -Wno-unused-variable -std=c99 -O1
OBJS=env.c
TARGET=env
DIST=env.c Makefile
DISTTARGET=hw02.zip

all: $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o $(TARGET)

clean:
	rm -rf $(TARGET) $(DISTTARGET)

dist: life.c Makefile
	rm -rf $(DISTTARGET);
	zip -r $(DISTTARGET) $(DIST)
