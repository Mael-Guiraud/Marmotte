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
	double mess[200000];
} message;
 

  void affiche_message(message m)
 {
   int i;
   printf("Author = %d\n",m.author);
   for(i=0;i<200000;i++)
   {
     printf("%f(%d) ",m.mess[i],i);
   }
   printf("\n");
 }

int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;
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
    server.sin_port = htons( 8889 );
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected\n");
     int nb_objet = 0;
     int recvnumber;
    //keep communicating with server
    while(1)
    {
		//Receive a reply from the server
      recvnumber = recv(sock , (message *)&m , sizeof(message) , MSG_WAITALL);
        if(recvnumber <= 0)
        {
            puts("Connection Closed");
            break;
        }
        //Calculation
        printf("on a recu %f (author = %d /%d) \n",m.mess[0],m.author,recvnumber);
       //affiche_message(m);
       nb_objet++;
        //Calcul de la réponse
        m.author = 0;
             
        
        
        //Send some data
        if( send(sock , &nb_objet , sizeof(nb_objet) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
        printf("reemis(%d)\n",nb_objet);
         
        printf("%d\n",sizeof(message));
    }
     
    close(sock);
    return 0;
}

