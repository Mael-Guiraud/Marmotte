#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
//socket libs
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "struct.h"
#include "socket.h"

//return the number of ip red in the IP adress file
char** read_servers_adresses()
{
	FILE * f =fopen ("../addressescalcul","r");

	char** adds= (char **)malloc(sizeof(char*)*NB_MACHINES);
	char trash[4];
	for(int i=0;i<NB_MACHINES;i++)
	{
		adds[i] = (char*)malloc(sizeof(char)*16);
		if(!fgets(adds[i],15,f)) 
		{
			printf("Not enough addresses in file\n");
			exit(27);
		}
		fgets(trash,4,f);
	}

	printf("Lus = \n");
	for(int i=0;i<NB_MACHINES;i++)printf("[%s]",adds[i]);
		printf("\n");
	return adds;

}
void free_adds(char** adds)
{
	for(int i=0;i<NB_MACHINES;i++)
		free(adds[i]);
	free(adds);
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

//Create all the socket and connect them to their respective servers.
int* create_and_connect_sockets()
{
    struct sockaddr_in server;

    int * servers_id; //Socket id from all servers

    server.sin_family = AF_INET; //IPV4 add
    server.sin_port = htons( 8888 ); //port number

	char ** addresses = read_servers_adresses();
	int machine_connect = 0;

    assert(servers_id = (int *) malloc(sizeof(int)*NB_MACHINES));

    /**/


    for(int i=0;i<NB_MACHINES;i++)
    {
		printf("Connection %d/%d\n", i+1, NB_MACHINES);
		// Creation of the socket for the servers
		if ( (servers_id[i] = socket(AF_INET , SOCK_STREAM , 0))== -1)
	        printf("Could not create socket");
		what_do_i_read[i] = PAUSE;

		///////// Sockets options /////////
		//Allow the socket to be re-used after being closed
	    if (setsockopt(servers_id[i], SOL_SOCKET, SO_REUSEPORT, &(int){ 1 }, sizeof(int)) < 0)
	        perror("setsockopt(SO_REUSEADDR) failed");

	    if (setsockopt(servers_id[i], IPPROTO_TCP, TCP_NODELAY, &(int){ 1 }, sizeof(int)) < 0)
	        perror("setsockopt(TCP_NODELAY) failed");

	    /*if (setsockopt(servers_id[i], IPPROTO_TCP, TCP_QUICKACK, &(int){ 1 }, sizeof(int)) < 0)
	        perror("setsockopt(TCP_QUICKACK) failed");*/
	    server.sin_addr.s_addr = inet_addr(addresses[machine_connect]);
	    machine_connect++;
		//Connection to all servers
		if(connect(servers_id[i],(struct sockaddr *) &server, sizeof(struct sockaddr)) == -1)
        {
            perror("Connection failed");
            return NULL;
        }
    }
    free_adds(addresses);

    return servers_id;
}
