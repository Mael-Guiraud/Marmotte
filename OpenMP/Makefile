CFLAGS = -std=c99 -c -O2 -Wall -g -fopenmp

all: run

run: link
	  ./parsim

debug: clean link
	valgrind --leak-check=full --show-reachable=yes --track-fds=yes --track-origins=yes ./parsim

link: parsim.o alea.o fsim.o 
	gcc -o parsim parsim.o alea.o fsim.o -g -fopenmp

parsim.o: parsim.c 
	gcc $(CFLAGS) parsim.c

alea.o: alea.c
	gcc $(CFLAGS) alea.c

fsim.o: fsim.c
	gcc $(CFLAGS) fsim.c

clean:
	rm -f *.o
	rm -f *.gch
	rm -f prog
