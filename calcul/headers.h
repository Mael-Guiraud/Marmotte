#include <stdio.h> //printf
#include <stdlib.h>
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include "alea.h"
#include "simuls.h"
#include <sys/time.h>


typedef struct seed{
	int seed;
	int nb_elems;
}seed;


typedef struct message{
	int indice_Un;
	int nb_elems;
	Etat x0;
	Etat y0;
} message;

typedef struct reponse{
	Etat x0;
	Etat y0;
} reponse;

typedef Etat trajectoire_partielle[TAILLE_SEQUENCE/(X*NB_MACHINES) +1];




