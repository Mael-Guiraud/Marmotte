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
	printf("Sockets creation...\n");
    if(! (servers_id = create_and_connect_sockets()) )
    {
        printf("Erreur while creating sockets\n");
        exit(1);
    }
	printf("Sockets created and connected...\n\n");

	int message[24];
	//int message[ (NB_QUEUES * 2) + 4 ];
	int taille_message = sizeof(message);
	nb_inter = 5;
	int interval_size = 20;
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

	printf("Send new configuration...\n");
	for(int i=0; i<NB_MACHINES; i++)
	{
		if( send(servers_id[i], message, sizeof(message), 0) < 0 )
		{
		    perror("send()");
		    return(-1);
		}
	}

	//Send new seeds
	message[0] = 0;
	message[1] = nb_inter;

	printf("Send new seed...\n");
	for(int i=0; i<NB_MACHINES; i++)
	{
		if( send(servers_id[i], message, taille_message, 0) < 0 )
		{
		    perror("send()");
		    return(-1);
		}
	}

	printf("Send new bounds...\n");
	message[0] = 1;		//BOUNDS
	message[2] = interval_size;
		for (int j=0; j<NB_QUEUES; j++)
			{

				bounds[0].lb[j] = 0;
				bounds[0].ub[j] = 0;
			}
	/*for(int i=1; i<NB_MACHINES; i++)
	{
		if (i == 0)
		{
			what_do_i_read[i] = TRAJECTORY;
			initDpeartureBounds(bounds[i].lb, bounds[i].ub, max);
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
	}*/


		what_do_i_read[0] = BOUNDS;
		message[1] = 1;
		message[3] = seeds[1];

		cpy_state(bounds[1].lb, &message[4]);
		cpy_state(bounds[1].ub, &message[4+NB_QUEUES]);

		if( send(servers_id[0], message, taille_message, 0) < 0 )
		{
		    perror("sssend()");
		    return(-1);
		}


	for(int i=0; i<nb_inter; i++)
		interval_state[i] = UPDATED;

	printf("End of initialization\n");

	int max_sd, new_interval, current_interval;
	int size_bounds_buffer = (NB_QUEUES * 2) + 1;
	int buffer_bounds[size_bounds_buffer];
	int size_trajectory_buffer = (NB_QUEUES * interval_size) + 1;;
	int buffer_trajectory[size_trajectory_buffer];

	int cpt = 0;
	fd_set readfds;
	int nb_finished =0;
	while (nb_finished < nb_inter)
	{
		printf("debut while\n");
		max_sd = initialize_set(&readfds, NB_MACHINES, servers_id);
		printf("select\n");
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

					printf("Avant reception : \n");
					affiche_bounds(bounds, nb_inter);
				if ( FD_ISSET(servers_id[cpt], &readfds) )
				{

					if (what_do_i_read[cpt] == BOUNDS)
					{
						if (recv(servers_id[cpt], buffer_bounds,sizeof(int)* size_bounds_buffer, 0) < 0)
						{
							printf("Reception error\n");
							return(-1);
						}
						printf("reception bounds\n");
						for(int l=0;l<size_bounds_buffer;l++)printf("%d ",buffer_bounds[l]);printf("\n");
						current_interval = buffer_bounds[0];
					}

					else	//We expect a trajectory
					{
						if (recv(servers_id[cpt], buffer_trajectory, sizeof(int)*size_trajectory_buffer, 0) < 0)
						{
							printf("Reception error\n");
							return(-1);
						}
						printf("reception traj\n");
						for(int l=0;l<size_trajectory_buffer;l++)printf("%d ",buffer_trajectory[l]);printf("\n");
						current_interval = buffer_trajectory[0];
						nb_finished++;
					}
					printf("Interval recu = %d\n",current_interval);

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
						interval_state[current_interval+1] = UPDATED;
					}
					printf("Après : \n");
					affiche_bounds(bounds, nb_inter);
					//GIVE A NEW INTERVAL TO THE SERVER
					new_interval = sniffer_interval();
					if (new_interval == -1)break;
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

					printf("Send new interval...\n");

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

    free(servers_id);

    return 0;
}
