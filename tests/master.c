#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>


#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include "nbq.h"
double time_diff(struct timeval tv1, struct timeval tv2)
{
    return (((double)tv2.tv_sec*(double)1000 +(double)tv2.tv_usec/(double)1000) - ((double)tv1.tv_sec*(double)1000 + (double)tv1.tv_usec/(double)1000));
}
int main(int argc, char const *argv[])
{
	struct sockaddr_in server;

    int server_id; 

    server.sin_family = AF_INET; //IPV4 add
    server.sin_port = htons( 8888 ); //port number

	if ( (server_id = socket(AF_INET , SOCK_STREAM , 0))== -1)
	        printf("Could not create socket");

		///////// Sockets options /////////
		//Allow the socket to be re-used after being closed
	if (setsockopt(server_id, SOL_SOCKET, SO_REUSEPORT, &(int){ 1 }, sizeof(int)) < 0)
	        perror("setsockopt(SO_REUSEADDR) failed");

	if (setsockopt(server_id, IPPROTO_TCP, TCP_NODELAY, &(int){ 1 }, sizeof(int)) < 0)
	        perror("setsockopt(TCP_NODELAY) failed");

	 server.sin_addr.s_addr = inet_addr("192.168.90.10");
	  
		
	if(connect(server_id,(struct sockaddr *) &server, sizeof(struct sockaddr)) == -1)
    {
        perror("Connection failed");
        return 0;
    }

    struct timeval tv1, tv2;
    int size_buff_send = 10 * 2 +4;
    int sendbuf[size_buff_send];
    int size_buff_recv = NB_QUEUES +1;
	int  recvbuf[size_buff_recv];
    double average = 0.0;
    double time_dif = 0.0;

	
	for(int i=0;i<100;i++)
	{
		gettimeofday (&tv1, NULL);
		send(server_id, &sendbuf, sizeof(int)*size_buff_send, 0);
		recv(server_id, &recvbuf, sizeof(int)*size_buff_recv, MSG_WAITALL);
		gettimeofday (&tv2, NULL);
		time_dif = time_diff(tv1,tv2);
		average += time_dif;
		//printf(" Temps = %f .\n",time_dif);
	}
	printf("%d %f \n",NB_QUEUES,average/100);
	
	close(server_id);
    	
	return 0;
}