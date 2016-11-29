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
	//Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
 	if (sock == -1)
    {
        printf("Could not create socket");
    }
   // puts("Socket created");
    server.sin_addr.s_addr = inet_addr("169.254.147.32");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    //puts("Connected\n");

    //Reception de la graine et génération de la sequence aléa
    seed s;
    if( recv(sock , &s , sizeof(seed) , MSG_WAITALL) <= 0)
    {
        puts("Connection Closed");
        return 0;
    }
    //printf("Génération de la séquence Aléatoire...\n");
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
      struct timeval tv1, tv2,tv3;
    long long temps;
    long long temps2;
    
    //keep communicating with server
    while(1)
    {
		//Receive a reply from the server
        if( recv(sock , &m , sizeof(message) , MSG_WAITALL) <= 0)
        {
            puts("Connection Closed");
            break;
        }
    
 		gettimeofday (&tv1, NULL);
        //On n'a pas couplé à l'étape d'avant.
        if(!couplage(m.x0,m.y0))
        {
        	//printf("Recherche d'un couplage entre deux trajectoires de %d etapes.\n",m.nb_elems);
          reponse r;
		  for(i=0;i<m.nb_elems;i++)
			{
		  		F(m.x0,sequence[i+m.indice_Un]);
		  		F(m.y0,sequence[i+m.indice_Un]);
			}
			cpy_state(m.x0,r.x0);
			cpy_state(m.y0,r.y0);
			gettimeofday (&tv2, NULL);
    		temps = ( (tv2.tv_sec*1000LL +tv2.tv_usec/1000) - (tv1.tv_sec*1000LL-tv1.tv_usec/1000));
    		//printf("temps passé en calcul sans couplage %lld ms\n",temps);
		   //Send some data
	       if( send(sock , &r , sizeof(reponse) , 0) < 0)
	        {
	            puts("Send failed");
	            break;
	        }
	        gettimeofday (&tv3, NULL);
 		  	 temps = ( (tv3.tv_sec*1000LL +tv3.tv_usec/1000) - (tv2.tv_sec*1000LL-tv2.tv_usec/1000));
 		  	 temps2 = ( (tv3.tv_sec*1000LL +tv3.tv_usec/1000) - (tv1.tv_sec*1000LL-tv1.tv_usec/1000));
   			 //printf("temps de réémission = %lld ms (depuis reception = %lld\n",temps,temps2);
        }        
        else
        {
        	//printf("Calcul d'une trajectoire de %d etapes.\n",m.nb_elems);
        	//printf("on renvoi la trajectoire \n");
        	trajectoire_partielle r;
        	for(i=0;i<m.nb_elems;i++)
			{
				
				cpy_state(m.x0,r[i]);
				//printf("%d :",i);
				//affiche_state(r[i]);
		  		F(m.x0,sequence[i+m.indice_Un]);

			}
        	gettimeofday (&tv2, NULL);
    		temps = ( (tv2.tv_sec*1000LL +tv2.tv_usec/1000) - (tv1.tv_sec*1000LL-tv1.tv_usec/1000));
    		//printf("temps passé en calcul avec couplage %lld ms\n",temps);
		   //Send some data
	       if( send(sock , &r , sizeof(trajectoire_partielle) , 0) < 0)
	        {
	            puts("Send failed");
	            break;
	        }
	         gettimeofday (&tv3, NULL);
 		  	 temps = ( (tv3.tv_sec*1000LL +tv3.tv_usec/1000) - (tv2.tv_sec*1000LL-tv2.tv_usec/1000));
 		  	 temps2 = ( (tv3.tv_sec*1000LL +tv3.tv_usec/1000) - (tv1.tv_sec*1000LL-tv1.tv_usec/1000));
   			 //printf("temps de réémission = %lld ms (depuis reception = %lld\n",temps,temps2);

        }

           

    }
     free(sequence);
    close(sock);
    return 0;
}

