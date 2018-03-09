#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
//socket libs
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>


#include "nbq.h"
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

	//Sockets
    int server_socket = 0;
	int master_socket = 0;
    struct sockaddr_in socket_type;
	fd_set readfds;
	int taille_socket_type = sizeof(socket_type);
	int size_buff_send = NB_QUEUES +1;
	int  sendbuf[size_buff_send];
	int size_buff_recv = 24;
	int recvbuf[size_buff_recv];


	//create server socket
    if( (server_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        return(-1);
    }
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, &(int){ 1 }, sizeof(int)) < 0)
	        perror("setsockopt(SO_REUSEADDR) failed");


    if (setsockopt(server_socket, IPPROTO_TCP, TCP_NODELAY, &(int){ 1 }, sizeof(int)) < 0)
        perror("setsockopt(TCP_NODELAY) failed");

  // if (setsockopt(server_socket, IPPROTO_TCP, TCP_QUICKACK, &(int){ 1 }, sizeof(int)) < 0)
    //    perror("setsockopt(TCP_QUICKACK) failed");



    socket_type.sin_family = AF_INET;
    socket_type.sin_addr.s_addr = INADDR_ANY;
    socket_type.sin_port = htons( 8888 );


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



    while(1)
    {
		int fd_max = initialize_set(&readfds, server_socket, master_socket);

        if (select( fd_max + 1 , &readfds , NULL , NULL , NULL) < 0)
        {
            printf("Select Failed");
			return(-1);
        }
		
	    
		//If something happened on the server socket , then its an incoming connection
        if (FD_ISSET(server_socket, &readfds))
        {

            if ((master_socket = accept(server_socket, (struct sockaddr *)&socket_type, (socklen_t*)&taille_socket_type))<0)
			{
                perror("Accept Failed");
                return(-1);
            }

		}
		//If the master socket is here, we have a new message from it.
		else 
		{
			if (FD_ISSET(master_socket, &readfds) )
			{
				
				if( recv(master_socket, &recvbuf, sizeof(int)*size_buff_recv, MSG_WAITALL) <= 0)
	            {
					puts("Connection Closed by MASTER");
					return 0;
					
				}

				//We received a message
				else
				{
				
					if( send(master_socket ,&sendbuf, sizeof(int)*size_buff_send  , 0) < 0)
					 {
					     puts("Send (reply) failed");
					     return 0;
					 }
				}
				//else
			}
		}

    }//while


    close(master_socket);
	close(server_socket);
    return 0;
}
