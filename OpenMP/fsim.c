#include "fsim.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_BUFFER 10

#define MIN 0
#define LOAD 1.0
#define P 0.75
#define MU 300


double Distrib[3*NB_QUEUES];


void InitDistribution()
{
	double arrival[NB_QUEUES];
	double service[NB_QUEUES];
	double departure[NB_QUEUES];
    int i;
    double total = 0.0;

    service[0] = P*MU;
    departure[0]= (1-P)*MU;
    arrival[0] = MU*LOAD;
    for(i=1;i<NB_QUEUES;i++)
    {
        service[i] = P*MU;
        departure[i]= (1-P)*MU;
        arrival[i] = arrival[0]*(1-P);
     }

    for(i=0;i<NB_QUEUES;i++)
    {    total+=arrival[i]+service[i]+departure[i];
    }

    for(i=0;i<NB_QUEUES;i++)
    {   Distrib[i] = arrival[i]/total;
        Distrib[i+NB_QUEUES] = departure[i]/total;
        Distrib[i+2*NB_QUEUES] = service[i]/total;
    }
   
}

int Inverse(double U)
{
    double sum;
    sum =0.0;
    for (int i=0; i<3*NB_QUEUES;i++){
        sum+=Distrib[i];
        if (sum>=U) {return i;}
    }
    return 3*NB_QUEUES;
}


STATE transition(STATE t1, double random){ 
	static int nb_calls = 0;
	if(!nb_calls)
	{
		nb_calls++;
   		InitDistribution();

	}
	int indexevt = Inverse(random);
  
       /* effet des evenements dans le reseau, indexevt entre 0 et nb_queues-1 */
    if (indexevt<NB_QUEUES) {
        if (t1.queues[indexevt]<MAX_BUFFER) {t1.queues[indexevt]++;} /*  arrivee */
    }
    else if (indexevt<2*NB_QUEUES) {
        if (t1.queues[indexevt-NB_QUEUES]>MIN) {t1.queues[indexevt-NB_QUEUES]--;} /* depart definitf */
    }
    else if (indexevt<3*NB_QUEUES-1) {
        if (t1.queues[indexevt-2*NB_QUEUES]>MIN) {
            t1.queues[indexevt-2*NB_QUEUES]--;
            if (t1.queues[indexevt-2*NB_QUEUES+1]<MAX_BUFFER) {t1.queues[indexevt-2*NB_QUEUES+1]++;}
        } /* transit entre deux files successives */
    }
    else {if (t1.queues[indexevt-2*NB_QUEUES]>MIN) {t1.queues[indexevt-2*NB_QUEUES]--;}
        /* depart definitf, la derniere file est particuliere */
    }

    return t1;
}

int compare(STATE s1, STATE s2){
	int i;
	for( i = 0; i < NB_QUEUES && s1.queues[i]== s2.queues[i]; i++); // va au premier indice pour lequel il y a une différence entre s1 et s3
	if( i != NB_QUEUES) {
	    return s1.queues[i] < s2.queues[i] ? -1 : 1;
	}
	return 0;
}

STATE random_state(){
	STATE t;
	for(int i=0;i<NB_QUEUES;i++)
	{
		t.queues[i] = rand() % (MAX_BUFFER +1);
	}
	return t;
}


STATE min_state(){
	STATE t;
	memset(t.queues,MIN,sizeof(int)*NB_QUEUES);
	return t;
}

STATE max_state(){
	STATE t;
	for(int i=0;i<NB_QUEUES;i++)
	{
		t.queues[i]=MAX_BUFFER;
	}
	return t;
}