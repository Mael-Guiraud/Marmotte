CFLAGS = -Wall -g -O2

all: prog
	 ./prog

prog: calcul.o alea.o simuls.o
	gcc -o prog calcul.o alea.o simuls.o 

calcul.o: calcul.c headers.h constantes.h
	gcc -c calcul.c $(CFLAGS)

alea.o: alea.c alea.h
	gcc -c -Wall alea.c $(CFLAGS)

simuls.o: simuls.c simuls.h constantes.h
	gcc -c -Wall simuls.c $(CFLAGS)

clean:
	rm -f *.o
	rm -f prog
