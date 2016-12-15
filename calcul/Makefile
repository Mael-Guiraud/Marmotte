CFLAGS = -c -Wall -g -O3 

all: clean prog
	 ./prog

prog: calcul.o alea.o simuls.o
	gcc -o prog calcul.o alea.o simuls.o -lm

calcul.o: calcul.c simuls.o alea.o
	gcc $(CFLAGS) calcul.c 

alea.o: alea.c
	gcc $(CFLAGS) alea.c 

simuls.o: simuls.c const.h
	gcc $(CFLAGS) simuls.c 

clean:
	rm -f *.o
	rm -f prog
