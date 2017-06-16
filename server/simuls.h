void init_simul(int nb_q, int min, int max,double load, double p, double mu);
int coupling(int* s1, int* s2);
void F (int * OldS,double U );
void cpy_state(int* s1, int* s2);
void simulation_mem_free();
