#include "constantes.h"


#define SizeDistrib 3*NComponent

int Min[NComponent], Max[NComponent];
double Distrib[SizeDistrib];
double arrival[NComponent], service[NComponent], departure[NComponent];

typedef int*  Etat;
void InitRectangle();
void InitDistribution();
int Inverse(double U);
void Equation(int* OldS,int indexevt);
void F (int * OldS,double U );
void initEtat(Etat e);
int couplage(Etat e1, Etat e2);

