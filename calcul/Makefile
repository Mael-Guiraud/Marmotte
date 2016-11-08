all: prog
	./prog

prog: calcul.o 
	gcc -o prog calcul.o 

calcul.o: calcul.c headers.h
	gcc -c -Wall calcul.c
