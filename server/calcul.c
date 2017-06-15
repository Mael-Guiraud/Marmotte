#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>
//socket libs
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "const.h"
#include "alea.h"
#include "simuls.h"


typedef struct message{
	int Un_id;
	int nb_elems;
	int * x0;
	int * y0;
} Message;

double time_diff(struct timeval tv1, struct timeval tv2)
{
    return (((double)tv2.tv_sec*(double)1000 +(double)tv2.tv_usec/(double)1000) - ((double)tv1.tv_sec*(double)1000 + (double)tv1.tv_usec/(double)1000));
}

int initialize_set(fd_set *set, int server_socket, int master_socket)
{
	FD_ZERO(set);
	int max_sd = server_socket;

	//if valid socket descriptor then add to read list
	FD_SET(server_socket, set);
	if(master_socket > 0)
		FD_SET( master_socket , set);

	//highest file descriptor number, need it for the select function
	if(master_socket > max_sd)
		max_sd = master_socket;
	return max_sd;
}

int main(int argc , char *argv[])
{
	struct timeval tv1, tv2;
    double tps_e=0.0;
    double tps_c=0.0;
    double tps_r=0.0;

    int server_socket = 0;
	int master_socket = 0;
    struct sockaddr_in socket_type;
	fd_set readfds;

	//create server socket
    if( (server_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        return(-1);
    }

    if (setsockopt(server_socket, IPPROTO_TCP, TCP_NODELAY, &(int){ 1 }, sizeof(int)) < 0)
        perror("setsockopt(TCP_NODELAY) failed");

    if (setsockopt(server_socket, IPPROTO_TCP, TCP_QUICKACK, &(int){ 1 }, sizeof(int)) < 0)
        perror("setsockopt(TCP_QUICKACK) failed");

    if(EXEC_TYPE == 0)
        socket_type.sin_addr.s_addr = inet_addr("127.0.0.1");
    else
        socket_type.sin_addr.s_addr = inet_addr("193.51.25.106");

    socket_type.sin_family = AF_INET;
    socket_type.sin_port = htons( 8888 );
	int taille_socket_type = sizeof(socket_type);

	if (bind(server_socket, (struct sockaddr *)&socket_type, sizeof(socket_type))<0)
    {
        perror("bind failed");
        return(-1);
    }

	if (listen(server_socket, 5) < 0)
    {
        perror("listen");
        return(-1);
    }
	Message m;

    int message_size = sizeof(int)*(NB_QUEUES*2 + 4);
    int reply_size = sizeof(int)*(NB_QUEUES*2+1);
    int interval_size;

	assert(m.x0 = malloc(sizeof(int)*NB_QUEUES));
	assert(m.y0 = malloc(sizeof(int)*NB_QUEUES));

	int * message;
	assert(message=malloc(message_size));
	int * reply;
	assert(reply = malloc(reply_size));
	int * trajectory;

	//Initialisation de la fonction de calcul
    InitDistribution(LOAD);
	InitRectangle();
	double * sequence =NULL;
	int i;
    int nb_elems;

	int borne_inf, borne_sup;
    while(1)
    {
		gettimeofday (&tv1, NULL);

		int fd_max = initialize_set(&readfds, server_socket, master_socket);

		//wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        if (select( fd_max + 1 , &readfds , NULL , NULL , NULL) < 0)
        {
            printf("select error");
			return(-1);
        }

		//If something happened on the server socket , then its an incoming connection
        if (FD_ISSET(server_socket, &readfds))
        {
			printf("Demande de connexion du maitre\n");
            if ((master_socket = accept(server_socket, (struct sockaddr *)&socket_type, (socklen_t*)&taille_socket_type))<0)
			{
                perror("accept");
                return(-1);
            }
		}
		//If the master socket is here, we have a new message from it.
		else if (FD_ISSET(master_socket, &readfds) )
		{
			//The master ended the connection
			if( recv(master_socket, message, message_size, 0) <= 0)
            {
				puts("Connection Closed by MASTER");
				master_socket = 0;
			}
			//We received a message
			else
			{
				printf("J'ai recu un message du maitre\n");
				switch (message[0])
				{
					case 0: //New seed
						printf("Réception d'une nouvelle graine\n");
						break;
					case 1: //BOUNDS
						borne_inf = message[1];
						borne_sup = message[2];
						printf("Réception de la borne inférieure : %d et de la borne supérieure : %d\n", borne_inf, borne_sup);

						borne_inf = 30;
						borne_sup = 90;

						//Création de la réponse
						reply[0] = message[1];
						reply[1] = borne_inf;
						reply[2] = borne_sup;

						printf("Envoie d'une réponse au maitre\n");
						if( send(master_socket ,reply, reply_size, 0) < 0)
        		        {
        		            puts("Send (reply) failed");
        		            break;
        		        }
						break;

					default: //New configuration
						printf("Demande d'une nouvelle configuration\n");
						break;
				}
			}
			//else
		}
    }//while
    printf("Time spent in calulations = %f \n",tps_c);
    printf("Time spent in emissions = %f \n",tps_e);
    printf("Time spent in receptions+wait = %f \n",tps_r);
    free(m.x0);
	free(reply);
    free(m.y0);
    free(message);
    free(sequence);

    close(master_socket);
    return 0;
}
