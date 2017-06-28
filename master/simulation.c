
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <sys/socket.h>

#include "operations.h"
#include "struct.h"
#include "thread.h"


//send the message of the interval given in parameter. Returns 1 if the message send is composed of 2 coupled state, 0 otherwise
int send_message(int * servers_id,int * message,int interval,int message_bytes_size)
{
    int val = 0;


    int machine_used = sniffer_machine();

    switch(MOD)
    {
        case 2:
            message[0]=2;
            break;
        default:
            message[0]=1;
            break;
    }
    message[0]=1;
    message[1]=(interval_size)*interval;
    message[2]=interval_size;
    cpy_state(res[interval].x0,&message[3]);

    switch(MOD)
    {
        case 2:
            if(interval_state[interval] == VALIDATED)
            {
                cpy_state(res[interval].x0,&message[3+NB_QUEUES]);
                what_do_i_read[machine_used]=TRAJECTORY;
                val = 1;
                interval_state[interval] = SENT;
            }
            else
            {
                if(interval_state[interval] == UPDATED)
                {
                    initState(&message[3+NB_QUEUES]);
                    what_do_i_read[machine_used]=BOUNDS;
                    interval_state[interval] = SENT;
                }
            }
            break;
        default:
            interval_state[interval]=SENT;
            cpy_state(res[interval].y0,&message[3+NB_QUEUES]);
            if(coupling(&message[3],&message[3+NB_QUEUES]))
            {
                what_do_i_read[machine_used]=TRAJECTORY;
                val = 1;
            }
            else
            {
               what_do_i_read[machine_used]=BOUNDS;
            }
            break;
    }
    send(servers_id[machine_used], message , message_bytes_size , 0);

    return val;

}

return_values simul(int * servers_id)
{

    int message_bytes_size = sizeof(int)*(NB_QUEUES*2+3);
    int nb_recv = 0;
    int nb_round = 0;
    int * message;

    //For optimised version
    int nb_calculed_intervals=0;
    int interval_used;

    assert(final_result = malloc(sizeof(int*)*SEQUENCE_SIZE));
    for(int i = 0;i<SEQUENCE_SIZE;i++)
    {
        assert(final_result[i]=malloc(sizeof(int)*NB_QUEUES));
    }
	printf("simul : %d\n", nb_inter);
    assert(interval_state = malloc(sizeof(Interval_state)*(nb_inter)));
    assert(message = malloc(message_bytes_size));


    //initialisation
    for(int i=0;i<nb_inter;i++)
    {
        switch(MOD)
        {
            case 0:
                interval_state[i]=SENT;
                break;
            default:
                interval_state[i]=UPDATED;
                break;
        }
        switch(INIT)
        {
            case 2:
                initStateRand(res[i].x0);
                break;
            case 1:
                initStateMAX(res[i].x0);
                break;
            default:
                initStateMIN(res[i].x0);
                break;
        }
         switch(MOD)
        {
            case 2:
                initState(res[i].y0);
                break;
            default:
                if(i==0)
                {
                    cpy_state(res[i].x0,res[i].y0);
                }
                else
                {
                    initStateMAX(res[i].y0);
                }
                break;

        }
    }

    //Send seed to all servers
    message[0]=0;
    message[1]=time(NULL);
    message[2]=SEQUENCE_SIZE;
    for(int i =0;i<NB_QUEUES;i++){message[3+i]=-1;message[3+i+NB_QUEUES]=-1;};//fill the message with -1 instead of states
    for(int i=0;i<NB_MACHINES;i++) send(servers_id[i] , message , message_bytes_size , 0);
    switch(MOD)
    {

        case 2:
            interval_state[0]=VALIDATED;
            while(nb_recv != nb_inter)
            {

                for(int i=0;i<nb_inter;i++)
                {
                    if(interval_state[i]!=FINISHED)//Treating only not finished intervals
                    {
                        nb_recv += send_message(servers_id,message,i,message_bytes_size);
                        nb_calculed_intervals++;
                    }

                }

                //waiting for all the intervals reply
                //wait_threads(nb_inter);


                for(int i=0;i<nb_inter-1;i++)
                {
                    if(interval_state[i+1]!=FINISHED)
                    {
                        if((interval_state[i] == FINISHED) || ( (interval_state[i]== VALIDATED) && (coupling(res[i].x0,res[i].y0)) ) )
                        {
                             interval_state[i+1]=VALIDATED;
                        }
                    }
                    cpy_state(res[i].x0,res[i].y0);
                }

                //Old final states are begin states of the next interval
                for(int i=(nb_inter)-1;i>0;i--)
                {
                    cpy_state(res[i-1].x0,res[i].x0);
                }

            nb_round++;
            }
            break;
        case 1:
            while( (interval_used = sniffer_interval()) != -1 )
            {
                nb_recv += send_message(servers_id,message,interval_used,message_bytes_size);
                nb_calculed_intervals++;

            }
            for(int i = 0;i<NB_MACHINES;i++)//wait for all threads to finish before closing the simulation (otherwise, one thread could write in free'd memory)
            {
                while(what_do_i_read[i]!=PAUSE);
            }
            break;
        default:
            while(nb_recv != nb_inter)
            {

                for(int i=0;i<nb_inter;i++)
                {
                    if(interval_state[i]!=FINISHED)//Treating only not finished intervals
                    {
                        nb_recv += send_message(servers_id,message,i,message_bytes_size);
                        nb_calculed_intervals++;

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
            break;

    }


    free(message);
    for(int i = 0;i<SEQUENCE_SIZE;i++)
    {
        free(final_result[i]);
    }
    free(final_result);

    free((void*)interval_state);

    return_values r;
    r.nb_round = nb_round;
    r.nb_intervals = nb_calculed_intervals;
    return r;


}
