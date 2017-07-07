#include <stdio.h>

int coupling(int* s1, int* s2);
void cpy_state(int* s1, int* s2);
void initStateMIN(int* s);
void initStateMAX(int* s);
void initState(int* s);
void initStateRand(int* s);
void write_result_file(FILE * f, int inter_size, double rounds, double intervals, double time);
int sniffer_interval(int nb_inter);
int better(int *s1,int*s2,int*s3,int*s4);
struct Bounds *initBounds(int nb_interval, int min, int max);
void free_bounds(struct Bounds *bounds, int nb_inter);
void initDpeartureBounds(int *borne_min, int *borne_max, int max);
void affiche_bounds(struct Bounds* bounds, int nb_interval);
void floatToint(float f, int * tab);
void floatTointLoad(float f, int * tab);