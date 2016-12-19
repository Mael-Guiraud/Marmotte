
#include <stdlib.h>
#include <stdio.h> 
#include <assert.h>
#include <time.h>
#include <sys/socket.h>

#include "operations.h"
#include "struct.h"
#include "thread.h"


int simul(int * servers_id)
{

    int message_bytes_size = sizeof(int)*(NB_QUEUES*2+3);
    int nb_recv = 0;
    int machine_used;
    int nb_round = 0;
    int * message;


    assert(final_result = malloc(sizeof(int*)*SEQUENCE_SIZE)); 
    for(int i = 0;i<SEQUENCE_SIZE;i++)
    {
        assert(final_result[i]=malloc(sizeof(int)*NB_QUEUES));
    }
    assert(interval_state = malloc(sizeof(int)*(nb_inter)));
    assert(message = malloc(message_bytes_size));
   

    //initialisation 
    for(int i=0;i<nb_inter;i++)
    {
        interval_state[i]=TODO;
        initStateMIN(res[i].x0);
        if(i==0)
        {
            initStateMIN(res[i].y0);
          
        }
        else
        {
            initStateMAX(res[i].y0);
        }        
    } 
    
    //Send seed to all servers
    message[0]=0;
    message[1]=time(NULL);;
    message[2]=SEQUENCE_SIZE;
    for(int i =0;i<NB_QUEUES;i++){message[3+i]=-1;message[3+i+NB_QUEUES]=-1;};
    for(int i=0;i<NB_MACHINES;i++) send(servers_id[i] , message , message_bytes_size , 0);
    

    while(nb_recv != nb_inter)
    {

        for(int i=0;i<nb_inter;i++)
        {
            if(interval_state[i]!=FINISHED)//Treating only not finished intervals
            {
                interval_state[i]=TODO;

                machine_used = sniffer_machine();
                machine_availability[machine_used]=WORKING;
            

                message[0]=1;
                message[1]=(interval_size-1)*i;
                message[2]=interval_size;
                cpy_state(res[i].x0,&message[3]);
                cpy_state(res[i].y0,&message[3+NB_QUEUES]);
                send(servers_id[machine_used], message , message_bytes_size , 0);


                if(coupling(res[i].x0,res[i].y0))
                {
                    what_do_i_read[machine_used]=TRAJECTORY; 
                    nb_recv++; 
                }
                else
                {
                   what_do_i_read[machine_used]=BOUNDS;
                }
            }

        }
        //waiting for all the intervals reply
        wait_threads(nb_inter);

        //Old final states are begin states of the next interval
        for(int i=(nb_inter)-1;i>0;i--)
        {
            cpy_state(res[i-1].x0,res[i].x0);
            cpy_state(res[i-1].y0,res[i].y0); 
        }
        
    nb_round++; 
    }
  

    free(message);
    for(int i = 0;i<SEQUENCE_SIZE;i++)
    {
        free(final_result[i]);
    }
    free(final_result);

    free((void*)interval_state);  


    return nb_round;

}   


int simul_optim(int * servers_id)
{

    int message_bytes_size = sizeof(int)*(NB_QUEUES*2+3);
    int machine_used;
    int interval_used;
    int * message;
    int nb_calculed_intervals=0;


    assert(final_result = malloc(sizeof(int*)*SEQUENCE_SIZE)); 
    for(int i = 0;i<SEQUENCE_SIZE;i++)
    {
        assert(final_result[i]=malloc(sizeof(int)*NB_QUEUES));
    }
    assert(interval_state = malloc(sizeof(int)*(nb_inter)));
    assert(message = malloc(message_bytes_size));
   

    //initialisation 
    for(int i=0;i<nb_inter;i++)
    {
        interval_state[i]=UPDATED;
        initStateMIN(res[i].x0);
        if(i==0)
        {
            initStateMIN(res[i].y0);
          
        }
        else
        {
            initStateMAX(res[i].y0);
        }        
    } 
    interval_state[nb_inter-1]=TODO;
    
    //Send seed to all servers
    message[0]=0;
    message[1]=time(NULL);;
    message[2]=SEQUENCE_SIZE;
    for(int i =0;i<NB_QUEUES;i++){message[3+i]=-1;message[3+i+NB_QUEUES]=-1;};
    for(int i=0;i<NB_MACHINES;i++) send(servers_id[i] , message , message_bytes_size , 0);
    
    nb_recv = 0;


    while(nb_recv != nb_inter)
    {
        if( (interval_used = sniffer_interval()) == -1)break;
        if( !( (interval_used == nb_inter-1) && (!coupling(res[interval_used].x0,res[interval_used].y0)) )   )//we want to make calculation on the last inteval only if this is the good starting point
        {
            machine_used = sniffer_machine();
            machine_availability[machine_used]=WORKING;  
            message[0]=1;
            message[1]=(interval_size-1)*interval_used;
            message[2]=interval_size;

            cpy_state(res[interval_used].x0,&message[3]);
            cpy_state(res[interval_used].y0,&message[3+NB_QUEUES]);
            send(servers_id[machine_used], message , message_bytes_size , 0);
            if(coupling(&message[3],&message[3+NB_QUEUES]))
            {
                what_do_i_read[machine_used]=TRAJECTORY; 
            }
            else
            {
                what_do_i_read[machine_used]=BOUNDS;
            }
            interval_state[interval_used]=TODO;
        }

    }

    free(message);
    for(int i = 0;i<SEQUENCE_SIZE;i++)
    {
        free(final_result[i]);
    }
    free(final_result);

    free((void*)interval_state);  


    return nb_calculed_intervals;

}   