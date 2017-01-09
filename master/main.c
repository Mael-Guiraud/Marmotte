#include <stdio.h> 
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include "struct.h"
#include "thread.h"
#include "operations.h"
#include "simulation.h"
int main(int argc , char *argv[])
{

    struct timeval tv1, tv2;
    double timer;
    double total_time ;
    double total_rounds;
    double total_intervals;
    argument * args ; 
    int * servers_id;
    int socket_desc;
    return_values r;

    srand(time(NULL));
    //creating sockets
    if(!(servers_id= create_sockets(&socket_desc)))
        {
            printf("Erreur while creating sockets\n");
            exit(1);
        }

    assert(args= malloc(sizeof(argument)*(NB_MACHINES) ));
    FILE* f;
    if( (f = fopen("results_moyennes.data","w")) == NULL)
    {
        printf("Error : fopen : results_moyennes.data\n");
        exit(3);
    }

    int last_size = 0;
    for(int expected_size=INTERVAL_SIZE_MIN;expected_size<= INTERVAL_SIZE_MAX;expected_size+=STEP)
    {      
        nb_inter = SEQUENCE_SIZE/expected_size;
        if( (interval_size = SEQUENCE_SIZE/(nb_inter)) != last_size)
        {
            printf("SEQUENCE_SIZE = %d /nb_inter = %d /interval_size = %d\n",SEQUENCE_SIZE,nb_inter,interval_size);

            total_rounds =0.0;
            total_intervals=0.0;
            total_time=0.0;

            //res is greater
            alloc_res();
            //New threads with new interval_size
            if(!create_threads(servers_id,args))
            {
                printf("Error while creating threads\n");
                exit(2);
            }
            //simulation loop
            for(int i=0;i<NB_SIMULS;i++)
            {
                if(NB_SIMULS>=100)
                {
                    if(i%(NB_SIMULS/100) == 0)fprintf(stdout,"\r[%3d%%] ",i/(NB_SIMULS/100)+1);
                    fflush(stdout);
                }
                else
                {
                    fprintf(stdout,"\rStep%3d ",i+1);fflush(stdout);
                }
                gettimeofday (&tv1, NULL);
                r= simul(servers_id);


                gettimeofday (&tv2, NULL);
                timer = ( ((double)tv2.tv_sec*(double)1000 +(double)tv2.tv_usec/(double)1000) - ((double)tv1.tv_sec*(double)1000 + (double)tv1.tv_usec/(double)1000));
                total_rounds+=r.nb_round;
                total_intervals+=r.nb_intervals;
                total_time+=timer;
            }
            printf("\n");

            quit_threads=1;
            wait_all_threads_close();
            write_result_file(f,expected_size,total_rounds/NB_SIMULS,total_intervals/NB_SIMULS,total_time/NB_SIMULS);
            free_res();
            last_size = interval_size;
        }
        
    }
    

    fclose(f);
    free(args);
    free(servers_id);
    close(socket_desc);
    
    return 0;
}
 
