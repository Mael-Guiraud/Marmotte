

#include <stdlib.h>
#include <stdio.h>


#include <assert.h>

//socket libs
#include <sys/socket.h>

#include "struct.h"
#include "socket.h"
#include "operations.h"


int AlgoTwoBounds(int * servers_id,int * message,int message_size,int nb_inter,int interval_size,int nb_machines,int nb_queues,int min,int max)
{


	//Seeds of the intervals
	int seeds[nb_inter];

	// For Select Function
	int max_sd;
	fd_set readfds;

	//For the treatment
	int new_interval, current_interval;
	int current_machine = 0;
	int nb_finished =0;

	//Buffer for reception of bounds
	int size_bounds_buffer = (nb_queues * 2) + 1;
	int buffer_bounds[size_bounds_buffer];

	//Buffer for the trajectory
	int size_trajectory_buffer = (nb_queues * interval_size) + 1;;
	int * buffer_trajectory = (int *)malloc(sizeof(int)*size_trajectory_buffer);

	//Remember what kind of message we expect from a server
	Message_kind what_do_i_read[nb_machines];

	//allocate the intervals state table
	Interval_state * interval_state;
	assert(interval_state = malloc(sizeof(Interval_state)*(nb_inter)));
		
	if (nb_inter < nb_machines)
	{
		perror("Nb interval < Nb machines\n");
		exit(28);
	}



	//Send the singal to reinit seeds to all the servers
	build_seed_message(message, nb_inter);
	for(int i=0; i<nb_machines; i++)
	{
		if( send(servers_id[i], message, message_size, 0) < 0 )
		{
		    perror("send()");
		    return(-1);
		}
	}

	//Generate seeds for all intervals
	for (int i=0; i<nb_inter; i++)
		seeds[i] = rand();

	//Init the bounds to min/max and a some random coupled bounds for the first interval
	Bounds *bounds = (Bounds *) initBounds(nb_inter, min, max,nb_queues);
	initDpeartureBounds(bounds[0].lb, bounds[0].ub, max,nb_queues);



	//We consider all intervals to UPDATED, (unless the last one)
	for(int i=0; i<nb_inter-1; i++)
		interval_state[i] = UPDATED;
	interval_state[nb_inter-1] = SENT;


	// We send an interval to each machine at the beginning
	for(int i=0; i<nb_machines; i++)
	{
		if (coupling(bounds[i].lb, bounds[i].ub,nb_queues))
		{
			what_do_i_read[i] = TRAJECTORY;
		}
		else
			what_do_i_read[i] = BOUNDS;


		build_bounds_message(message, bounds, i, interval_size, seeds[i],nb_queues);

		if( send(servers_id[i], message, message_size, 0) < 0 )
		{
		    perror("send()");
		    return(-1);
		}
		interval_state[i] = SENT;
	}


	
	while (nb_finished < nb_inter)
	{
		//put all the socket descriptors in the set
		max_sd = initialize_set(&readfds, nb_machines, servers_id);

		//Wait from a reply of a server, and put the server id in the set (may have several servs)
		if (select( max_sd + 1 , &readfds , NULL , NULL , NULL) < 0)
		{
			printf("select error");
			return (-1);
		}
		else
		{
			//Loop on the servs
			while(1)
			{
				//look if the serv "current_machine" is in the set returned by select
				if ( FD_ISSET(servers_id[current_machine], &readfds) )
				{
					if (what_do_i_read[current_machine] == BOUNDS)
					{
						if (recv(servers_id[current_machine], buffer_bounds, sizeof(int)*size_bounds_buffer, MSG_WAITALL) < 0)
						{
							printf("Reception error (bounds)\n");
							return(-1);
						}
						current_interval = buffer_bounds[0];
					}

					else	//We expect a trajectory
					{
						if (recv(servers_id[current_machine], buffer_trajectory, sizeof(int)*size_trajectory_buffer, MSG_WAITALL) < 0)
						{
							printf("Reception error (trajectory)\n");
							return(-1);
						}
						current_interval = buffer_trajectory[0];
						nb_finished++;
					}
					//We want tu update the bounds of the next interval if we are not in the last interval
					if (current_interval < nb_inter -1)
					{
						if (what_do_i_read[current_machine] == BOUNDS)
						{
							cpy_state(&buffer_bounds[1], bounds[current_interval+1].lb,nb_queues);
							cpy_state(&buffer_bounds[1+nb_queues], bounds[current_interval+1].ub,nb_queues);
						}
						else
						{

							//WE CAN DO A TREATMENT (Writting in a file ) OF THE TRAJECTORY HERE
							
							//Copy last value of the trajectory in the bounds of the next interval
							cpy_state(&buffer_trajectory[size_trajectory_buffer-nb_queues], bounds[current_interval+1].lb,nb_queues);
							cpy_state(&buffer_trajectory[size_trajectory_buffer-nb_queues], bounds[current_interval+1].ub,nb_queues);
							for(int a=0;a<10;a++){printf("%d ",buffer_trajectory[size_trajectory_buffer-nb_queues+a]);}printf("\n");
							interval_state[current_interval] = FINISHED;
						}
						//We update the next interval only if it's not finished yet
						if(interval_state[current_interval+1] != FINISHED)
							interval_state[current_interval+1] = UPDATED;
					}


					//Search for an updated interval
					new_interval = sniffer_interval(interval_state,nb_inter);

					//If there is no updated intervals (end of the simulation), the server doesnt work anymore
					if (new_interval == -1)
					{
						current_machine=0;
						break;
					}

					build_bounds_message(message, bounds, new_interval, interval_size, seeds[new_interval],nb_queues);

					if ( coupling(bounds[new_interval].lb, bounds[new_interval].ub,nb_queues) )
						what_do_i_read[current_machine] = TRAJECTORY;
					else
						what_do_i_read[current_machine] = BOUNDS;


					if( send(servers_id[current_machine], message, message_size, 0) < 0 )
					{
					    perror("send()");
					    return(-1);
					}

						interval_state[new_interval] = SENT;

					current_machine = 0;
					break;
				}
				current_machine++;
			}
		}

	}
	ask_for_time_display(message,message_size,servers_id, nb_machines);
	free_bounds(bounds, nb_inter);
	free( (void *) interval_state);
	free(buffer_trajectory);
	return 1;
}