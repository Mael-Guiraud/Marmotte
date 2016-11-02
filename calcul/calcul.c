#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>



#define W 32
#define R 16
#define P 0
#define M1 13
#define M2 9
#define M3 5

#define MAT0POS(t,v) (v^(v>>t))
#define MAT0NEG(t,v) (v^(v<<(-(t))))
#define MAT3NEG(t,v) (v<<(-(t)))
#define MAT4NEG(t,b,v) (v ^ ((v<<(-(t))) & b))

#define V0            STATE[state_i                   ]
#define VM1           STATE[(state_i+M1) & 0x0000000fU]
#define VM2           STATE[(state_i+M2) & 0x0000000fU]
#define VM3           STATE[(state_i+M3) & 0x0000000fU]
#define VRm1          STATE[(state_i+15) & 0x0000000fU]
#define VRm2          STATE[(state_i+14) & 0x0000000fU]
#define newV0         STATE[(state_i+15) & 0x0000000fU]
#define newV1         STATE[state_i                 ]
#define newVRm1       STATE[(state_i+14) & 0x0000000fU]

#define FACT 2.32830643653869628906e-10

static unsigned int state_i = 0;
static unsigned int STATE[R];
static unsigned int z0, z1, z2;
void InitWELLRNG512a (unsigned int *seed);

double WELLRNG512a (void);

#define MASK32  0xffffffffU
#define JMAX 16
static unsigned int B[JMAX];


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

void InitWELLRNG512a (unsigned int *init)
{
   int j;
   state_i = 0;
   for (j = 0; j < R; j++)
      STATE[j] = init[j];
}

double WELLRNG512a (void)
{
   z0 = VRm1;
   z1 = MAT0NEG(-16, V0) ^ MAT0NEG(-15, VM1);
   z2 = MAT0POS(11, VM2);
   newV1 = z1 ^ z2;
   newV0 = MAT0NEG(-2, z0) ^ MAT0NEG(-18, z1) ^ MAT3NEG(-28, z2) ^ MAT4NEG(-5, 0xda442d24U, newV1);
   state_i = (state_i + 15) & 0x0000000fU;
   return ((double) STATE[state_i]) * FACT;
}
