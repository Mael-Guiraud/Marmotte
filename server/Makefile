CFLAGS = -std=c99 -c -Wall -g -O3

all: run

run: compil
	  ./prog

debug: clean compil
	valgrind --leak-check=full --show-reachable=yes --track-fds=yes --track-origins=yes ./prog

compil: calcul.o alea.o simuls.o operations.o
	gcc -o prog calcul.o alea.o simuls.o operations.o -lm -g

calcul.o: calcul.c simuls.o alea.o
	gcc $(CFLAGS) calcul.c

alea.o: alea.c
	gcc $(CFLAGS) alea.c

operations.o: operations.c
	gcc $(CFLAGS) operations.c

simuls.o: simuls.c
	gcc $(CFLAGS) simuls.c

clean:
	rm -f *.o
	rm -f *.gch
	rm -f prog
