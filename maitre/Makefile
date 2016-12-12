CFLAGS = -g -std=c99 -lpthread -Wall -O2

all: maitre
	 ./maitre

maitre: maitre.o fonctions.o simul.o thread.o
	gcc -o maitre maitre.o fonctions.o thread.o simul.o -lpthread

maitre.o: maitre.c simul.o
	gcc -c $(CFLAGS) maitre.c  

fonctions.o: fonctions.c fonctions.h headers.h constantes.h
	gcc -c $(CFLAGS) fonctions.c

simul.o: simul.c thread.o
	gcc -c $(CFLAGS) simul.c

thread.o: thread.c fonctions.o 
	gcc -c $(CFLAGS) thread.c

clean:
	rm -f *.o
	rm -f maitre
