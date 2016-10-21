#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>


  typedef struct message{
	int author;
	int mess;
} message;
 
int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;
    char server_reply[2000];
     message m;
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr("169.254.147.32");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected\n");
     
    //keep communicating with server
    while(1)
    {
		//Receive a reply from the server
        if( recv(sock , &m , sizeof(message) , 0) <= 0)
        {
            puts("Connection Closed");
            break;
        }
        //Calculation
        printf("on a recu %d (author = %d )\n",m.mess,m.author);

        //Calcul de la réponse
        m.author = m.mess;
        m.mess++; 
          
        
        
        //Send some data
        if( send(sock , &m , sizeof(message) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
        printf("reemis\n");
         

    }
     
    close(sock);
    return 0;
}
