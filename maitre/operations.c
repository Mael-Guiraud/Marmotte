#include "struct.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

//Return 1 if s1 and s2 are the same, 0 otherwise
int coupling(int* s1, int* s2)
{
    int i=0;
    while(i<NB_QUEUES && s1[i]==s2[i])i++;
    return (i==NB_QUEUES);
}

//Copy s1 in s2
void cpy_state(int * s1, int* s2)
{
    for(int i=0;i<NB_QUEUES;i++)
    {
        s2[i] = s1[i];
    }
}

//Set all the queues to 0
void initStateMIN(int* s)
{
    memset(s,0,sizeof(int)*NB_QUEUES);
}
//Set all the queues to BUFF_MAX
void initStateMAX(int* s)
{
    for(int i=0;i<NB_QUEUES;i++)s[i]=100;
}

//Return the number of the first free machine, -1 otherwise
int number_free()
{
    int i;
    for( i = 0; i< NB_MACHINES && !(machine_availability[i]==FREE); i++);
    return (i<NB_MACHINES) ? i : -1;
}

//Wait for one machine to be available, and returns its number
int sniffer_machine()
{
    int i;
    while( (i = number_free()) == -1);
    return i;

}

void write_result_file(FILE * f,int inter_size,double rounds,double time)
{
    fprintf(f,"%d %f %f\n",inter_size,rounds,time);
    printf("%d %f %f\n",inter_size,rounds,time);
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
    assert(res = malloc(sizeof(Bounds)*(nb_inter) )); 
    for(int i = 0;i<nb_inter;i++)
    {
        assert(res[i].x0 = malloc(sizeof(int)*NB_QUEUES));
        assert(res[i].y0 = malloc(sizeof(int)*NB_QUEUES));
    }
}
