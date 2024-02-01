CC=gcc
CFLAGS=-I.
DEPS = proxy.h
OBJ = proxy.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -pthread

compile: P1 P2 P3

P1: $(OBJ) P1.o 
	$(CC) -o P1 $(OBJ) P1.o

P2: $(OBJ) P2.o 
	$(CC) -o P2 $(OBJ) P2.o

P3: $(OBJ) P3.o 
	$(CC) -o P3 $(OBJ) P3.o

clean:
	rm -f P1
	rm -f P2
	rm -f P3