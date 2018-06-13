SHELL = /bin/sh

CC = g++
CFLAGS = -Wall -Wextra -pedantic -Wshadow -funroll-loops -DNDEBUG -O3 -std=c++0x -march=native
#CFLAGS = -Wall -Wextra -pedantic -Wshadow -pthread -std=c++0x -g2

all: test

test:
	$(CC) $(CFLAGS) -o test main.cpp

clean:
	/bin/rm -f test
