OS := $(shell uname)
ifeq ($(OS),Darwin)
  CC = gcc-7
else
  CC = gcc
endif



CFLAGS = -c -g -std=c99 -Wall -O2

all: maitre
	./maitre

debug: maitre
	valgrind ./maitre

maitre: main.o operations.o simulation.o thread.o
	$(CC) -o maitre main.o operations.o thread.o simulation.o -lpthread

main.o: main.c simulation.o
	$(CC) $(CFLAGS) main.c

operations.o: operations.c operations.h struct.h ../const.h
	$(CC) $(CFLAGS) operations.c

simulation.o: simulation.c thread.o
	$(CC) $(CFLAGS) simulation.c

thread.o: thread.c operations.o
	$(CC) $(CFLAGS) thread.c

clean:
	rm -f *.o
	rm -f main
