CC = g++

all: prog run
prog:
	$(CC) -o mailClient mailClient.cpp
run:
	./mailClient
clean:
	rm -f mailClient