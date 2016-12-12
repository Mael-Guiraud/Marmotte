#include <stdio.h> 
#include <stdlib.h>
#include <string.h>    //strlen
#include <sys/socket.h>    //socekt
#include <arpa/inet.h> //inet_addr
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include "headers.h"
#include "simul.h"
#include <stdarg.h>


int* creation_sockets(int * socket_desc)
{
    int c,i;
    struct sockaddr_in server;
    struct sockaddr_in* client_addr = malloc(sizeof(struct sockaddr_in)*NB_MACHINES);
    assert(client_addr);
    int * clients_id = malloc(sizeof(int)*NB_MACHINES ); // tableau des id de socket
    assert(clients_id);
    c = sizeof(struct sockaddr_in);
    *socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (*socket_desc == -1)
    {
        printf("Could not create socket");
    }
    if (setsockopt(*socket_desc, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &(int){ 1 }, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    server.sin_family = AF_INET; //adresse IPV4
    server.sin_addr.s_addr = INADDR_ANY; // Recevoir sur n'importe laquelle des adresses de la machine
    server.sin_port = htons( 8888 ); //Numero du port
     
    if( bind(*socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return NULL;
    }
    listen(*socket_desc , 5);

    for(i=0;i<NB_MACHINES;i++)  
    {
        clients_id[i] = accept(*socket_desc, (struct sockaddr *)&client_addr[i], (socklen_t*)&c);
        if (clients_id[i] < 0)
        {
            perror("accept failed");
            return NULL;
        }
    }

    free(client_addr);
    return clients_id;
}

void ecrire_fichier_res(FILE * f,int taille_inter,double e,double t)
{
    fprintf(f,"%d %f %f\n",taille_inter,e,t);
    printf("%d %f %f\n",taille_inter,e,t);
}

int main(int argc , char *argv[])
{
    TAILLE_SEQUENCE = 320000;
    int * clients_id;
    int socket_desc;
    if(!(clients_id= creation_sockets(&socket_desc)))
        {
            printf("Erreur lors de la création des sockets");
            exit(1);
        }

    int i;
    int nb_simuls=100;
    int taille_inter;
    simulation_result res;
    double temps_total ;
    double etapes_total;
    FILE* f = fopen("results_moyennes.data","w");
    for(taille_inter=50000;taille_inter>= 1000;taille_inter-=5000)
    {      
        NB_INTER = TAILLE_SEQUENCE/taille_inter;
        TAILLE_TRAJECTOIRE_PARTIELLE = TAILLE_SEQUENCE/(NB_INTER) +1;
        printf("Simulation pour TAILLE_SEQUENCE = %d et NB_INTER = %d (TAILLE_INTER = %d)\n",TAILLE_SEQUENCE,NB_INTER,TAILLE_TRAJECTOIRE_PARTIELLE);
        etapes_total =0;
        temps_total=0;
        for(i=0;i<nb_simuls;i++)
        {
            if(nb_simuls>=100)
            {
                if(i%(nb_simuls/100) == 0)fprintf(stdout,"\r[%3d%%]",i/(nb_simuls/100)+1);
                fflush(stdout);
            }
            else
            {
                fprintf(stdout,"\rEtape%3d",i+1);fflush(stdout);
            }
            res= simul(clients_id);
            etapes_total+=res.etapes;
            temps_total+=res.temps;
        }printf("\n");

        ecrire_fichier_res(f,NB_INTER,temps_total/nb_simuls,etapes_total/nb_simuls);
    }

    fclose(f);
    free(clients_id);
    close(socket_desc);

        
    return 0;
}
 
