#include "const.h"
#include <math.h>
#include <string.h>
void InitRectangle();
int couplage(int * e1, int * e2);
void InitDistribution();
int Inverse(double U);
void Equation(int* OldS,int indexevt);
void initEtat(int * e);


int Min[NB_QUEUES], Max[NB_QUEUES];
double Distrib[SizeDistrib];
double arrival[NB_QUEUES], service[NB_QUEUES], departure[NB_QUEUES];

void cpy_state(int* s1, int* s2)
{
	int i;
	for(i=0;i<NB_QUEUES;i++)
	{
		s2[i] = s1[i];
	}
}


void InitRectangle()
{
    int i;
    for(i=0;i<NB_QUEUES;i++)
    {
        Min[i ] = 0;
        Max[i ] = 100;
    }
}


void InitDistribution(double charge)
{
    int i,j;
    double total;
    double p= 0.75;
    double mu = 300.0;
    /* construit la distribution pour un reseau en tandem */
    /* la premeire partie permet de coder les taux d'arrivee, puis de sortie definitive
     et enfin de service
     pour aller a la file suivante . Ca pourrait être mis dans un
     fichier de parametre pour eviter de recompiler */
   
    service[0] = p*( mu);  
    departure[0]= (1-p)*(  mu);
    arrival[0] = mu*charge;
    for(i=1;i<NB_QUEUES;i++)
    {
        service[i] = p*( mu);  
        departure[i]= (1-p)*(  mu);
        arrival[i] = mu*charge;
        for(j=i-1;j>=0;j--)
        {
            arrival[i]-= (arrival[j]*pow(p,(double)(i-j)));
        }
        /*     arrival[i ] = 10.0;
        service[i ] = 20.0*(i+1);
        departure[i]= 5.0;*/
     }
    
    total = 0.0;
    
    for(i=0;i<NB_QUEUES;i++)
    {    total+=arrival[i]+service[i]+departure[i];
    }

    for(i=0;i<NB_QUEUES;i++)
    {   Distrib[i] = arrival[i]/total;
        Distrib[i+NB_QUEUES] = departure[i]/total;
        Distrib[i+2*NB_QUEUES] = service[i]/total;
    }
    total = 0.0;
    for(i=0;i<NB_QUEUES;i++)
    {    
        total+=Distrib[i]+ Distrib[i+NB_QUEUES]+Distrib[i+2*NB_QUEUES];
    }
}


int Inverse(double U)
{
    /* Methode de transformée inverse, pas optimisee  */
    int i,j;
    double sum;
    j=SizeDistrib;
    sum =0.0;
    for (i=0; i<SizeDistrib;i++){
        sum+=Distrib[i];
        if (sum>=U) {j=i; break;}
    }
    return(j);
}

void Equation(int* NewS,int indexevt)
{
    /* effet des evenements dans le reseau, indexevt entre 0 et 3NB_QUEUES-1 */
    if (indexevt<NB_QUEUES) {
        if (NewS[indexevt]<Max[indexevt]) {NewS[indexevt]++;} /*  arrivee */
    }
    else if (indexevt<2*NB_QUEUES) {
        if (NewS[indexevt-NB_QUEUES]>Min[indexevt-NB_QUEUES]) {NewS[indexevt-NB_QUEUES]--;} /* depart definitf */
    }
    else if (indexevt<3*NB_QUEUES-1) {
        if (NewS[indexevt-2*NB_QUEUES]>Min[indexevt-2*NB_QUEUES]) {
            NewS[indexevt-2*NB_QUEUES]--;
            if (NewS[indexevt-2*NB_QUEUES+1]<Max[indexevt-2*NB_QUEUES+1]) {NewS[indexevt-2*NB_QUEUES+1]++;}
        } /* transit entre deux files successives */
    }
    else {if (NewS[indexevt-2*NB_QUEUES]>Min[indexevt-2*NB_QUEUES]) {NewS[indexevt-2*NB_QUEUES]--;}
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
    memset(e,0,sizeof(int)*NB_QUEUES);
}

//Return 1 if s1 and s2 are the same, 0 otherwise
int coupling(int* s1, int* s2)
{
    int i=0;
    while(i<NB_QUEUES && s1[i]==s2[i])i++;
    return (i==NB_QUEUES);
}