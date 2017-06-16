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


#include "simuls.h"
#include "operations.h"

#define BORNE_NB 10


typedef struct message{
	int Un_id;
	int nb_elems;
	int * x0;
	int * y0;
} Message;


int main(int argc , char *argv[])
{

	//Sockets
    int server_socket = 0;
	int master_socket = 0;
    struct sockaddr_in socket_type;
	fd_set readfds;
	int taille_socket_type = sizeof(socket_type);


	//Messages Buffers
	int message_size = sizeof(int)*(BORNE_NB*2 + 4);
	int * message;
	assert(message=malloc(message_size));
	int reply_size;
	int * reply = NULL;
	int trajectory_size;
	int * trajectory= NULL;



	//Datas for the simulations
	double load,p,mu;
	int * lower_bound = NULL;
	int * upper_bound = NULL;

	//seeds
	int ** seeds =NULL;

	//Global datas
	int nb_inter;
	int nb_queues = -1;

	//Random sequence
	int* Un;

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

   /* if (setsockopt(server_socket, IPPROTO_TCP, TCP_QUICKACK, &(int){ 1 }, sizeof(int)) < 0)
        perror("setsockopt(TCP_QUICKACK) failed");*/

          
   
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

		//wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        if (select( fd_max + 1 , &readfds , NULL , NULL , NULL) < 0)
        {
            printf("Select Failed");
			return(-1);
        }

		//If something happened on the server socket , then its an incoming connection
        if (FD_ISSET(server_socket, &readfds))
        {
			printf("Demande de connexion du maitre\n");
            if ((master_socket = accept(server_socket, (struct sockaddr *)&socket_type, (socklen_t*)&taille_socket_type))<0)
			{
                perror("Accept Failed");
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
				printf("\n");
				switch (message[0])
				{
					case 0: //New seed
						nb_inter=message[5];
						free_random_sequences(seeds,nb_inter);
						seeds = init_random_sequences(nb_inter);
						break;
					case 1: //BOUNDS
						if(!lower_bound || !upper_bound || !reply)
						{
							printf("Error, The simulation is not initialised\n");
							exit(13);
						}
						if(!seeds)
						{
							printf("Error, the seeds aren't initialised\n");
							exit(14);
						}
						for(int i=0;i<nb_queues;i++)
						{
							lower_bound[i] = message[i+4];
							upper_bound[i] = message[i+4+nb_queues];
						}
						// message[1] = inter id
						// message[2] = inter size
						// message[3] = seed
						Un = gives_un(seeds, message[2],message[1],message[3]);
						printf("[%d %d] [%d %d]\n",message[4],message[5],message[6],message[7]);
						if(!coupling(&message[4],&message[nb_queues+4]))
		    	        {
		    	          	reply[0]=message[1];
		                    
	        			  for(int i=0;i<message[2];i++)
	        				{
	        			  		F(&message[4],Un[i]);
	        			  		F(&message[nb_queues+4],Un[i]);
	        				}
	        	        	cpy_state(&message[4],&reply[1]);
	        	        	cpy_state(&message[nb_queues+4],&reply[nb_queues+1]);
	      
	        		       if( send(master_socket ,reply, reply_size  , 0) < 0)
	        		        {
	        		            puts("Send (reply) failed");
	        		            break;
	        		        }
		    	        }        
		    	        else
		    	        {
		    	        	trajectory_size = sizeof(int)*(message[2]*nb_queues+1);
		    	        	printf("On crée une trajectoire de taille %d\n",trajectory_size);
		    	        	if(!trajectory)
		    	        		assert(trajectory = malloc(trajectory_size));

		    	        	trajectory[0]=message[1];
		    	        	for(int i=0;i<message[2];i++)
		    				{

		    					F(&message[4],Un[i]);
		    					
		    			  		cpy_state(&message[4],&trajectory[1+i*nb_queues]);
		    			  		printf("%d %d %d %d \n",message[4],message[5],trajectory[1+i*nb_queues],trajectory[1+i*nb_queues+1]);
		    				}
		    		
		    				if( send(master_socket ,trajectory, trajectory_size  , 0) < 0)
		    		        {
		    		            puts("Send (trajectory) failed");
		    		            break;
		    		        }
		    			}

						break;
					default://Reset of simul
						nb_queues = message[1];
					    load = intToDoubleLoad(message,4);
						p = intToDouble(message,8);
						mu = intToDouble(message,16);
						init_simul(message[1],message[2],message[3],load,p,mu);
						if(lower_bound)
							free(lower_bound);
						assert(lower_bound = malloc(sizeof(int)*nb_queues));
						if(upper_bound)
							free(upper_bound);
						assert(upper_bound = malloc(sizeof(int)*nb_queues));
						reply_size = sizeof(int)*(nb_queues*2+1);
						if(reply)
							free(reply);
						assert(reply = malloc(reply_size));
						break;
				}
			}
			//else
		}
    }//while
    void free_random_sequences(int** tab,int nb_inter);

    if(lower_bound)
		free(lower_bound);
	if(upper_bound)
		free(upper_bound);
    simulation_mem_free();
    free(message);
    if(reply)
    	free(reply);
     if(trajectory)
    	free(trajectory);

    close(master_socket);
    return 0;
}
