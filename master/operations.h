
#include "struct.h"

//Return 1 if s1 and s2 are the same, 0 otherwise
int coupling(int* s1, int* s2, int nb_queues);
void cpy_state(int * s1, int* s2, int nb_queues);
void free_bounds(Bounds *bounds, int nb_inter);
Bounds *initBounds(int nb_interval, int min, int max, int nb_queues);
int sniffer_interval(Interval_state * interval_state,int nb_inter, int begin);
int better(int *s1,int*s2,int*s3,int*s4, int nb_queues); 
void initDpeartureBounds(int *borne_min, int *borne_max, int max, int nb_queues);
void affiche_bounds(Bounds* bounds, int nb_interval, int nb_queues);
void floatToint(float f, int * tab);
void floatTointLoad(float f, int * tab);
int snifer_machine(Message_kind * what_do_i_read, int nb_machines);
int updated(int * bound, int nb_queues);
int all_finished(Message_kind * what_do_i_read, int nb_machines);