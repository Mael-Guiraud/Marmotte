#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>

#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <assert.h>


#include "socket.h"
#include "operations.h"
#include "simulation.h"
#include "data_treatment.h"


#define MAX_QUEUES 10
#define NB_MACHINES 3
#define NB_QUEUES 2



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
	int * message;
	(message = (int *) malloc(message_size));


	//Send the simulation config to the servers
	send_config( message,message_size, servers_id,  nb_machines,  min,  max, 1.1,0.75,300.0,nb_queues);
	

	//Parameters of the simulation
	int nb_inter = 20;
	int interval_size;

	//Var for the simulation
	int nb_simuls = 10;
	int max_calculated = 0;
	long long average = 0;
	//To store the measures of the servers
	int nb_measures = 5;
	double **times;
	assert(times = (double **)malloc(sizeof(double*)*nb_machines));
	for(int i=0;i<nb_machines;i++)
	{
		assert(times[i] = (double*)malloc(sizeof(double)*nb_measures));
	}

	//File for the results
	FILE* f_result = fopen("../results/inters.data","w");
	if(!f_result)
	{
		printf("Failed to open \"../results/inters.data\".\n");
		return 1;
	}
	//We reset the timers before starting to measure.
	ask_for_time_display(times,message,message_size,servers_id, nb_machines,nb_measures);

	//interval measures parameters
	int begin = 1000;
	int end = 15000;
	int step=1000;

	for(interval_size = begin;interval_size <= end;interval_size= interval_size + step)
	{
		for(int i=0;i<nb_simuls;i++)
		{
			printf("NEW_SIMUL interval size = %d\n",interval_size);
			average += AlgoTwoBounds(servers_id,message,message_size,nb_inter,interval_size,nb_machines,nb_queues,min,max);
			ask_for_time_display(times,message,message_size,servers_id, nb_machines,nb_measures);

		}
		fprintf(f_result,"%d %d\n",interval_size,(int)(average/nb_simuls));
		if(max_calculated < average / nb_simuls)
		{
			max_calculated = average / nb_simuls;
		}

	}
	print_gnuplot("inters.pdf",begin,end,0,max_calculated);


	send_exit( message, message_size,  servers_id,  nb_machines);

	for(int i=0;i<nb_machines;i++)
	{
		free(times[i]);
	}
	free(times);
	free(message);
	destroy_sockets(servers_id,nb_machines);
    free(servers_id);
    fclose(f_result);

    return 0;
}
