CC = g++

all: prog
prog:
	$(CC) mailClient.cpp -o mailClient
clean:
	rm -f mailClient