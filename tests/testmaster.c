#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <pthread.h> //for threading , link with lpthread
#include <time.h>//pour le caclul du temps 

#define N 1200000
#define NB_MACHINES 6
#define nd N/NB_MACHINES

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

static void init (unsigned int *A)
{
   int i;
   A[0] = 123456789;
   for (i = 1; i < JMAX; i++)
      A[i] = (663608941 * A[i - 1]) & MASK32;
}



 //Structure des messages envoyées entre les machines
  typedef struct message{
	int author;
	double* mess;
} message;
 

 //Structure de l'argument passé à la fonction de thread
typedef struct argument{
	int id;		//Socket id
	int numero;	//id dans le programme
	message *res;
} argument;
 
 
/*
 void affiche_res(message* m, int taille)
 {
	 int i;
	 for(i=0;i<taille;i++)
	 {
		 printf("%d ",m[i].mess);
	 }
	 printf("\n");
 }*/
 void affiche_message(message m)
 {
   int i;
   printf("Author = %d\n",m.author);
   for(i=0;i<nd;i++)
   {
     printf("%f(%d)",m.mess[i],i);
   }
   printf("\n");
 }
 void affiche_tab(double *tab,int taille)
 {
	 int i;
	 for(i=0;i<taille;i++)
	 {
		 printf("%f ",tab[i]);
	 }
	 printf("\n");
 }

 //the thread function
void *reception_results(void *);
 
int main(int argc , char *argv[])
{

 
  printf("Init\n");
	//Initialisation des variables
    int socket_desc, c;
    int nombre_machines = NB_MACHINES;
    int nb_etapes = 1;
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
   int n = N;
   double* vals = malloc(sizeof(double)*n);
    clock_t debut = clock();
    printf("Date avant la génération des %d aléatoires : %f\n",N,(double)debut/ CLOCKS_PER_SEC);
   init (B);
   InitWELLRNG512a (B);
   for (i = 0; i < n; i++) {
      u = WELLRNG512a();
      vals[i]=u;
       //printf ("%f\n", u); 
   }
 printf("date après %f \ntemps de calcul de %d de données : %f s\n",(double)(clock ()) / CLOCKS_PER_SEC,N, (double)(clock () - debut) / CLOCKS_PER_SEC);
    
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
    server.sin_port = htons( 8889 ); //Numero du port
     
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
  int j;
    for(i=0;i<nombre_machines;i++)
    {
        args[i].res = res;
        args[i].id = clients_id[i];
        args[i].numero = i;
        res[i].mess = malloc(sizeof(double)*nd);
        for(j=0;j<nd;j++)
        {
         // printf("%d ",i*nd+j);
          res[i].mess[j] = vals[i*nd+j];
         // printf("%f %f\n",res[i].mess[j],vals[i*nd+j]);
        }
	}

    for(etapes = 0;etapes < nb_etapes;etapes++)
    {
        debut = clock();
         printf("Date avant l'envoi des %d aléatoires : %f\n",N,(double)debut/ CLOCKS_PER_SEC);

		for(i=0;i<nombre_machines;i++)
		{
			
			res[i].author = 1;//on signe le message comme envoyé par le server
     //affiche_message(res[i]);
      printf("on envoie à %d\n",clients_id[i]);
			int val = send(clients_id[i] , (double *)res[i].mess , 8*nd , 0);
      printf("Val de send = %d\n",val);
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
		//affiche_res(res,nombre_machines);
     printf("Date après l'emission (et reception de l'acusé) %f \ntemps d'envoi de %d de données : %f s\n",(double)clock() / CLOCKS_PER_SEC,N, (double)(clock () - debut) / CLOCKS_PER_SEC);

		printf("--------------------------\n");
	}
    close(socket_desc);
    free(vals);


    return 0;
}
 
void *reception_results(void *arg)
{
    //Get the socket descriptor
    argument a = *(argument*)arg;
    int read_size;
    message client_message;    
    int reponse; 
    //Receive a message from client
    //printf("Avant de recevoir (%d,%d)\n",a.id,a.numero);
	read_size = recv(a.id , &reponse , sizeof(reponse), 0); 

     //printf("On a recu %f\n",client_message);
     printf("On a recu %d \n",reponse);
	//On met dans le tableau de resultat ce qu'on a recu
     /*int i;
     for(i=0;i<nd;i++)
     {
       a.res[a.numero].mess[i] = client_message.mess[i];
      }*/

    
  
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
