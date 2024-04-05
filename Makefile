CC = g++

all: prog
prog:
	$(CC) -o mailClient mailClient.cpp
clean:
	rm -f mailClient