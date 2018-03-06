typedef int STATE;


void print_trajectory(STATE *traj, int size);

STATE transition(STATE t, double random);//return the next state using random

int compare(STATE t1, STATE t2); //return -1 if t1 < t2, 0 if t1=t2 and 1 otherwise

STATE random_state();

STATE min_state();

STATE max_state();