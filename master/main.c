#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>

#include <unistd.h>
#include <time.h>
#include <sys/socket.h>


#include "socket.h"
#include "operations.h"
#include "simulation.h"


#define MAX_QUEUES 10
#define NB_MACHINES 3
#define NB_QUEUES 10



int main(int argc , char *argv[])
{
	int nb_machines = NB_MACHINES;
	int nb_queues = NB_QUEUES;
	//Socket id
    int * servers_id;
    
    //Min & MAX clients in a queue
    int min = 0;
	int max = 100;


	//Init seed of alea
	srand(time(NULL));


    //creating sockets
    if(! (servers_id = create_and_connect_sockets(nb_machines)) )
    {
        printf("Erreur while creating sockets\n");
        exit(1);
    }

    //Allocation of the message, have an unique size. 
	int message_size = sizeof(int) * (4+2*MAX_QUEUES);
	int * message = (int *) malloc(message_size);


	//Send the simulation config to the servers
	send_config( message,message_size, servers_id,  nb_machines,  min,  max, 1.0,0.75,300.0,nb_queues);
	


	int nb_inter = 20;
	int interval_size = 20000;
	
	AlgoTwoBounds(servers_id,message,message_size,nb_inter,interval_size,nb_machines,nb_queues,min,max);

	send_exit( message, message_size,  servers_id,  nb_machines);
	free(message);
	destroy_sockets(servers_id,nb_machines);
    free(servers_id);

    return 0;
}
