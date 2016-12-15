#include <stdio.h>

void cherche_max(FILE * f,double * max,int* max_inter)
{
	char c;
	int space = 0;
	double read;

	while((c=fgetc(f))!=EOF)
		{
			if(c== ' ' && !space)//On va lire un double
			{
				fscanf(f,"%lf",&read);
				if(read > *max)*max=read;
				space = 1;
			}
			if(c=='\n')
			{	
				fscanf(f,"%d",max_inter);
				space = 0;
			}
		}
}
int max(int a, int b){return (a>b)?a:b;}
int min(int a, int b){return (a>b)?b:a;}


int main(int argc, char* argv[])
{
	if(argc != 3){perror("Must have 2 files as args\n");return(1);}


	FILE * fichier_distrib = fopen(argv[2],"r");


	double max_distrib = 0.0;
	int min_inter_distrib; fscanf(fichier_distrib,"%d",&min_inter_distrib);
	int max_inter_distrib;
	cherche_max(fichier_distrib,&max_distrib,&max_inter_distrib);

	FILE * fichier_simuls = fopen(argv[1],"r");
	double max_simul = 0.0;
	int min_inter_simul; fscanf(fichier_distrib,"%d",&min_inter_simul);
	int max_inter_simul;
	cherche_max(fichier_simuls,&max_simul,&max_inter_simul);

	FILE * gplt = fopen("temps.gplt","w");
	fprintf(gplt,"set grid\nset xrange [%d:%d]\nset ylabel \"Rounds\"\nset yrange [0:%lf]\nplot '%s' using 1:2 notitle\nset y2label \"Occurences(%%)\";  set y2tics\nset y2range [0:%f] \nreplot '%s' using 1:2 axes x1y2 smooth bezier notitle \nset term postscript color solid\nset output \"temps.ps\"\nreplot\n",min(min_inter_distrib,min_inter_simul),max(max_inter_simul,max_inter_distrib),max_simul,argv[1],max_distrib,argv[2]);




	fclose(gplt);
	fclose(fichier_distrib);
	fclose(fichier_simuls);


	return 0;
}

