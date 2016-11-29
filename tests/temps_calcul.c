/*asimulation d'une reseau en tandem de 10 files
 avec buffer entre 0 et 100 pour chaque file */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>





#define W 32
#define R 16
#define P 0
#define M1 13
#define M2 9
#define M3 5

#define MAT0POS(t,v) (v^(v>>t))
#define MAT0NEG(t,v) (v^(v<<(-(t))))
#define MAT3NEG(t,v) (v<<(-(t)))
#define MAT4NEG(t,b,v) (v ^ ((v<<(-(t))) & b))

#define V0            STATE[state_i                   ]
#define VM1           STATE[(state_i+M1) & 0x0000000fU]
#define VM2           STATE[(state_i+M2) & 0x0000000fU]
#define VM3           STATE[(state_i+M3) & 0x0000000fU]
#define VRm1          STATE[(state_i+15) & 0x0000000fU]
#define VRm2          STATE[(state_i+14) & 0x0000000fU]
#define newV0         STATE[(state_i+15) & 0x0000000fU]
#define newV1         STATE[state_i                 ]
#define newVRm1       STATE[(state_i+14) & 0x0000000fU]

#define FACT 2.32830643653869628906e-10

static unsigned int state_i = 0;
static unsigned int STATE[R];
static unsigned int z0, z1, z2;


void InitWELLRNG512a (unsigned int *seed);

double WELLRNG512a (void);

#define MASK32  0xffffffffU
#define JMAX 16
static unsigned int B[JMAX];

static void init (unsigned int *A,int seed)
{
   int i;
   A[0] = seed;
   for (i = 1; i < JMAX; i++)
      A[i] = (663608941 * A[i - 1]) & MASK32;
}








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
        service[i ] = 20.0*(i+1);
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

void Equation(int* NewS,int indexevt)
{
    int i;

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
/*   Il faut que ta fonction appelante ait defini les etats OldS et NewS (de type Etat)
 et ait genere le U par un random. */

void afficheEtat(Etat e)
{
    int i;
    for(i=0;i<NComponent;i++)
    {
        printf("%d ",e[i]);
    }
    printf("\n");
}
void initEtatMIN(Etat e)
{
    int i;
    for(i=0;i<NComponent;i++)
    {
        e[i] = 0;
    }
}

void initEtatMAX(Etat e)
{
    int i;
    for(i=0;i<NComponent;i++)
    {
        e[i] = 100;
    }
}
int main(){
    
   int graine = time(NULL);
    init(B,graine);
    InitWELLRNG512a (B);
    InitDistribution();
    InitRectangle();

    Etat min, max;

    int i,j;

    struct timeval tv1, tv2;
    long long temps;
    
    int TAILLE = 120000;
    double * u = malloc(sizeof(double)*TAILLE);

    initEtatMIN(min);
    initEtatMAX(max);
    //FILE *f = fopen("res_temps","w");
    

    for(j=0;j<10;j++)
    { 
        gettimeofday (&tv1, NULL);
        //printf("Calcul sur 1 processeur d'une sequence de taille %d.\n",TAILLE);
    
        for(i=0;i<TAILLE;i++)
         {
              u[i] = WELLRNG512a();
                    
         }
        for(i=0;i<TAILLE;i++)
         {

                F(min,u[i]);
                F(max,u[i]);
            //if(i%(TAILLE/100)==0)printf("\r[%3d%%]",i/(TAILLE/100));     
         }
        gettimeofday (&tv2, NULL);
        temps += ( (tv2.tv_sec*1000LL +tv2.tv_usec/1000) - (tv1.tv_sec*1000LL-tv1.tv_usec/1000));
        //fprintf(f,"%d %lld \n",j,temps);

        //FAIRE UNE MOYENNE
    }
    printf("temps_moyen :%d\n",temps/j);
    
    free(u);
    return 0;

}


void InitWELLRNG512a (unsigned int *init)
{
   int j;
   state_i = 0;
   for (j = 0; j < R; j++)
      STATE[j] = init[j];
}

double WELLRNG512a (void)
{
   z0 = VRm1;
   z1 = MAT0NEG(-16, V0) ^ MAT0NEG(-15, VM1);
   z2 = MAT0POS(11, VM2);
   newV1 = z1 ^ z2;
   newV0 = MAT0NEG(-2, z0) ^ MAT0NEG(-18, z1) ^ MAT3NEG(-28, z2) ^ MAT4NEG(-5, 0xda442d24U, newV1);
   state_i = (state_i + 15) & 0x0000000fU;
   return ((double) STATE[state_i]) * FACT;
}
