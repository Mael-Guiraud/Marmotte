/*simulation d'une reseau en tandem de 10 files
 avec buffer entre 0 et 100 pour chaque file */

#define NComponent 10
#define SizeDistrib 3*NComponent


static int Min[NComponent], Max[NComponent];
static double Distrib[SizeDistrib];
static double arrival[NComponent], service[NComponent], departure[NComponent];

/* Pour  les etats  du modele */

typedef int Etat[NComponent];

/* ces deux fonctions sont à faire au debut, une seule fois... */

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
        arrival[i ] = 10.0;
        service[i ] = 20.0*i;
        departure[i]= 5.0;
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

void Equation(int* OldS,int indexevt,int* NewS)
{
    int i;
    /* effet des evenements dans le reseau, indexevt entre 0 et 3NComponent-1 */
    for(i=0;i<NComponent;i++)
    {    NewS[i]=OldS[i];
    }
    if (indexevt<NComponent) {
        if (NewS[i]<Max[i]) {NewS[i]++;} /*  arrivee */
    }
    else if (indexevt<2*NComponent) {
        if (NewS[i-NComponent]>Min[i-NComponent]) {NewS[i-NComponent]--;} /* depart definitf */
    }
    else if (indexevt<3*NComponent-1) {
        if (NewS[i-NComponent]>Min[i-NComponent]) {
            NewS[i-NComponent]--;
            if (NewS[i-NComponent+1]<Max[i-NComponent+1]) {NewS[i-NComponent+1]++;}
        } /* transit entre deux files successives */
    }
    else {if (NewS[i-2*NComponent]>Min[i-2*NComponent]) {NewS[i-2*NComponent]--;}
        /* depart definitf, la derniere file est particuliere */
    }
}

void F (int * OldS,double U ,int * NewS)
{ int indexevt;
    indexevt = Inverse(U);
    Equation(OldS,indexevt,NewS);
}


/*   Il faut que ta fonction appelante ait defini les etats OldS et NewS (de type Etat)
 et ait genere le U par un random. */


