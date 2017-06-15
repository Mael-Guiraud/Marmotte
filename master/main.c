#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>

#include "struct.h"
#include "thread.h"
#include "operations.h"
#include "simulation.h"

int initialize_set(fd_set *set, int *servers_sockets)
{
	FD_ZERO(set);
	int max_sd = 0;
	for ( int i = 0 ; i < NB_MACHINES ; i++)
	{
		//socket descriptor
		int sd = servers_sockets[i];

		//if valid socket descriptor then add to read list
		if(sd > 0)
			FD_SET( sd , set);

		//highest file descriptor number, need it for the select function
		if(sd > max_sd)
			max_sd = sd;
	}
	return max_sd;
}

int main(int argc , char *argv[])
{

    int * servers_id;

    srand(time(NULL));
    //creating sockets
	printf("Création des sockets...\n");
    if(!(servers_id = create_sockets()))
    {
        printf("Erreur while creating sockets\n");
        exit(1);
    }
	printf("Sockets créés...\n");

	fd_set readfds;
	int message[3];
	for(int i=0; i<NB_MACHINES; i++)
	{
		int fd_max = initialize_set(&readfds, servers_id);
		printf("i : %d\n", i);
		//message[0] = 0;	//We send a seed
		message[0] = 1;	message[1] = 20; message[2] = 99; //We send new bounds
		//message[0] = 2;	//We ask for a new configuration

		int taille_message = sizeof(message);
		printf("Envoie de message...\n");
		if( send(servers_id[i], message, taille_message, 0) < 0 )
		{
		    perror("send()");
		    return(-1);
		}
		printf("Message envoyé!\n");

		printf("En attente d'une réponse du server\n");
		select(fd_max+1, &readfds, NULL, NULL, NULL);
		for (int i=0; i<NB_MACHINES; i++)
		{
			int sd = servers_id[i];
			if ( FD_ISSET(sd, &readfds) )
			{
				recv(sd, message, taille_message, 0);
				printf("Réponse : \n0 : [%d], 1 : [%d], 2 : [%d]\n", message[0], message[1], message[2]);
			}
		}
	}

    free(servers_id);

    return 0;
}
