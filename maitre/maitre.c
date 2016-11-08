#include "headers.h"


unsigned int state_i = 0;
unsigned int STATE[R];
unsigned int z0, z1, z2;
unsigned int B[JMAX];


int Min[NComponent], Max[NComponent];
double Distrib[SizeDistrib];
double arrival[NComponent], service[NComponent], departure[NComponent];


 void affiche_res(message* m, int taille)
 {
	 int i;
	 for(i=0;i<taille;i++)
	 {
		 printf("%d ",m[i].mess);
	 }
	 printf("\n");
 }

 void affiche_tab(int *tab,int taille)
 {
	 int i;
	 for(i=0;i<taille;i++)
	 {
		 printf("%d ",tab[i]);
	 }
	 printf("\n");
 }

 //the thread function
void *reception_results(void *);
 
int main(int argc , char *argv[])
{

 

	//Initialisation des variables
    int socket_desc, c;
    int nombre_machines = 6;
    int nb_etapes = 5;
    c = sizeof(struct sockaddr_in);
    struct sockaddr_in server;
    struct sockaddr_in* client_addr = malloc(sizeof(struct sockaddr_in)*nombre_machines);
    pthread_t * sniffer_thread = malloc(sizeof(pthread_t)*nombre_machines ); // tableau des id de thread
    int * clients_id = malloc(sizeof(int)*nombre_machines ); // tableay des id de socket
    message * res = malloc(sizeof(message)*nombre_machines ); // tableau des resultats
    argument * args = malloc(sizeof(argument)*nombre_machines ); // tableau des arguments que l'on passe aux threads (chaque argument contient une adresse vers le tableau de messages précédent, commun à tous)
    int i=0;
    int etapes;

   double u, som = 0;
   int n = 1000000;
    clock_t debut = clock();
   init (B);
   InitWELLRNG512a (B);
   for (i = 0; i < n; i++) {
      u = WELLRNG512a();
      som += u;
      // printf ("%f\n", u); 
   }
 printf("temps de calcul de 1 000 000 de données : %f s\n", (double)(clock () - debut) / CLOCKS_PER_SEC);

     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }

    puts("Socket created");
    //Option pour que l'adresse puisse être réutilisée directement après la fin du programme
    if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &(int){ 1 }, sizeof(int)) < 0)
    	perror("setsockopt(SO_REUSEADDR) failed");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET; //adresse IPV4
    server.sin_addr.s_addr = INADDR_ANY; // Recevoir sur n'importe laquelle des adresses de la machine
    server.sin_port = htons( 8888 ); //Numero du port
     
    //Bind = association de la socket au port et adresse
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }


    puts("bind done");

    //Listen = peut mettre en attente 5 connection a la fois
    listen(socket_desc , 5);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");


    for(i=0;i<nombre_machines;i++)  
    {
		clients_id[i] = accept(socket_desc, (struct sockaddr *)&client_addr[i], (socklen_t*)&c);
		printf("Socket %d acceptée\n",clients_id[i]);
		if (clients_id[i] < 0)
		{
			perror("accept failed");
			return 1;
		}
	}

    //affiche_tab(clients_id,nombre_machines);
    //initialisation 
    for(i=0;i<nombre_machines;i++)
    {
        args[i].res = res;
        args[i].id = clients_id[i];
		args[i].numero = i;
		res[i].mess = clients_id[i];
	}


    for(etapes = 0;etapes < nb_etapes;etapes++)
    {
		for(i=0;i<nombre_machines;i++)
		{
			//printf("Envoi à %d\n",clients_id[i]); 
			res[i].author = 1;//on signe le message comme envoyé par le server
			send(clients_id[i] , &res[i] , sizeof(message) , 0);
			//printf("envoyé\n");
			//printf("Creation de thread i=%d\n",i);

	        if(pthread_create( &sniffer_thread[i] , NULL ,  reception_results , (void*)&args[i]) < 0)
	        {
	            perror("could not create thread");
	            return 1;
	        }		
		}
		//printf("On attends la réponse de tout le monde \n");
		for(i=0;i<nombre_machines;i++)
		{
            //printf("join %d\n",i);
			pthread_join( sniffer_thread[i] , NULL);
		}
		affiche_res(res,nombre_machines);
		printf("--------------------------\n");
	}

    close(socket_desc);



    return 0;
}
 
void *reception_results(void *arg)
{
    //Get the socket descriptor
    argument a = *(argument*)arg;
    int read_size;
    message client_message;     
    //Receive a message from client
    //printf("Avant de recevoir (%d,%d)\n",a.id,a.numero);
	read_size = recv(a.id , &client_message , sizeof(message), 0); 

     //printf("On a recu %d\n",client_message.mess);
	//On met dans le tableau de resultat ce qu'on a recu
     a.res[a.numero].mess = client_message.mess;
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else 
	{
		if(read_size == -1)
	    {
	        perror("recv failed");
	    }
	}   
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