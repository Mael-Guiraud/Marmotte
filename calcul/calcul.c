#include "headers.h"


unsigned int state_i = 0;
unsigned int STATE[R];
unsigned int z0, z1, z2;
unsigned int B[JMAX];


int Min[NComponent], Max[NComponent];
double Distrib[SizeDistrib];
double arrival[NComponent], service[NComponent], departure[NComponent];


void affiche_state(Etat e)
{
	int i;
	for(i=0;i<NComponent;i++)
	{
		printf("%d ",e[i]);
	}
	printf("\n");
}

void cpy_state(Etat e1, Etat e2)
{
	int i;
	for(i=0;i<NComponent;i++)
	{
		e2[i] = e1[i];
	}
}
 
int main(int argc , char *argv[])
{

    int sock;
    struct sockaddr_in server;
	//Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
 	if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
    server.sin_addr.s_addr = inet_addr("169.254.147.32");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected\n");

    //Reception de la graine et génération de la sequence aléa
    seed s;
    if( recv(sock , &s , sizeof(seed) , MSG_WAITALL) <= 0)
    {
        puts("Connection Closed");
        return 0;
    }
    printf("Génération de la séquence Aléatoire...\n");
    int i;
    double * sequence = (double *)malloc(sizeof(double)*s.nb_elems);
    init(B,s.seed);
    InitWELLRNG512a(B);
    for(i=0;i<s.nb_elems;i++)
    {
    	sequence[i]= WELLRNG512a();
    }

    //Initialisation de la fonction de calcul
    InitDistribution();
	InitRectangle();

     message m; 
     reponse r;
    //keep communicating with server
    while(1)
    {
		//Receive a reply from the server
        if( recv(sock , &m , sizeof(message) , MSG_WAITALL) <= 0)
        {
            puts("Connection Closed");
            break;
        }
        printf("on a recu :\n");
          affiche_state(m.x0);
          affiche_state(m.y0);
        //On n'a pas couplé à l'étape d'avant.
        if(!couplage(m.x0,m.y0))
        {
          
		  for(i=0;i<m.nb_elems;i++)
			{
		  		F(m.x0,sequence[i+m.indice_Un]);
		  		F(m.y0,sequence[i+m.indice_Un]);
			}
			cpy_state(m.x0,r.x0);
			cpy_state(m.y0,r.y0);

          printf("on a renvoi :\n");
          affiche_state(r.x0);
          affiche_state(r.y0);
          printf("\n\n");
		   //Send some data
	       if( send(sock , &r , sizeof(reponse) , 0) < 0)
	        {
	            puts("Send failed");
	            break;
	        }
	        printf("Retour de reponse\n");
        }        
        else
        {
        	printf("COUPLAGE\n");
        	break;
        }
       

    }
     
    close(sock);
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


void init (unsigned int *A,int seed)
{
   int i;
   A[0] = seed;
   for (i = 1; i < JMAX; i++)
      A[i] = (663608941 * A[i - 1]) & MASK32;
}


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