#include "struct.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>

//Return 1 if s1 and s2 are the same, 0 otherwise
int coupling(int* s1, int* s2)
{
    return(!memcmp(s1,s2,sizeof(int)*NB_QUEUES));
}

//Copy s1 in s2
void cpy_state(int * s1, int* s2)
{
    memcpy(s2,s1,sizeof(int)*NB_QUEUES);
}

//Set all the queues to 0
void initStateMIN(int* s)
{
    memset(s,0,sizeof(int)*NB_QUEUES);
}
//Set all the queues to BUFF_MAX
void initStateMAX(int* s)
{
    for(int i=0;i<NB_QUEUES;i++)s[i]=BUFF_MAX;
}
//Set all the queues to -1
void initState(int* s)
{
    for(int i=0;i<NB_QUEUES;i++)s[i]=-1;
}
//Set all the queues to a random number
void initStateRand(int* s)
{
    for(int i=0;i<NB_QUEUES;i++)s[i]=rand()%101;
}

//Wait for one machine to be available, and returns its number
int sniffer_machine()
{
    int i;
    while(1){
        for( i = 0; i< NB_MACHINES; i++){
            if(what_do_i_read[i]==PAUSE) return i;
        }
    }
}

void write_result_file(FILE * f,int inter_size,double rounds,double intervals,double time)
{
    fprintf(f,"%d %f %f %f \n",inter_size,rounds,intervals,time);
    printf("Average : Rounds = %f; Intervals calculated = %f; Time =  %f \n",rounds,intervals,time);
}

void free_res()
{

    for(int i = 0;i<nb_inter;i++)
    {
        free(res[i].x0);
        free(res[i].y0);
    }
    free(res);
}

void alloc_res()
{
    assert(res = (Bounds*) malloc(sizeof(Bounds)*(nb_inter) ));
    for(int i = 0;i<nb_inter;i++)
    {
        assert(res[i].x0 = (int *) malloc(sizeof(int)*NB_QUEUES));
        assert(res[i].y0 = (int *) malloc(sizeof(int)*NB_QUEUES));
    }
}

//Wait for an interval to be updated, and returns its number
int sniffer_interval()
{
    int i;
    int end = 0;
    while(!end)
	{
        end = 1;
        for( i = 0; i < nb_inter-1; i++)
		{
            if (interval_state[i] == UPDATED) return i;
            end = (end && interval_state[i] == FINISHED); // end is set to 0 if some element is not finished
			printf("i : %d\n", i);
        }
        if(end && interval_state[i] == UPDATED){ return i;} //deal with the last interval only at the end
    }
	//printf("fin sniffer_interval\n");
    return -1;
}

int better(int *s1,int*s2,int*s3,int*s4) //(s1,s2) couple borne inf borne sup comparé à (s3,s4) couple born inf borne sup
{
    int i;
    for( i = 0; i < NB_QUEUES && s1[i]== s3[i]; i++); // va au premier indice pour lequel il y a une différence entre s1 et s3
    if( i != NB_QUEUES) {
        return s1[i] < s3[i] ? -1 : 1;
    } // si cet indice est dans le tableau on peut décider: si s1 < s3 alors
                                                        //s3,s4 est plus proche de coupler sinon c'est s1,s2
    for( i = 0; i < NB_QUEUES && s2[i]==s4[i]; i++);//même test pour s2, s4 si s1 et s3 sont égaux.
    if( i != NB_QUEUES) {
        return s2[i] > s4[i] ? -1 : 1;
    }//l'ordre est inverse car on compare les bornes sup cette fois
    return 0;
}

Bounds *initBounds(int nb_interval, int min, int max)
{
	Bounds *bounds;
	assert(bounds = (Bounds *) malloc(sizeof(Bounds)*(nb_interval) ));
    for(int i = 0;i<nb_interval;i++)
    {
        assert(bounds[i].x0 = (int *) malloc(sizeof(int)*NB_QUEUES));
        assert(bounds[i].y0 = (int *) malloc(sizeof(int)*NB_QUEUES));
		for (int j=0; j<NB_QUEUES; j++)
		{
			bounds[i].x0[j] = min;
			bounds[i].y0[j] = max;
		}
    }
	return bounds;
}

void initDpeartureBounds(Bounds *bounds, int max)
{
	srand(time(NULL));
	int random_value = rand() % max;
	*(bounds[0].x0) = random_value;
	*(bounds[0].y0) = random_value;
}
