CFLAGS = -c -g -std=c99 -lpthread -Wall -O2 

all: maitre
	valgrind ./maitre

maitre: main.o operations.o simulation.o thread.o
	gcc -o maitre main.o operations.o thread.o simulation.o -lpthread

main.o: main.c simulation.o
	gcc $(CFLAGS) main.c  

operations.o: operations.c operations.h struct.h ../const.h
	gcc $(CFLAGS) operations.c

simulation.o: simulation.c thread.o
	gcc $(CFLAGS) simulation.c

thread.o: thread.c operations.o 
	gcc $(CFLAGS) thread.c

clean:
	rm -f *.o
	rm -f main
