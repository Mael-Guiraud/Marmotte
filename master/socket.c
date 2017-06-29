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
int read_servers_adresses(char ** ip_adresses)
{
	FILE *file = fopen ( "../addressescalcul", "r" );
	char ip_adress[15];	//maximum number of character for an IP adress
	int i = 0;
	if ( file != NULL )
	{
		while ( fgets ( ip_adress, sizeof(ip_adress), file ) != NULL ) /* read a line */
		{
			if (i+1 > NB_MACHINES)
			{
				perror ( "Error while opening the file with the servers IPs adresses.\n Number of servers doesn't match with the numer of IP in the file" );
				fclose ( file );
				return -1;
			}
			else
			{
				ip_adresses[i] = ip_adress;
				i++;
			}
		}
		fclose ( file );
	}
	else
	{
		perror ( "Error while opening the file with the servers IPs adresses" );
		return -1;
	}
	return i;
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
    struct sockaddr_in master;
    struct sockaddr_in* client_addr;
    int * servers_id; //Socket id from all servers

    master.sin_family = AF_INET; //IPV4 add
    master.sin_port = htons( 8888 ); //port number

	if(EXEC_TYPE == 0)
        master.sin_addr.s_addr = inet_addr("127.0.0.1");
    else
    {
		master.sin_addr.s_addr = inet_addr("192.168.90.107");
	}

    assert(client_addr = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in)*NB_MACHINES));
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

		//Connection to all servers
		if(connect(servers_id[i],(struct sockaddr *) &master, sizeof(struct sockaddr)) == -1)
        {
            perror("Connection failed");
            return NULL;
        }
    }
    free(client_addr);

    return servers_id;
}
