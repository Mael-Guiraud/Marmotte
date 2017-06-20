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
	printf("Sockets Opening...\n");
    if(!(servers_id = create_sockets()))
    {
        printf("Erreur while creating sockets\n");
        exit(1);
    }
	printf("Sockets Opened...\n");

	fd_set readfds;
	int message[24];
	int taille_message = sizeof(message);

	for(int i=0; i<NB_MACHINES; i++)
	{
		int fd_max = initialize_set(&readfds, servers_id);


		message[0] = 2;
		message[1] = 2;
		message[2] = 0;
		message[3] = 100;
		//load is splitted in X.XX
		message[4] = 1;
		message[5] = 0;
		message[6] = 0;
		message[7] = 0;

		//rho is splitted in XXX.XXXX
		message[8] = 0;
		message[9] = 0;
		message[10] = 0;
		message[11] = 0;
		message[12] = 7;
		message[13] = 5;
		message[14] = 0;
		message[15] = 0;

		//mu is splitted in XXX.XXXX
		message[16] = 3;
		message[17] = 0;
		message[18] = 0;
		message[19] = 0;
		message[20] = 0;
		message[21] = 0;
		message[22] = 0;
		message[23] = 0;

		printf("Envoie de message...de taille %d \n",taille_message);
		if( send(servers_id[i], message, taille_message, 0) < 0 )
		{
		    perror("send()");
		    return(-1);
		}
		printf("Message envoyé!\n");


		

		message[0] = 1;
		message[1] = 0;
		message[2]= 10;
		message[3] = time(NULL); 
		for(int i=0;i< NB_QUEUES ;i++)
		{
			message[4+i] = 0;
		}
		for(int i=0;i< NB_QUEUES ;i++)
		{
			message[4+NB_QUEUES+i] = 100;
		}

		printf("Envoie de message...\n");
		if( send(servers_id[i], message, taille_message, 0) < 0 )
		{
		    perror("send()");
		    return(-1);
		}
		printf("Message envoyé!\n");
		printf("En attente d'une réponse du server\n");
		int taille_reponse = sizeof(int)*(2*NB_QUEUES+1);

		recv(servers_id[i], message, taille_reponse, 0);

		for(int i=0;i<2*NB_QUEUES+1;i++)printf("%d ",message[i]);
			printf("\n");



		message[0] = 1;
		message[1] = 0;
		message[2]= 10;
		message[3] = time(NULL); 
		for(int i=0;i< NB_QUEUES ;i++)
		{
			message[4+i] = 50;
		}
		for(int i=0;i< NB_QUEUES ;i++)
		{
			message[4+NB_QUEUES+i] = 50;
		}

		printf("Envoie de message...\n");
		if( send(servers_id[i], message, taille_message, 0) < 0 )
		{
		    perror("send()");
		    return(-1);
		}
		printf("Message envoyé!\n");
		printf("En attente d'une réponse du server\n");
		taille_reponse = sizeof(int)*(10*NB_QUEUES+1);
		int buffer[200];
		recv(servers_id[i], buffer, taille_reponse, 0);

		for(int i=0;i<10*NB_QUEUES+1;i++)printf("%d ",buffer[i]);
			printf("\n");

		message[0] = 2;
		message[1] = 10;
		message[2] = 0;
		message[3] = 100;
		//load is splitted in X.XX
		message[4] = 0;
		message[5] = 0;
		message[6] = 0;
		message[7] = 0;

		//rho is splitted in XXX.XXXX
		message[8] = 0;
		message[9] = 0;
		message[10] = 0;
		message[11] = 0;
		message[12] = 7;
		message[13] = 5;
		message[14] = 0;
		message[15] = 0;

		//mu is splitted in XXX.XXXX
		message[16] = 3;
		message[17] = 0;
		message[18] = 0;
		message[19] = 0;
		message[20] = 0;
		message[21] = 0;
		message[22] = 0;
		message[23] = 0;


int nb = 10;
		printf("Envoie de message...de taille %d \n",taille_message);
		if( send(servers_id[i], message, taille_message, 0) < 0 )
		{
		    perror("send()");
		    return(-1);
		}
		printf("Message envoyé!\n");
		message[0] = 0;
		message[1] = 5;	//We send a seed
		//message[0] = 1;	message[1] = 20; message[2] = 99; //We send new bounds
		//message[0] = 2;	//We ask for a new configuration

	
		printf("Envoie de message...\n");
		if( send(servers_id[i], message, taille_message, 0) < 0 )
		{
		    perror("send()");
		    return(-1);
		}
		printf("Message envoyé!\n");

		message[0] = 1;
		message[1] = 0;
		message[2]= 100;
		message[3] = time(NULL); 
		for(int i=0;i< nb ;i++)
		{
			message[4+i] = 0;
		}
		for(int i=0;i< nb ;i++)
		{
			message[4+nb+i] = 100;
		}

		printf("Envoie de message...\n");
		if( send(servers_id[i], message, taille_message, 0) < 0 )
		{
		    perror("send()");
		    return(-1);
		}
		printf("Message envoyé!\n");
		printf("En attente d'une réponse du server\n");
		 taille_reponse = sizeof(int)*(2*nb+1);

		recv(servers_id[i], message, taille_reponse, 0);

		for(int i=0;i<2*nb+1;i++)printf("%d ",message[i]);
			printf("\n");



		message[0] = 1;
		message[1] = 0;
		message[2]= 100;
		message[3] = time(NULL); 
		for(int i=0;i< nb ;i++)
		{
			message[4+i] = 50;
		}
		for(int i=0;i< nb ;i++)
		{
			message[4+nb+i] = 50;
		}

		printf("Envoie de message...\n");
		if( send(servers_id[i], message, taille_message, 0) < 0 )
		{
		    perror("send()");
		    return(-1);
		}
		printf("Message envoyé!\n");
		printf("En attente d'une réponse du server\n");
		taille_reponse = sizeof(int)*(100*nb+1);
		 buffer[2000];
		recv(servers_id[i], buffer, taille_reponse, 0);

		for(int i=0;i<10*nb+1;i++)printf("%d ",buffer[i]);
			printf("\n");
		
	}

    free(servers_id);

    return 0;
}
