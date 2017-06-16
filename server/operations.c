#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "alea.h"

int initialize_set(fd_set *set, int server_socket, int master_socket)
{
	FD_ZERO(set);
	int max_sd = server_socket;

	//if valid socket descriptor then add to read list
	FD_SET(server_socket, set);
	if(master_socket > 0)
		FD_SET( master_socket , set);

	//highest file descriptor number, need it for the select function
	if(master_socket > max_sd)
		max_sd = master_socket;
	return max_sd;
}
double intToDouble(int* tab,int indice)
{
	char  * nbr ;
	assert(nbr=calloc(64,sizeof(char)));
	
	for(int i=indice;i<indice+3;i++)
	{
		sprintf(nbr,"%s%d",nbr,tab[i]);
	}
	sprintf(nbr,"%s.",nbr);
	for(int i=indice+4;i<indice+8;i++)
	{
		sprintf(nbr,"%s%d",nbr,tab[i]);
	}
	double retour = atof(nbr);
	free(nbr);
	return retour;

}
double intToDoubleLoad(int* tab,int indice)
{
	char  * nbr ;
	assert(nbr=calloc(64,sizeof(char)));

	sprintf(nbr,"%s%d.",nbr,tab[indice]);
	for(int i=indice+2;i<indice+4;i++)
	{
		sprintf(nbr,"%s%d",nbr,tab[i]);
	}
	double retour = atof(nbr);
	free(nbr);
	return retour;

}

int** init_random_sequences(int nb_inter)
{
	int ** tab;
	assert(tab = calloc(nb_inter,sizeof(int*)));
	return tab;
}
void free_random_sequences(int** tab,int nb_inter)
{
	if(tab)
	{
		for(int i=0;i<nb_inter;i++)
		{
			if(tab[i])
				free(tab[i]);
		}
		free(tab);
	}

}
int * gives_un(int ** seeds, int inter_size,int inter_id,int seed)
{
	if(seeds[inter_id])return seeds[inter_id];
	
	printf("Graine non trouvée pour %d , calucl ...\n",inter_id);
	assert(seeds[inter_id] = (int *)(malloc(sizeof(int)*inter_size)));
	init(B,seed);
	InitWELLRNG512a(B);
	for(int i=0;i<inter_size;i++)
	{
		seeds[inter_id][i]= WELLRNG512a();
	}
	return seeds[inter_id];


}