#include <stdio.h>
#include <stdlib.h> 
static int taille_struct;
  typedef struct message{
	int author;
	double mess[taille_struct];

} message;

int main()
{
  FILE * f = fopen("config.cfg","r");
  message m;
  printf("%d\n",(int)sizeof(message));
  fscanf(f,"%d",&taille_struct);
  printf("%d\n",taille_struct);
  printf("%d\n",(int)sizeof(message));
  return 0;
}