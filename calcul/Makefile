all: prog
	./prog

prog: calcul.o alea.o simuls.o
	gcc -o prog calcul.o alea.o simuls.o 

calcul.o: calcul.c headers.h constantes.h
	gcc -c calcul.c

alea.o: alea.c alea.h
	gcc -c -Wall alea.c

simuls.o: simuls.c simuls.h constantes.h
	gcc -c -Wall simuls.c

clean:
	rm -f *.o
	rm -f prog
