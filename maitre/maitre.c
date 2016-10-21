#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <pthread.h> //for threading , link with lpthread
 

 //Structure des messages envoyées entre les machines
  typedef struct message{
	int author;
	int mess;
} message;
 

 //Structure de l'argument passé à la fonction de thread
typedef struct argument{
	int id;		//Socket id
	int numero;	//id dans le programme
	message *res;
} argument;
 
 

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
    c = sizeof(struct sockaddr_in);
    struct sockaddr_in server;
    struct sockaddr_in* client_addr = malloc(sizeof(struct sockaddr_in)*nombre_machines);
    pthread_t * sniffer_thread = malloc(sizeof(pthread_t)*nombre_machines ); // tableau des id de thread
    int * clients_id = malloc(sizeof(int)*nombre_machines ); // tableay des id de socket
    message * res = malloc(sizeof(message)*nombre_machines ); // tableau des resultats
    argument * args = malloc(sizeof(argument)*nombre_machines ); // tableau des arguments que l'on passe aux threads (chaque argument contient une adresse vers le tableau de messages précédent, commun à tous)
    int i=0;
    int etapes;


     
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


    for(etapes = 0;etapes < 150;etapes++)
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
