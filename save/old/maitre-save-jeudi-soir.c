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
#include "constantes.h"


typedef int Etat[NComponent];
typedef Etat trajectoire_partielle[TAILLE_SEQUENCE/(X*NB_MACHINES) +1];

typedef Etat trajectoire_finale[TAILLE_SEQUENCE];


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
    int type_reponse;
    int id_machine;
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
void ecrire_state(FILE * f,Etat e)
{
    int i;
    for(i=0;i<NComponent;i++)
    {
        fprintf(f,"%d ",e[i]);
    }
    fprintf(f,"\n");
}

void affiche_all_states(reponse * r,int nb_machines)
{
    int i;
    for(i=0;i<nb_machines*X;i++)
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


trajectoire_finale RESULT;

int disponibilite_machines[NB_MACHINES];

void affiche_trajectoire_finale()
{
    int i,j;
    for(i=0;i<NB_MACHINES*X;i++)
    {
        printf("\nPartie %d \n",i);
        for(j=0;j<TAILLE_SEQUENCE/(NB_MACHINES*X);j++)
        {
            printf("%d :",i*(TAILLE_SEQUENCE/(NB_MACHINES*X))+j);
            affiche_state(RESULT[i*(TAILLE_SEQUENCE/(NB_MACHINES*X))+j]);
        }
    }
}

void ecrire_trajectoire_finale()
{
    int i,j;
    FILE * F = fopen("Trajectoire","w");
    for(i=0;i<NB_MACHINES*X;i++)
    {
        fprintf(F,"\nPartie %d \n",i);
        for(j=0;j<TAILLE_SEQUENCE/(NB_MACHINES*X);j++)
        {
            fprintf(F,"%d :",i*(TAILLE_SEQUENCE/(NB_MACHINES*X))+j);
            ecrire_state(F,RESULT[i*(TAILLE_SEQUENCE/(NB_MACHINES*X))+j]);
        }
    }
}

void affiche_dispo_machines()
{
    int i;
    for(i=0;i<NB_MACHINES;i++)
    {
        printf("%d ", disponibilite_machines[i]);
    }
    printf("\n");
}

void affiche_recu(int * recu)
{
    int i;
    for(i=0;i<NB_MACHINES*X;i++)
    {
        printf("%d ", recu[i]);
    }
    printf("\n");
}


int premier_libre()
{
    int i;
    for(i=0;i<NB_MACHINES;i++)
    {
        if(disponibilite_machines[i]==1)
            return i;
    }
    return -1;
}

int a_repondu[X*NB_MACHINES];
int main(int argc , char *argv[])
{


    int nombre_machines = NB_MACHINES;
    int n = TAILLE_SEQUENCE;
    int x = X;

	//Initialisation des variables
    int socket_desc, c;
    struct sockaddr_in server;
    struct sockaddr_in* client_addr = malloc(sizeof(struct sockaddr_in)*nombre_machines);
    pthread_t * sniffer_thread = malloc(sizeof(pthread_t)*(x*nombre_machines) ); // tableau des id de thread
    int * clients_id = malloc(sizeof(int)*nombre_machines ); // tableay des id de socket
    reponse * res = malloc(sizeof(reponse)*(nombre_machines*x) ); // tableau des resultats
    c = sizeof(struct sockaddr_in);
    argument * args = malloc(sizeof(argument)*(x*nombre_machines) ); // tableau des arguments que l'on passe aux threads (chaque argument contient une adresse vers le tableau de messages précédent, commun à tous)
    

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
        disponibilite_machines[i] = 1;
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
    int recu[x*nombre_machines];
   
    //initialisation 
    for(i=0;i<x*nombre_machines;i++)
    {
        recu[i]=0;
        a_repondu[i]=0;
         initEtatMIN(res[i].x0);
        if(i==0)
        {
            initEtatMIN(res[i].y0);
        }
        else
        {
            initEtatMAX(res[i].y0);
        }
       
        args[i].res = res;
        
		args[i].numero = i;
        send(clients_id[i] , &s , sizeof(seed) , 0);
	}
    message m;
    int nb_recup = 0;


    int machine_utilisee;
    int nb_tours = 0;
    while(nb_recup != x*nombre_machines)
    {
        
		for(i=0;i<nombre_machines*X;i++)
		{

            if(!recu[i])//si on a pas deja calculé
            {

                machine_utilisee = -1;
                while(machine_utilisee==-1)
                {
                    //printf("%d\n",i);
                    //affiche_dispo_machines(disponibilite_machines);
                    machine_utilisee = premier_libre();
                }
                printf("on shotgun la machine %d pour la tache %d\n",machine_utilisee,i);
                affiche_dispo_machines();
                disponibilite_machines[machine_utilisee]=0;
                args[i].id = clients_id[machine_utilisee];
                args[i].id_machine = machine_utilisee;
               if(couplage(res[i].x0,res[i].y0))//si on attends une trajectoire
                {
                    args[i].type_reponse = 1;
                    recu[i] = 1;
                    nb_recup++; 
                }
                 else
                {
                   args[i].type_reponse = 0;
                }
              
                cpy_state(res[i].x0,m.x0);
                cpy_state(res[i].y0,m.y0);
                m.indice_Un = n/(x*nombre_machines)*i;
                m.nb_elems = n/(x*nombre_machines)+1;
                send(args[i].id , &m , sizeof(message) , 0);

    	        if(pthread_create( &sniffer_thread[i] , NULL ,  reception_results , (void*)&args[i]) < 0)
    	        {
    	            perror("could not create thread");
    	            return 1;
    	        }	
                pthread_detach(sniffer_thread[i]);
                //pthread_join(sniffer_thread[i],NULL);

            }

		}

        


		for(i=0;i<nombre_machines*x;i++)
		{

            while(!a_repondu[i]);
            a_repondu[i]=recu[i];
     
            
		}
        for(i=(nombre_machines*x)-1;i>0;i--)
        {

            cpy_state(res[i-1].x0,res[i].x0);
            cpy_state(res[i-1].y0,res[i].y0); 

     
        }
        
	nb_tours++;	
    printf("On a fait %d tours de boucle et récupéré %d trajectoires\n",nb_tours,nb_recup);
    }
    close(socket_desc);

    ecrire_trajectoire_finale();


    return 0;
}
 
void *reception_results(void *arg)
{

    //printf("Enter\n");
    //Get the socket descriptor
    argument a = *(argument*)arg;
    int read_size;
    printf("On est dans le calcul du thread (%d)(utilisation de la machine %d)\n",a.numero,a.id_machine);
    if(a.type_reponse == 0)
    {
	   read_size = recv(a.id , &a.res[a.numero] , sizeof(reponse), MSG_WAITALL); 
    }
    else
    {
        trajectoire_partielle reponse ;
       read_size = recv(a.id , &reponse , sizeof(trajectoire_partielle), MSG_WAITALL);  
        int i;
       for(i=0;i<TAILLE_SEQUENCE/(X*NB_MACHINES)+1;i++)
        {

            if(i==TAILLE_SEQUENCE/(X*NB_MACHINES)) // derniere etape
            {
                if(a.numero!=(NB_MACHINES*X)-1)
                {
                    //on ecrit dans l'etat initial de la prochaine iteration
                    cpy_state(reponse[i],a.res[a.numero].x0);
                    cpy_state(reponse[i],a.res[a.numero].y0);
                }
            }
            else
            {
                cpy_state(reponse[i],RESULT[a.numero*(TAILLE_SEQUENCE/(NB_MACHINES*X))+i]);
            }
       }
    }        
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
    a_repondu[a.numero]=1;
    //printf("%d remis en service--------------------------\n",a.id_machine);
    disponibilite_machines[a.id_machine] = 1;
    //printf("[%d]\n",disponibilite_machines[a.id_machine]);
    //affiche_dispo_machines();
    return 0;
}




