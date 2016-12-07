all: maitre
	./maitre >> res

maitre: maitre.c constantes.h
	gcc -o maitre maitre.c -std=c99 -lpthread -Wall 
