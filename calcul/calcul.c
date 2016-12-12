#include "headers.h"


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
    sock = socket(AF_INET , SOCK_STREAM , 0);
 	if (sock == -1)
    {
        printf("Could not create socket");
    }
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    //server.sin_addr.s_addr = inet_addr("192.168.90.219");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
    message m;

    int taille_message = sizeof(int)*(NComponent*2 + 3);
    int taille_reponse = sizeof(int)*(NComponent*2+1);
    int taille_trajectoire;

     m.x0 = malloc(sizeof(int)*NComponent); 
     m.y0 = malloc(sizeof(int)*NComponent); 

     int * MESSAGE=malloc(taille_message);
     int * REPONSE = malloc(taille_reponse);
     int * TRAJECTOIRE;
      //Initialisation de la fonction de calcul
    InitDistribution(1.0);
	InitRectangle();
	double * sequence =NULL;
	int i;
    int nb_elems;



    int continuer = 1;

    while(continuer)
    {
		//Reception du message
        if( recv(sock , MESSAGE, taille_message , MSG_WAITALL) <= 0)
        {
            puts("Connection Closed by MASTER");
            break;
        }
        nb_elems = MESSAGE[2];
        if(MESSAGE[0] == 0)//Si on doit générer une nouvelle graine
        {
        	if(sequence != NULL)free(sequence);//Si ce n'est pas la premiere initialisation, on free l'ancienne sequence 
		    sequence = (double *)malloc(sizeof(double)*nb_elems);
		    init(B,MESSAGE[1]);
		    InitWELLRNG512a(B);
		    for(i=0;i<nb_elems;i++)
		    {
		    	sequence[i]= WELLRNG512a();
		    }
        }
        else
        {
        	m.nb_elems = nb_elems;
        	m.indice_Un = MESSAGE[1];
        	cpy_state(&MESSAGE[3],m.x0);
        	cpy_state(&MESSAGE[NComponent+3],m.y0);
	        if(!couplage(m.x0,m.y0))
	        {
	          REPONSE[0]=MESSAGE[1];
			  for(i=0;i<m.nb_elems;i++)
				{
			  		F(m.x0,sequence[i+m.indice_Un]);
			  		F(m.y0,sequence[i+m.indice_Un]);
				}
	        	cpy_state(m.x0,&REPONSE[1]);
	        	cpy_state(m.y0,&REPONSE[NComponent+1]);
		       if( send(sock ,REPONSE, taille_reponse  , 0) < 0)
		        {
		            puts("Send (reponse) failed");
		            break;
		        }
	        }        
	        else
	        {
	        	taille_trajectoire= sizeof(int)*(m.nb_elems*NComponent+1);
	        	TRAJECTOIRE= malloc(taille_trajectoire);
	        	TRAJECTOIRE[0]=m.indice_Un;
	        	for(i=0;i<m.nb_elems;i++)
				{
					F(m.x0,sequence[i+m.indice_Un]);
			  		cpy_state(m.x0,&TRAJECTOIRE[1+i*NComponent]);
				}
				if( send(sock ,TRAJECTOIRE, taille_trajectoire  , 0) < 0)
		        {
		            puts("Send (trajectoire) failed");
		            break;
		        }
		        free(TRAJECTOIRE);

			}
        }

    }
    free(m.x0);
	free(REPONSE);
    free(m.y0);
    free(MESSAGE);
    free(sequence);
    
    close(sock);
    return 0;
}

