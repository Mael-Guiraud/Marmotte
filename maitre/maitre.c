#include <stdio.h> //printf
#include <stdlib.h>
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <pthread.h> 
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#define NComponent 10

typedef int Etat[NComponent];

typedef struct seed{
    int seed;
    int nb_elems;
}seed;


typedef struct message{
    int indice_Un;
    int nb_elems;
    Etat x0;
    Etat y0;
} message;

typedef struct reponse{
    Etat x0;
    Etat y0;
} reponse;


 //Structure de l'argument passé à la fonction de thread
typedef struct argument{
    int id;     //Socket id
    int numero; //id dans le programme
    reponse *res;
} argument;
 
 //the thread function
void *reception_results(void *);


void affiche_state(Etat e)
{
    int i;
    for(i=0;i<NComponent;i++)
    {
        printf("%d ",e[i]);
    }
    printf("\n");
}

void affiche_all_states(reponse * r,int nb_machines)
{
    int i;
    for(i=0;i<nb_machines;i++)
    {
        printf("Partie %d :\n",i);
        affiche_state(r[i].x0);
        affiche_state(r[i].y0);
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

//renvoie 1 si tous les elements ont couplés, 0 sinon
int detecte_couplage(reponse* res, int nb_machines)
{   
    int i;
    for(i=0;i<nb_machines;i++)
    {
        if(!couplage(res[i].x0,res[i].y0))
            return 0;
    }
    return 1;

}

void cpy_state(Etat e1, Etat e2)
{
    int i;
    for(i=0;i<NComponent;i++)
    {
        e2[i] = e1[i];
    }
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
int main(int argc , char *argv[])
{


    int nombre_machines = 1;
    int n = 500;

	//Initialisation des variables
    int socket_desc, c;
    struct sockaddr_in server;
    struct sockaddr_in* client_addr = malloc(sizeof(struct sockaddr_in)*nombre_machines);
    pthread_t * sniffer_thread = malloc(sizeof(pthread_t)*nombre_machines ); // tableau des id de thread
    int * clients_id = malloc(sizeof(int)*nombre_machines ); // tableay des id de socket
    reponse * res = malloc(sizeof(reponse)*nombre_machines ); // tableau des resultats
    c = sizeof(struct sockaddr_in);
    argument * args = malloc(sizeof(argument)*nombre_machines ); // tableau des arguments que l'on passe aux threads (chaque argument contient une adresse vers le tableau de messages précédent, commun à tous)
    

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


    int i;
    for(i=0;i<nombre_machines;i++)  
    {
		clients_id[i] = accept(socket_desc, (struct sockaddr *)&client_addr[i], (socklen_t*)&c);
		printf("Connection %d acceptée\n",clients_id[i]);
		if (clients_id[i] < 0)
		{
			perror("accept failed");
			return 1;
		}
	}
    seed s;
    s.nb_elems = n;
    s.seed = time(NULL);
    //initialisation 
    for(i=0;i<nombre_machines;i++)
    {
         
        initEtatMIN(res[i].x0);
        initEtatMAX(res[i].y0);
        args[i].res = res;
        args[i].id = clients_id[i];
		args[i].numero = i;
        send(clients_id[i] , &s , sizeof(seed) , 0);
	}
    message m;

    do//tant que tous les elements n'ont pas couplé
    {

        printf("Avant Calcul\n");
        affiche_all_states(res,nombre_machines);
		for(i=0;i<nombre_machines;i++)
		{
            cpy_state(res[i].x0,m.x0);
            cpy_state(res[i].y0,m.y0);
            m.indice_Un = n/nombre_machines*i;
            m.nb_elems = n/nombre_machines;
			send(clients_id[i] , &m , sizeof(message) , 0);
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
        printf("Après Calcul\n");
        affiche_all_states(res,nombre_machines);
		printf("--------------------------\n");
	
    }while(!detecte_couplage(res,nombre_machines));
    close(socket_desc);



    return 0;
}
 
void *reception_results(void *arg)
{
    //Get the socket descriptor
    argument a = *(argument*)arg;
    int read_size;
       

	read_size = recv(a.id , &a.res[a.numero] , sizeof(reponse), 0); 


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




