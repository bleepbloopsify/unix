CC = gcc
FLAGS=-Werror -Wall -Wno-unused-variable -std=c99 -O1
OBJS=life.c
TARGET=life
DIST=life.c Makefile
DISTTARGET=hw01.zip

all: $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o $(TARGET)

clean:
	rm -rf $(TARGET) $(DISTTARGET)

dist: life.c Makefile
	rm -rf $(DISTTARGET);
	zip -r $(DISTTARGET) $(DIST)
