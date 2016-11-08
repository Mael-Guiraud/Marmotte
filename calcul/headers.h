#include <stdio.h> //printf
#include <stdlib.h>
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
#define MASK32  0xffffffffU
#define JMAX 16


#define NComponent 10
#define SizeDistrib 3*NComponent


void InitWELLRNG512a (unsigned int *seed);
double WELLRNG512a (void);
void init(unsigned int *A,int seed);


typedef int Etat[NComponent];

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


/* Pour  les etats  du modele */


void InitRectangle();
void InitDistribution();
int Inverse(double U);
void Equation(int* OldS,int indexevt);
void F (int * OldS,double U );
void initEtat(Etat e);
int couplage(Etat e1, Etat e2);

