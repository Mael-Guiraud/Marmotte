#include "struct.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>

//Return 1 if s1 and s2 are the same, 0 otherwise
int coupling(int* s1, int* s2, int nb_queues)
{
    return(!memcmp(s1,s2,sizeof(int)*nb_queues));
}

//Copy s1 in s2
void cpy_state(int * s1, int* s2, int nb_queues)
{
    memcpy(s2,s1,sizeof(int)*nb_queues);
}


void free_bounds(Bounds *bounds, int nb_inter)
{

    for(int i = 0;i<nb_inter;i++)
    {
        free(bounds[i].lb);
        free(bounds[i].ub);
    }
    free(bounds);
}

Bounds *initBounds(int nb_interval, int min, int max, int nb_queues)
{
	Bounds *bounds;
	assert(bounds = (Bounds *) malloc(sizeof(Bounds)*(nb_interval) ));
    for(int i = 0;i<nb_interval;i++)
    {
        assert(bounds[i].lb = (int *) malloc(sizeof(int)*nb_queues));
        assert(bounds[i].ub = (int *) malloc(sizeof(int)*nb_queues));
		for (int j=0; j<nb_queues; j++)
		{
			bounds[i].lb[j] = min;
			bounds[i].ub[j] = max;
		}
    }
	return bounds;
}

//Wait for an interval to be updated, and returns its number
int sniffer_interval(Interval_state * interval_state,int nb_inter,int begin)
{
    for(int  i = begin; i < begin+nb_inter; i++)
	{

        if (interval_state[i%nb_inter] == UPDATED)  return (i%nb_inter);
    }
    return -1;
}

int better(int *s1,int*s2,int*s3,int*s4, int nb_queues) //(s1,s2) couple borne inf borne sup comparé à (s3,s4) couple born inf borne sup
{
    int i;
    for( i = 0; i < nb_queues && s1[i]== s3[i]; i++); // va au premier indice pour lequel il y a une différence entre s1 et s3
    if( i != nb_queues) {
        return s1[i] < s3[i] ? -1 : 1;
    } // si cet indice est dans le tableau on peut décider: si s1 < s3 alors
                                                        //s3,s4 est plus proche de coupler sinon c'est s1,s2
    for( i = 0; i < nb_queues && s2[i]==s4[i]; i++);//même test pour s2, s4 si s1 et s3 sont égaux.
    if( i != nb_queues) {
        return s2[i] > s4[i] ? -1 : 1;
    }//l'ordre est inverse car on compare les bornes sup cette fois
    return 0;
}

void initDpeartureBounds(int *borne_min, int *borne_max, int max, int nb_queues)
{

	int random_value = rand() % max;
	for (int i=0; i<nb_queues; i++)
	{
		borne_min[i] = random_value;
		borne_max[i] = random_value;
	}
}

void affiche_bounds(Bounds* bounds, int nb_interval, int nb_queues)
{
    for(int i=0;i<nb_interval;i++)
    {
        printf("[(");
        for(int j=0;j<nb_queues;j++)
        {
            printf("%d ",bounds[i].lb[j]);
        }
        printf(")-(");
        for(int j=0;j<nb_queues;j++)
        {
            printf("%d ",bounds[i].ub[j]);
        }
        printf(")]");
    }
    printf("\n");


}
int snifer_machine(Message_kind * what_do_i_read, int nb_machines)
{
    for(int  i = 0; i < nb_machines; i++)
    {
        if (what_do_i_read[i] == PAUSE)  return (i);
    }
    return -1;
}

int all_finished(Message_kind * what_do_i_read, int nb_machines)
{
    for(int  i = 0; i < nb_machines; i++)
    {
        if (what_do_i_read[i] != PAUSE)  return 0;
    }
    return 1;
}



int updated(int * bound, int nb_queues)
{
    for(int i=0;i<nb_queues;i++)
    {
        if(bound[i] != -1)
            return 1;
    }
    return 0;
}




