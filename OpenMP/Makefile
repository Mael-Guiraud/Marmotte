OS := $(shell uname)
ifeq ($(OS),Darwin)
  CC = gcc-7
else
  CC = gcc-7
endif
CFLAGS = -std=c99 -c -g -O2 -Wall -fopenmp 

all: run

run: clean link
	  ./parsim

debug: clean link
	valgrind --leak-check=full --show-reachable=yes --track-fds=yes --track-origins=yes ./parsim

link: parsim.o alea.o fsim.o 
	$(CC) -o parsim parsim.o alea.o fsim.o -g -fopenmp -lm

parsim.o: parsim.c 
	$(CC) $(CFLAGS) parsim.c

alea.o: alea.c
	$(CC) $(CFLAGS) alea.c

fsim.o: fsim.c
	$(CC) $(CFLAGS) fsim.c


profile: clean link
	valgrind --tool=callgrind ./parsim
	kcachegrind

clean:
	rm -f *.o
	rm -f *.gch
	rm -f prog

