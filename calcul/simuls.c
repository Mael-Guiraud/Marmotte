#include "simuls.h"





void InitRectangle()
{
    int i;
    for(i=0;i<NComponent;i++)
    {
        Min[i ] = 0;
        Max[i ] = 100;
    }
}

void InitDistribution()
{
    int i;
    double total;
    /* construit la distribution pour un reseau en tandem */
    /* la premeire partie permet de coder les taux d'arrivee, puis de sortie definitive
     et enfin de service
     pour aller a la file suivante . Ca pourrait être mis dans un
     fichier de parametre pour eviter de recompiler */
    
    
    for(i=0;i<NComponent;i++)
    {
       arrival[i ] = 50.0;
        if(i==0)
        {
            service[i ] = 3*( arrival[i] )/4;  
            departure[i]= ( arrival[i]  )/4;
        }
        else
        {
            service[i ] = 3*( arrival[i] +  service[i-1] )/4;  
            departure[i]= ( arrival[i] + service[i-1] )/4;
        }
    }
    
    total = 0.0;
    
    for(i=0;i<NComponent;i++)
    {    total+=arrival[i]+service[i]+departure[i];
    }
    for(i=0;i<NComponent;i++)
    {   Distrib[i] = arrival[i]/total;
        Distrib[i+NComponent] = departure[i]/total;
        Distrib[i+2*NComponent] = service[i]/total;
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
    /* effet des evenements dans le reseau, indexevt entre 0 et 3NComponent-1 */
    if (indexevt<NComponent) {
        if (NewS[indexevt]<Max[indexevt]) {NewS[indexevt]++;} /*  arrivee */
    }
    else if (indexevt<2*NComponent) {
        if (NewS[indexevt-NComponent]>Min[indexevt-NComponent]) {NewS[indexevt-NComponent]--;} /* depart definitf */
    }
    else if (indexevt<3*NComponent-1) {
        if (NewS[indexevt-2*NComponent]>Min[indexevt-2*NComponent]) {
            NewS[indexevt-2*NComponent]--;
            if (NewS[indexevt-2*NComponent+1]<Max[indexevt-2*NComponent+1]) {NewS[indexevt-2*NComponent+1]++;}
        } /* transit entre deux files successives */
    }
    else {if (NewS[indexevt-2*NComponent]>Min[indexevt-2*NComponent]) {NewS[indexevt-2*NComponent]--;}
        /* depart definitf, la derniere file est particuliere */
    }
}

void F (int * OldS,double U )
{ int indexevt;
    indexevt = Inverse(U);
    Equation(OldS,indexevt);
}


void initEtat(Etat e)
{
    int i;
    for(i=0;i<NComponent;i++)
    {
        e[i] = 0;
    }
}

//retourne 1 si deux etats ont couplé
int couplage(Etat e1, Etat e2)
{
    int i;
    for(i=0;i<NComponent;i++)
    {
        if(e1[i]!=e2[i])
            return 0;
    }
    return 1;
}