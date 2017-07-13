#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
//socket libs
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "struct.h"
#include "socket.h"
#include "operations.h"

//return the number of ip red in the IP adress file
char** read_servers_adresses(int nb_machines)
{
	FILE * f =fopen ("../addressescalcul","r");

	char** adds= (char **)malloc(sizeof(char*)*nb_machines);
	char trash[4];
	for(int i=0;i<nb_machines;i++)
	{
		adds[i] = (char*)malloc(sizeof(char)*16);
		if(!fgets(adds[i],15,f))
		{
			printf("Not enough addresses in file\n");
			exit(27);
		}
		fgets(trash,4,f);
	}
	fclose(f);
	printf("Connection to -> ");
	for(int i=0;i<nb_machines;i++)printf("%d:[%s]",i,adds[i]);
		printf("\n");
	return adds;

}

//initialize a set for the select function
int initialize_set(fd_set *set, int nb_servers, int *servers_id)
{
	FD_ZERO(set);
	int max_sd = 0;
	for ( int i = 0 ; i < nb_servers ; i++)
	{
		//socket descriptor
		int sd = servers_id[i];

		//if valid socket descriptor then add to read list
		if(sd > 0)
			FD_SET( sd , set);

		//highest file descriptor number, need it for the select function
		if(sd > max_sd)
			max_sd = sd;
	}
	return max_sd;
}

void free_adds(char** adds, int nb_machines)
{
	for(int i=0;i<nb_machines;i++)
		free(adds[i]);
	free(adds);
}
//Create all the socket and connect them to their respective servers.
int* create_and_connect_sockets(int nb_machines)
{
    struct sockaddr_in server;

    int * servers_id; //Socket id from all servers

    server.sin_family = AF_INET; //IPV4 add
    server.sin_port = htons( 8888 ); //port number

	char ** addresses = read_servers_adresses(nb_machines);
	int machine_connect = 0;

    assert(servers_id = (int *) malloc(sizeof(int)*nb_machines));

    /**/


    for(int i=0;i<nb_machines;i++)
    {
		printf("Connection %d/%d\n", i+1, nb_machines);
		// Creation of the socket for the servers
		if ( (servers_id[i] = socket(AF_INET , SOCK_STREAM , 0))== -1)
	        printf("Could not create socket");

		///////// Sockets options /////////
		//Allow the socket to be re-used after being closed
	    if (setsockopt(servers_id[i], SOL_SOCKET, SO_REUSEPORT, &(int){ 1 }, sizeof(int)) < 0)
	        perror("setsockopt(SO_REUSEADDR) failed");

	    if (setsockopt(servers_id[i], IPPROTO_TCP, TCP_NODELAY, &(int){ 1 }, sizeof(int)) < 0)
	        perror("setsockopt(TCP_NODELAY) failed");

	    //if (setsockopt(servers_id[i], IPPROTO_TCP, TCP_QUICKACK, &(int){ 1 }, sizeof(int)) < 0)
	      //  perror("setsockopt(TCP_QUICKACK) failed");
	    server.sin_addr.s_addr = inet_addr(addresses[machine_connect]);
	    machine_connect++;
		//Connection to all servers
		if(connect(servers_id[i],(struct sockaddr *) &server, sizeof(struct sockaddr)) == -1)
        {
            perror("Connection failed");
            return NULL;
        }
    }
    free_adds(addresses,nb_machines);

    return servers_id;
}

//create a bounds message
void build_bounds_message(int *message, Bounds *bounds, int interval, int interval_size, int seed, int nb_queues)
{
	message[0] = 1;		//BOUNDS
	message[1] = interval;
	message[2] = interval_size;
	message[3] = seed;

	cpy_state(bounds[interval].lb,&message[4],nb_queues);
	cpy_state(bounds[interval].ub,&message[4+nb_queues],nb_queues);
}

void build_seed_message(int *message, int nb_interval)
{
	message[0] = 0;
	message[1] = nb_interval;
}

//Destroy all the socket used by the servers
void destroy_sockets(int * sockets, int nb_machines)
{
	for (int i=0; i<nb_machines; i++)
		close(sockets[i]);
}

void ask_for_time_display(int *message,int message_size,int *servers_id, int nb_machines)
{
	message[0] = 0;
	message[1] = 0;
	for(int i=0; i<nb_machines; i++)
	{
		if( send(servers_id[i], message, message_size, 0) < 0 )
		{
			perror("send() asko for diaplay time");
		}
	}
}


void send_config(int * message,int message_size, int * servers_id, int nb_machines, int min, int max, float load, float p, float mu, int nb_queues)
{	
	message[0] = 2;
	message[1] = nb_queues;
	message[2] = min;
	message[3] = max;
	float f[3];
	f[0] = load;
	f[1] = p;
	f[2] = mu;
	memcpy(&message[4],f,sizeof(f));
	//floatTointLoad(load, &message[4]);// The load must be <= 1 and only have 2 decimals
	//floatToint(p, &message[8]);// must be <= 999, and only have 4 decimals
	//floatToint(mu, &message[16]);// must be <= 999, and only have 4 decimals

	//Send to all machines
	for(int i=0; i<nb_machines; i++)
	{
		if( send(servers_id[i], message, message_size, 0) < 0 )
		{
		    perror("send()");
		    exit(78);
		}
	}

}


void send_exit(int * message,int message_size, int * servers_id, int nb_machines)
{	
	message[0] = 3;

	for(int i=0; i<nb_machines; i++)
	{
		if( send(servers_id[i], message, message_size, 0) < 0 )
		{
		    perror("send()");
		    exit(78);
		}
	}

}
