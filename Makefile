CC = g++

all: prog
prog:
	$(CC) -o mailClient mailClient.cpp -I/mimetic -L/mimetic -lmimetic
clean:
	rm -f mailClient