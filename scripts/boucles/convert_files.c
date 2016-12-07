#include <stdio.h>

int main()
{
	int i,j;
	FILE * F, *F2 ;
	char nom[20];
	char nom2[20];
	int a,b;
	float
	 etapes,temps;
	sprintf(nom2,"results_moyennes");
	F2 = fopen(nom2,"w");
	for(i=10;i<44;i+=2)
	{
		etapes  = 0.0;
		temps = 0.0;
		sprintf(nom,"result%d",i);
		
		F = fopen(nom,"r");
		
		for(j=0;j<1000;j++)
		{
			fscanf(F,"%d",&a);
			fscanf(F,"%d",&b);
			etapes += (float)a;
			temps += (float)b;
			//printf("%d %d %f %f\n",i,j,etapes,temps);
			
		}
		fprintf(F2,"%d %f %f\n",100000/i,etapes/(float)1000,temps/(float)1000);
		fclose(F);

	}
	fclose(F2);
}
