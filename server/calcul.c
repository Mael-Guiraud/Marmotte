#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>
//socket libs
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "const.h"
#include "alea.h"
#include "simuls.h"


typedef struct message{
	int Un_id;
	int nb_elems;
	int * x0;
	int * y0;
} Message;

double time_diff(struct timeval tv1, struct timeval tv2)
{
    return (((double)tv2.tv_sec*(double)1000 +(double)tv2.tv_usec/(double)1000) - ((double)tv1.tv_sec*(double)1000 + (double)tv1.tv_usec/(double)1000));
}
int main(int argc , char *argv[])
{
	struct timeval tv1, tv2;
    double tps_e=0.0;
    double tps_c=0.0;
    double tps_r=0.0;

    int sock;
    struct sockaddr_in server;
    sock = socket(AF_INET , SOCK_STREAM , 0);

 	if (sock == -1)
    {
        printf("Could not create socket");
    }
     
    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &(int){ 1 }, sizeof(int)) < 0)
        perror("setsockopt(TCP_NODELAY) failed"); 

    if (setsockopt(sock, IPPROTO_TCP, TCP_QUICKACK, &(int){ 1 }, sizeof(int)) < 0)
        perror("setsockopt(TCP_QUICKACK) failed"); 
    if(EXEC_TYPE == 0)
        server.sin_addr.s_addr = inet_addr("127.0.0.1");
    else
        server.sin_addr.s_addr = inet_addr("192.168.90.178");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
    Message m;

    int message_size = sizeof(int)*(NB_QUEUES*2 + 3);
    int reply_size = sizeof(int)*(NB_QUEUES*2+1);
    int interval_size;

     assert(m.x0 = malloc(sizeof(int)*NB_QUEUES)); 
     assert(m.y0 = malloc(sizeof(int)*NB_QUEUES)); 

     int * message;
     assert(message=malloc(message_size));
     int * reply;
     assert(reply = malloc(reply_size));
     int * trajectory;
      //Initialisation de la fonction de calcul
    InitDistribution(LOAD);
	InitRectangle();
	double * sequence =NULL;
	int i;
    int nb_elems;

    while(1)
    {
    	gettimeofday (&tv1, NULL);
		//Reception du message
        if( recv(sock , message, message_size , MSG_WAITALL) <= 0)
        {
            puts("Connection Closed by MASTER");
            break;
        }
            gettimeofday (&tv2, NULL);
            tps_r += time_diff(tv1,tv2);
           gettimeofday (&tv1, NULL); 
        nb_elems = message[2];
        switch(message[0])
        {
            case 0://New seed
            	if(sequence != NULL)free(sequence);
    		    assert(sequence = (double *)malloc(sizeof(double)*nb_elems));
    		    init(B,message[1]);
    		    InitWELLRNG512a(B);
    		    for(i=0;i<nb_elems;i++)
    		    {
    		    	sequence[i]= WELLRNG512a();
    		    }
    		    gettimeofday (&tv2, NULL);
                tps_c += time_diff(tv1,tv2);
                break;       
            default:
            	m.nb_elems = nb_elems;
            	m.Un_id = message[1];
            	cpy_state(&message[3],m.x0);
            	cpy_state(&message[NB_QUEUES+3],m.y0);
    	        if(!coupling(m.x0,m.y0))
    	        {
    	          reply[0]=message[1];

                    if(message[0] == 1)
                    {
        			  for(i=0;i<m.nb_elems;i++)
        				{
        			  		F(m.x0,sequence[i+m.Un_id]);
        			  		F(m.y0,sequence[i+m.Un_id]);
        				}
        	        	cpy_state(m.x0,&reply[1]);
        	        	cpy_state(m.y0,&reply[NB_QUEUES+1]);
        	        	gettimeofday (&tv2, NULL);
                    	tps_c += time_diff(tv1,tv2);
                    	 gettimeofday (&tv1, NULL); 
        		       if( send(sock ,reply, reply_size  , 0) < 0)
        		        {
        		            puts("Send (reply) failed");
        		            break;
        		        }
        		        gettimeofday (&tv2, NULL);
                    	tps_e += time_diff(tv1,tv2);
                    }
                    else // only one trajectory
                    {
                        for(i=0;i<m.nb_elems;i++)
                        {
                            F(m.x0,sequence[i+m.Un_id]);
                        }
                        cpy_state(m.x0,&reply[1]);
                        cpy_state(m.y0,&reply[NB_QUEUES+1]);
                        gettimeofday (&tv2, NULL);
                        tps_c += time_diff(tv1,tv2);
                         gettimeofday (&tv1, NULL); 
                       if( send(sock ,reply, reply_size  , 0) < 0)
                        {
                            puts("Send (reply) failed");
                            break;
                        }
                        gettimeofday (&tv2, NULL);
                        tps_e += time_diff(tv1,tv2);
                    }

    	        }        
    	        else
    	        {
    	        	interval_size= sizeof(int)*(m.nb_elems*NB_QUEUES+1);
    	        	assert(trajectory= malloc(interval_size));
    	        	trajectory[0]=m.Un_id;
    	        	for(i=0;i<m.nb_elems;i++)
    				{
    					F(m.x0,sequence[i+m.Un_id]);
    			  		cpy_state(m.x0,&trajectory[1+i*NB_QUEUES]);
    				}
    				gettimeofday (&tv2, NULL);
                	tps_c += time_diff(tv1,tv2);
                	 gettimeofday (&tv1, NULL); 
    				if( send(sock ,trajectory, interval_size  , 0) < 0)
    		        {
    		            puts("Send (trajectory) failed");
    		            break;
    		        }
    		        gettimeofday (&tv2, NULL);
                	tps_e += time_diff(tv1,tv2);
    		        free(trajectory);

    			}
                break;
        }

    }
    printf("Time spent in calulations = %f \n",tps_c);
    printf("Time spent in emissions = %f \n",tps_e);
    printf("Time spent in receptions+wait = %f \n",tps_r);
    free(m.x0);
	free(reply);
    free(m.y0);
    free(message);
    free(sequence);
    
    close(sock);
    return 0;
}

