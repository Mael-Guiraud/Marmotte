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
                thread_activity[a.id_machine]=CLOSED;
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
           
            interval_state[interval_id]=FINISHED;
            
        }
       
        what_do_i_read[a.id_machine]=PAUSE;
    }   

    return 0;
}

int* create_sockets(int * socket_desc)
{
    int c;
    struct sockaddr_in master;
    struct sockaddr_in* client_addr;
    int * servers_id ;//Socket id of the servers

    master.sin_family = AF_INET; //IPV4 add
    master.sin_addr.s_addr = INADDR_ANY; //use any add of the machine
    master.sin_port = htons( 8888 ); //port number

    assert(client_addr = malloc(sizeof(struct sockaddr_in)*NB_MACHINES)); 
    assert(servers_id= malloc(sizeof(int)*NB_MACHINES));
    c = sizeof(struct sockaddr_in);

    //Creation of the socket on MASTER
    if ( (*socket_desc = socket(AF_INET , SOCK_STREAM , 0))== -1)
        printf("Could not create socket");
    
    //Allow the socket to be re-used after being closed
    if (setsockopt(*socket_desc, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &(int){ 1 }, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    if (setsockopt(*socket_desc, IPPROTO_TCP, TCP_NODELAY, &(int){ 1 }, sizeof(int)) < 0)
        perror("setsockopt(TCP_NODELAY) failed"); 
     
    if (setsockopt(*socket_desc, IPPROTO_TCP, TCP_QUICKACK, &(int){ 1 }, sizeof(int)) < 0)
        perror("setsockopt(TCP_QUICKACK) failed"); 
    
    //Bind, Connect the Socket to the port 
    if( bind(*socket_desc,(struct sockaddr *)&master , sizeof(master)) < 0)
    {
        perror("bind failed. Error");
        return NULL;
    }
    //Passive socket, will wait connections with accept.
    listen(*socket_desc , 5);

    for(int i=0;i<NB_MACHINES;i++)  
    {
        if ( (servers_id[i] = accept(*socket_desc, (struct sockaddr *)&client_addr[i], (socklen_t*)&c)) < 0)
        {
            perror("accept failed");
            return NULL;
        }
    }
    free(client_addr);

    return servers_id;
}

//returns 1 if the threads are correctly created and intialized, 0 otherwise
int create_threads(int * servers_id,argument * args)
{

    quit_threads=0;
    pthread_t sniffer_thread ;

    for(int i=0;i<NB_MACHINES;i++)
    {

        //Initialising and creating threads
        what_do_i_read[i]=PAUSE;
        thread_activity[i]=OPEN;
        args[i].id_socket = servers_id[i];
        args[i].id_machine = i;

        if(pthread_create( &sniffer_thread , NULL ,  server_listener , (void*)&args[i]) < 0)
                    {
                        perror("could not create thread");
                        return 0;
                    }  
            
                            
        
        pthread_detach(sniffer_thread); 

    }
    return 1;
}
