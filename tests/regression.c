#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
#define nb_points 10


float x[nb_points];
float y[nb_points];


void lire_fichier()
{
	FILE * f = fopen("../results/nb_queues.data","r");

	float nombre_lu;
	int colonne=0;
	int ligne = 0;
	while(fscanf(f,"%f",&nombre_lu))
	{
		printf("%f\n",nombre_lu);
		if(colonne == 0)
			x[ligne] = (float)nombre_lu;

		else
		{
			y[ligne] = (float)nombre_lu;
		}
		colonne++;

		if(colonne == 2)
		{
			colonne = 0;
			ligne ++;
			if(ligne == nb_points)
				break;
		}
	}
	fclose(f);

}


void affiche_tab(float *tab)
{
	printf("[");
	for(int i=0;i<nb_points;i++)
	{
		if(i < nb_points-1)
			printf("%f;",tab[i]);
		else
			printf("%f]",tab[i]);
	}
	printf("\n");
}

float moyenne(float * tab)
{
	float moy=0;
	for(int i =0;i<nb_points;i++)
	{
		moy += tab[i];
	}
	return moy/nb_points;
}
float variance(float * tab)
{
	float var = 0;
	for(int i=0;i<nb_points;i++)
	{
		var += tab[i]*tab[i];
	}
	var /= nb_points;
	return (float)(var - moyenne(tab)*moyenne(tab));

}
float covariance()
{
	float co_var = 0;
	for(int i=0;i<nb_points;i++)
	{
		co_var += x[i]*y[i];
	}
	co_var /= nb_points;
	return (float)(co_var - moyenne(x)*moyenne(y));

}
float coef_dir()
{
	return covariance()/variance(x);
}

float ordonee_origine()
{
	return moyenne(y)-coef_dir()*moyenne(x);
}

float coefficient_correlation()
{
	return covariance()/(sqrt(variance(x)*variance(y)));
}
int main()
{

		lire_fichier();
		printf("L'équation de la droite est : y = %f x + %f.\n Le coefficient de corélation est : %f.\n Moyenne = %f.\n\n",coef_dir(),ordonee_origine(),coefficient_correlation(),moyenne(y));

	
}