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
#define NB_QUEUES 3


double time_diff(struct timeval tv1, struct timeval tv2)
{
    return (((double)tv2.tv_sec*(double)1000 +(double)tv2.tv_usec/(double)1000) - ((double)tv1.tv_sec*(double)1000 + (double)tv1.tv_usec/(double)1000));
}

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
	int nb_inter;
	int simulation_length = 15000;
	int interval_size;


	//Var for the simulation
	int nb_simuls = 10;
	int max_calculated = 0;
	long long average = 0;
	struct timeval tv1, tv2;
	double time_computing = 0.0;
	double max_time = 0.0;

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
	int begin =4;
	int end = 20;
	int step=1;

	for(nb_inter = begin;nb_inter <= end;nb_inter+= step)
	{
		average =0;
		time_computing = 0.0;
		for(int i=0;i<nb_simuls;i++)
		{	
			interval_size = simulation_length/nb_inter;
			gettimeofday (&tv1, NULL);
			average += AlgoTwoBounds(servers_id,message,message_size,nb_inter,interval_size,nb_machines,nb_queues,min,max);
			gettimeofday (&tv2, NULL);
		    time_computing += time_diff(tv1,tv2);	
			ask_for_time_display(times,message,message_size,servers_id, nb_machines,nb_measures);

		}
		fprintf(f_result,"%d %d %f\n",nb_inter,(int)(average/nb_simuls),time_computing/nb_simuls);
		if(max_calculated < average / nb_simuls)
		{
			max_calculated = average / nb_simuls;
		}
		if(max_time < time_computing / nb_simuls)
		{
			max_time = time_computing / nb_simuls;
		}
		fprintf(stdout,"\r [%5d/%5d]",nb_inter,end);fflush(stdout);
		
	}
	printf("\n");
	print_gnuplot("../results/inters.pdf",begin,end,0,max_calculated,0,(int)(max_time+1));


	//send_exit( message, message_size,  servers_id,  nb_machines);

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
