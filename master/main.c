#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>

#include "struct.h"
#include "socket.h"
#include "operations.h"

int main(int argc , char *argv[])
{
    int * servers_id;
    srand(time(NULL));

    //creating sockets
    if(! (servers_id = create_and_connect_sockets()) )
    {
        printf("Erreur while creating sockets\n");
        exit(1);
    }


	int taille_message = sizeof(int) * (4+2*MAX_QUEUES);
	int * message = (int *) malloc(taille_message);

	//int message[24];
	//int taille_message = sizeof(message);
	int nb_inter = 1;
	if (nb_inter < NB_MACHINES)
	{
		perror("Nb interval < Nb machines\n");
		exit(28);
	}
	int interval_size = 3000000;
	int seeds[nb_inter];
	int min = 0;
	int max = 100;
	assert(interval_state = malloc(sizeof(Interval_state)*(nb_inter)));

	Bounds *bounds = (Bounds *) initBounds(nb_inter, min, max);

	//Generate seeds for all intervals
	for (int i=0; i<nb_inter; i++)
		seeds[i] = rand();

	//Send new configuration
	message[0] = 2;
	message[1] = 2;
	message[2] = min;
	message[3] = max;
	// -1 = comma : ','
	//load is splitted in X.XX
	message[4] = 1;
	message[5] = -1;
	message[6] = 0;
	message[7] = 0;

	//rho is splitted in XXX.XXXX
	message[8] = 0;
	message[9] = 0;
	message[10] = 0;
	message[11] = -1;
	message[12] = 7;
	message[13] = 5;
	message[14] = 0;
	message[15] = 0;

	//mu is splitted in XXX.XXXX
	message[16] = 3;
	message[17] = 0;
	message[18] = 0;
	message[19] = -1;
	message[20] = 0;
	message[21] = 0;
	message[22] = 0;
	message[23] = 0;


	for(int i=0; i<NB_MACHINES; i++)
	{

		if( send(servers_id[i], message, taille_message, 0) < 0 )
		{
		    perror("send()");
		    return(-1);
		}
	}

	//Send new seeds
	message[0] = 0;
	message[1] = nb_inter;


	for(int i=0; i<NB_MACHINES; i++)
	{
		if( send(servers_id[i], message, taille_message, 0) < 0 )
		{
		    perror("send()");
		    return(-1);
		}
	}

	message[0] = 1;		//BOUNDS
	message[2] = interval_size;

	initDpeartureBounds(bounds[0].lb, bounds[0].ub, max);

	for(int i=0; i<nb_inter-1; i++)
		interval_state[i] = UPDATED;
	interval_state[nb_inter-1] = SENT;
	for(int i=0; i<NB_MACHINES; i++)
	{
		if (coupling(bounds[i].lb, bounds[i].ub))
		{
			what_do_i_read[i] = TRAJECTORY;
		}
		else
			what_do_i_read[i] = BOUNDS;

		message[1] = i;
		message[3] = seeds[i];

		cpy_state(bounds[i].lb, &message[4]);
		cpy_state(bounds[i].ub, &message[4+NB_QUEUES]);

		if( send(servers_id[i], message, taille_message, 0) < 0 )
		{
		    perror("send()");
		    return(-1);
		}
		interval_state[i] = SENT;
	}


	int max_sd, new_interval, current_interval;
	int size_bounds_buffer = (NB_QUEUES * 2) + 1;
	int buffer_bounds[size_bounds_buffer];
	int size_trajectory_buffer = (NB_QUEUES * interval_size) + 1;;
	int * buffer_trajectory = (int *)malloc(sizeof(int)*size_trajectory_buffer);
	//int size_trajectory_buffer = (NB_QUEUES * interval_size) + 1;;
 	//int buffer_trajectory[size_trajectory_buffer];
	int cpt = 0;
	fd_set readfds;
	int nb_finished =0;
	while (nb_finished < nb_inter)
	{
		max_sd = initialize_set(&readfds, NB_MACHINES, servers_id);

		if (select( max_sd + 1 , &readfds , NULL , NULL , NULL) < 0)
		{
			printf("select error");
			return (-1);
		}
		else
		{
			//Here we will respond to one server
			while(1)
			{
				//RECEPTION`

				if ( FD_ISSET(servers_id[cpt], &readfds) )
				{
					if (what_do_i_read[cpt] == BOUNDS)
					{
						if (recv(servers_id[cpt], buffer_bounds, sizeof(int)*size_bounds_buffer, MSG_WAITALL) < 0)
						{
							printf("Reception error (bounds)\n");
							return(-1);
						}
						//for(int l=0;l<size_bounds_buffer;l++)printf("%d ",buffer_bounds[l]);printf("\n");
						current_interval = buffer_bounds[0];
					}

					else	//We expect a trajectory
					{

						if (recv(servers_id[cpt], buffer_trajectory, sizeof(int)*size_trajectory_buffer, MSG_WAITALL) < 0)
						{
							printf("Reception error (trajectory)\n");
							return(-1);
						}
						current_interval = buffer_trajectory[0];
						nb_finished++;
					}

					if (current_interval < nb_inter -1)
					{
						if (what_do_i_read[cpt] == BOUNDS)
						{
							cpy_state(&buffer_bounds[1], bounds[current_interval+1].lb);
							cpy_state(&buffer_bounds[1+NB_QUEUES], bounds[current_interval+1].ub);
						}
						else
						{
							//recupere derniere valeur
							cpy_state(&buffer_trajectory[size_trajectory_buffer-NB_QUEUES], bounds[current_interval+1].lb);
							cpy_state(&buffer_trajectory[size_trajectory_buffer-NB_QUEUES], bounds[current_interval+1].ub);
							interval_state[current_interval] = FINISHED;
						}
						if(interval_state[current_interval+1] != FINISHED)
							interval_state[current_interval+1] = UPDATED;
					}
					//GIVE A NEW INTERVAL TO THE SERVER
					new_interval = sniffer_interval(nb_inter);

					if (new_interval == -1)
						{
							cpt=0;
							break;
						}
					message[0] = 1;		//BOUNDS
					message[1] = new_interval;
					message[2] = interval_size;
					message[3] = seeds[new_interval];


					cpy_state(bounds[new_interval].lb,&message[4]);
					cpy_state(bounds[new_interval].ub,&message[4+NB_QUEUES]);
					if ( coupling(bounds[new_interval].lb, bounds[new_interval].ub) )
						what_do_i_read[cpt] = TRAJECTORY;
					else
						what_do_i_read[cpt] = BOUNDS;


					if( send(servers_id[cpt], message, taille_message, 0) < 0 )
					{
					    perror("send()");
					    return(-1);
					}

						interval_state[new_interval] = SENT;

					cpt = 0;
					break;
				}
				cpt++;
			}
		}

	}
	ask_for_time_display(servers_id);
	free_bounds(bounds, nb_inter);
	destroy_sockets(servers_id);
	free( (void *) interval_state);
	free(buffer_trajectory);
	free(message);
    free(servers_id);

    return 0;
}
