#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
//socket libs
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "operations.h"
#include "struct.h"
#include "thread.h"

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

//wait for all threads to be closed
void wait_all_threads_close()
{

    for(int i=0;i<NB_MACHINES;i++)
    {
        while(thread_activity[i]==OPEN);
    }
}

//wait the reply of all threads
void wait_threads(int nb_inter)
{
    for(int i=0;i<nb_inter;i++)
    {
        while(interval_state[i]==SENT);
    }
}

//Wait for the server's replys
void *server_listener(void *arg)
{
    argument a = *(argument*)arg;

    int interval_id;
    int i;
    int * bounds ;
    int * partial_trajectory;

    int bounds_bytes_size = sizeof(int)*(NB_QUEUES*2+1);
    int trajectory_bytes_size = sizeof(int)*(interval_size*NB_QUEUES+1);
    assert(bounds= malloc(bounds_bytes_size));
    assert(partial_trajectory= malloc(trajectory_bytes_size));

    while(1)
    {

        //Waiting for something to read
        while(what_do_i_read[a.id_machine]==PAUSE)
        {
            if(quit_threads)
            {
                free(bounds);
                free(partial_trajectory);
                //thread_activity[a.id_machine]=CLOSED;
                pthread_exit(NULL);
            }
        }

        if(what_do_i_read[a.id_machine] == BOUNDS)//Reception of two bounds
        {
            if( recv(a.id_socket , bounds , bounds_bytes_size, MSG_WAITALL) <= 0)
            {
                printf("Connection Closed by server %d",a.id_machine);
                break;
            }
            interval_id = bounds[0]/(interval_size);

            switch(MOD)
            {
                case 1:
                    if(better(&bounds[1],&bounds[1+NB_QUEUES],res[interval_id+1].x0,res[interval_id+1].y0 ) == 1)
                    //here we modify the state only if bounds are better than res
                    {
                        cpy_state(&bounds[1],res[interval_id+1].x0);
                        cpy_state(&bounds[1+NB_QUEUES],res[interval_id+1].y0);
                        interval_state[interval_id+1] = UPDATED;
                        //printf("Meilleurs bornes pour %d\n",interval_id+1);
                    }
                    break;
                default:
                    cpy_state(&bounds[1],res[interval_id].x0);
                    if(MOD == 0)
                        cpy_state(&bounds[1+NB_QUEUES],res[interval_id].y0);
                    interval_state[interval_id]=UPDATED;

                    break;
            }

        }
        else//Reception of a partial_trajectory
        {

            if( recv(a.id_socket , partial_trajectory , trajectory_bytes_size, MSG_WAITALL) <= 0)
            {
                printf("Connection Closed by server %d",a.id_machine);
                break;
            }
            interval_id = partial_trajectory[0]/(interval_size);


            for(i=0;i<interval_size;i++)
            {
                cpy_state(&partial_trajectory[1+i*NB_QUEUES],final_result[interval_id*(SEQUENCE_SIZE/(nb_inter))+i]);
            }
             i--;

            switch(MOD)
            {
                case 1:
                    if(interval_id != nb_inter-1)
                    {
                        if(better(&partial_trajectory[1+i*NB_QUEUES],&partial_trajectory[1+i*NB_QUEUES],res[interval_id+1].x0,res[interval_id+1].y0 ) == 1)//update only if it is better
                        {
                            cpy_state(&partial_trajectory[1+i*NB_QUEUES],res[interval_id+1].x0);
                            cpy_state(&partial_trajectory[1+i*NB_QUEUES],res[interval_id+1].y0);
                            interval_state[interval_id+1]=UPDATED;
                        }
                    }
                    break;
                default:
                    cpy_state(&partial_trajectory[1+i*NB_QUEUES],res[interval_id].x0);
                    if(MOD == 0)
                        cpy_state(&partial_trajectory[1+i*NB_QUEUES],res[interval_id].y0);
                    break;
            }
            //printf("Reception de la trajectoire %d\n",interval_id);
            interval_state[interval_id]=FINISHED;

        }

        what_do_i_read[a.id_machine]=PAUSE;
    }

    return 0;
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
        master.sin_addr.s_addr = inet_addr("192.168.90.107");

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

//returns 1 if the threads are correctly created and intialized, 0 otherwise
int create_thread(int *servers_id, argument * args)
{

    quit_threads=0;
    pthread_t sniffer_thread ;

    for(int i=0;i<NB_MACHINES;i++)
    {

        //Initialising and creating threads
        what_do_i_read[i]=PAUSE;
        args[i].id_socket = servers_id[i];
        args[i].id_machine = i;

        pthread_detach(sniffer_thread);
    }
	//The thread which is listening to all the servers
	if(pthread_create( &sniffer_thread , NULL ,  server_listener , (void*)&args) < 0)
	{
		perror("could not create thread");
		return 0;
	}
    return 1;
}
