#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
/*int Min[NB_QUEUES], Max[NB_QUEUES];
double Distrib[SizeDistrib];
double arrival[NB_QUEUES], service[NB_QUEUES], departure[NB_QUEUES];

#define SizeDistrib 3*NB_QUEUES
*/

int nb_queues;
int SizeDistrib;

int Min ;
int Max ;
double* Distrib = NULL;
double* arrival = NULL;
double* service=NULL;
double* departure = NULL;

void simulation_mem_init(int nb_q, int min, int max)
{
    nb_queues = nb_q;
    SizeDistrib = 3*nb_queues;
    Min = min;
    Max = max;
    Distrib = (double*)malloc(sizeof(double)*SizeDistrib);
    arrival = (double*)malloc(sizeof(double)*nb_queues);
    service = (double*)malloc(sizeof(double)*nb_queues);
    departure = (double*)malloc(sizeof(double)*nb_queues);
}
void simulation_mem_free()
{
    if(Distrib)
    {
        free(Distrib);
        Distrib = NULL;
    }
    if(arrival)
    {
        free(arrival);
        arrival = NULL;
    }
    if(service)
    {
        free(service);
        service = NULL;
    }
    if(departure)
    {
        free(departure);
        departure = NULL;
    }
}
void cpy_state(int* s1, int* s2)
{
	int i;
	for(i=0;i<nb_queues;i++)
	{
		s2[i] = s1[i];
	}
}


void InitDistribution(double load, double p, double mu)
{
    int i,j;
    double total;
    /*double p= 0.75;
    double mu = 300.0;*/
    printf("load %f p %f mu %f\n",load,p,mu);

    service[0] = p*( mu);
    departure[0]= (1-p)*(  mu);
    arrival[0] = mu*load;
    for(i=1;i<nb_queues;i++)
    {
        service[i] = p*( mu);
        departure[i]= (1-p)*(  mu);
        arrival[i] = mu*load;
        printf("i=%d \n",i);
    
        //arrival[i]-= (arrival[j]*pow(p,(double)(i-j)));
        arrival[i]-= arrival[0]*p;

        
        /*     arrival[i ] = 10.0;
        service[i ] = 20.0*(i+1);
        departure[i]= 5.0;*/
     }

    total = 0.0;

    for(i=0;i<nb_queues;i++)
    {    total+=arrival[i]+service[i]+departure[i];
    }

    for(i=0;i<nb_queues;i++)
    {   Distrib[i] = arrival[i]/total;
        Distrib[i+nb_queues] = departure[i]/total;
        Distrib[i+2*nb_queues] = service[i]/total;
    }
    total = 0.0;
    for(i=0;i<nb_queues;i++)
    {
        total+=Distrib[i]+ Distrib[i+nb_queues]+Distrib[i+2*nb_queues];
    }
    for(int i=0;i<3*nb_queues;i++)printf("%f ",Distrib[i]);printf("\n");
   
}
void init_simul(int nb_q, int min, int max,double load, double p, double mu)
{
    simulation_mem_free();
    simulation_mem_init( nb_q,  min,  max);
    InitDistribution( load,  p,  mu);
  //  printf("Valeurs de la simuls changés pour : %d %d %d %f %f %f\n",nb_q,min,max,load,p,mu);
}


int Inverse(double U)
{
    /* Methode de transformée inverse, pas optimisee  */
    if(!Distrib)
    {
        printf("Error : Simulation uninitialized.\n");
        exit(12);
    }
    int i,j;
    double sum;
    j=SizeDistrib;
    sum =0.0;
    for (i=0; i<SizeDistrib;i++){
        sum+=Distrib[i];
        if (sum>=U) {return i;}
    }
    return(j);
}

void Equation(int* NewS,int indexevt)
{
    /* effet des evenements dans le reseau, indexevt entre 0 et nb_queues-1 */
    if (indexevt<nb_queues) {
        if (NewS[indexevt]<Max) {NewS[indexevt]++;} /*  arrivee */
    }
    else if (indexevt<2*nb_queues) {
        if (NewS[indexevt-nb_queues]>Min) {NewS[indexevt-nb_queues]--;} /* depart definitf */
    }
    else if (indexevt<3*nb_queues-1) {
        if (NewS[indexevt-2*nb_queues]>Min) {
            NewS[indexevt-2*nb_queues]--;
            if (NewS[indexevt-2*nb_queues+1]<Max) {NewS[indexevt-2*nb_queues+1]++;}
        } /* transit entre deux files successives */
    }
    else {if (NewS[indexevt-2*nb_queues]>Min) {NewS[indexevt-2*nb_queues]--;}
        /* depart definitf, la derniere file est particuliere */
    }
}

void F (int * OldS,double U )
{ int indexevt;
    indexevt = Inverse(U);

    Equation(OldS,indexevt);

}


void initEtat(int * e)
{
    memset(e,0,sizeof(int)*nb_queues);
}

//Return 1 if s1 and s2 are the same, 0 otherwise
int coupling(int* s1, int* s2)
{
    int i=0;
    while(i<nb_queues && s1[i]==s2[i])i++;
    return (i==nb_queues);
}
