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
double time_diff(struct timeval tv1, struct timeval tv2)
{
    return (((double)tv2.tv_sec*(double)1000 +(double)tv2.tv_usec/(double)1000) - ((double)tv1.tv_sec*(double)1000 + (double)tv1.tv_usec/(double)1000));
}

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
	assert(message = (int *)malloc(message_size));
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
	int old_nb_inter;
	int nb_queues = -1;

	//Random sequence
	int* Un;

	int old_traj_size = 0;

	struct timeval tv1, tv2;
	double time_sending_traj=0.0;
	double time_sending_bounds=0.0;
	double time_recv=0.0;
	double time_computing=0.0;
	double time_select=0.0;


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

   if (setsockopt(server_socket, IPPROTO_TCP, TCP_QUICKACK, &(int){ 1 }, sizeof(int)) < 0)
        perror("setsockopt(TCP_QUICKACK) failed");



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
		gettimeofday (&tv1, NULL);
        if (select( fd_max + 1 , &readfds , NULL , NULL , NULL) < 0)
        {
            printf("Select Failed");
			return(-1);
        }
		gettimeofday (&tv2, NULL);
	    time_select += time_diff(tv1,tv2);
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
		else if (FD_ISSET(master_socket, &readfds) )
		{
			//The master ended the connection
			gettimeofday (&tv1, NULL);
			if( recv(master_socket, message, message_size, MSG_WAITALL) <= 0)
            {
				puts("Connection Closed by MASTER");
				old_nb_inter = nb_inter;
				master_socket = 0;
				gettimeofday (&tv2, NULL);
		    	time_recv += time_diff(tv1,tv2);
				
			}

			//We received a message
			else
			{
				for(int l=0;l<message_size/sizeof(int);l++)printf("%d ",message[l]);printf("FIN \n");
				gettimeofday (&tv2, NULL);
		    	time_recv += time_diff(tv1,tv2);
				switch (message[0])
				{
					case 0: //New seed
						nb_inter=message[1];
						if (nb_inter == 0)
							printf("\nComputing time : %lf ms\nTime of bounds sending : %lf ms\nTime of trajectory sending : %lf ms\nReceving time : %lf ms\nSelect time : %lf ms\n\n", time_computing, time_sending_bounds, time_sending_traj, time_recv, time_select);
						else
						{
							free_random_sequences(seeds, old_nb_inter);
							seeds = init_random_sequences(nb_inter);
							old_nb_inter = nb_inter;
						}
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
						printf("%d [%d %d] [%d %d]\n",message[1], message[4],message[5],message[6],message[7]);
						if(!coupling(&message[4],&message[nb_queues+4]))
		    	        {
		    	          	reply[0]=message[1];
		    	          	gettimeofday (&tv1, NULL);
	        			  	for(int i=0;i<message[2];i++)
	        				{
	        			  		F(&message[4],Un[i]);
	        			  		F(&message[nb_queues+4],Un[i]);
	        				}
	        				gettimeofday (&tv2, NULL);
	    					time_computing += time_diff(tv1,tv2);
	        	        	cpy_state(&message[4],&reply[1]);
	        	        	cpy_state(&message[nb_queues+4],&reply[nb_queues+1]);
	        		       if( send(master_socket ,reply, reply_size  , 0) < 0)
	        		        {
	        		            puts("Send (reply) failed");
	        		            break;
	        		        }
	        		        gettimeofday (&tv1, NULL);
	    					time_sending_bounds += time_diff(tv2,tv1);
		    	        }
		    	        else
		    	        {
		    	        	trajectory_size = sizeof(int)*(message[2]*nb_queues+1);
		    	        	if(old_traj_size != 0)
		    	        	{
		    	        		if(old_traj_size != trajectory_size)
		    	        		{
		    	        			free(trajectory);
		    	        			trajectory = NULL;
		    	        			old_traj_size = trajectory_size;
		    	        		}
		    	        	}
		    	        	else
		    	        	{
		    	        		old_traj_size = trajectory_size;
		    	        	}

		    	        	if(!trajectory)
		    	        		assert(trajectory = (int *)malloc(trajectory_size));

		    	        	trajectory[0]=message[1];
		    	        	gettimeofday (&tv1, NULL);
		    	        	for(int i=0;i<message[2];i++)
		    				{

		    					F(&message[4],Un[i]);

		    			  		cpy_state(&message[4],&trajectory[1+i*nb_queues]);
		    				}
		    				gettimeofday (&tv2, NULL);
		    				time_computing  += time_diff(tv1,tv2);
		    				if( send(master_socket ,trajectory, trajectory_size  , 0) < 0)
		    		        {
		    		            puts("Send (trajectory) failed");
		    		            break;
		    		        }
		    		           gettimeofday (&tv1, NULL);
	    					time_sending_traj += time_diff(tv2,tv1);
		    			}

						break;
					default://Reset of simul
						printf("Configuration message\n");
						nb_queues = message[1];
					    load = intToDoubleLoad(message,4);
						p = intToDouble(message,8);
						mu = intToDouble(message,16);
						init_simul(message[1],message[2],message[3],load,p,mu);
						if(lower_bound)
							free(lower_bound);
						assert(lower_bound = (int *) malloc(sizeof(int)*nb_queues));
						if(upper_bound)
							free(upper_bound);
						assert(upper_bound = (int *) malloc(sizeof(int)*nb_queues));
						reply_size = sizeof(int)*(nb_queues*2+1);
						if(reply)
							free(reply);
						assert(reply = (int *) malloc(reply_size));
						break;
				}
			}
			//else
		}
		else
			printf("Empty select\n");
    }//while
    free_random_sequences(seeds, old_nb_inter);

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
	close(server_socket);
    return 0;
}
