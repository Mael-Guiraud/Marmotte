#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <omp.h>
#include "fsim.h"
#include "alea.h"

int simul_interval(STATE *lb_trajectory, STATE* ub_trajectory, int size, double *randomness){
	int not_coupled = compare(lb_trajectory[0],ub_trajectory[0]); // 0 if coupled, different otherwise
	int i;
	for(i = 1; i < size && not_coupled; i++){ //simulate the two bounds while it is necessary
		ub_trajectory[i] = transition(ub_trajectory[i-1],randomness[i-1]);
		lb_trajectory[i] = transition(lb_trajectory[i-1],randomness[i-1]);
		not_coupled = compare(lb_trajectory[i],ub_trajectory[i]);
	}
	not_coupled = (i==size) ? size : i-1;
	for(; i < size; i++ ){//the simulation has coupled
		ub_trajectory[i] = transition(ub_trajectory[i-1],randomness[i-1]);
	}
	return not_coupled;
}//return the point at which it has coupled, size if not


void balanced_par_sim(STATE *trajectory, double *randomness, int size){
	int interval_number = 4; //use OPENMP to determine that, should always be less than the number of threads  
	STATE *ub_trajectory = trajectory; //the final trajectory is stored here
	STATE *lb_trajectory = malloc(sizeof(STATE)*size);
	//int *state_of_sim = calloc(sizeof(int),interval_number);  //used to know when to simulate a slice, better to 
	//ignore this value
	int interval_size = size/interval_number;
	//initialize the bounds on the first time of each slice
	for(int i = 1; i < interval_number; i++) lb_trajectory[i*interval_size] = min_state();
	lb_trajectory[0] = ub_trajectory[0];
	for(int i = 1; i < interval_number; i++) ub_trajectory[i*interval_size] = max_state(); 
	#pragma omp parallel for schedule(monotonic:dynamic) 
	for(int i = 0; i < interval_number; i++){
		STATE *ub = ub_trajectory + i*interval_size;
		STATE *lb = lb_trajectory + i*interval_size;
	    double *random = randomness + i*interval_size;
	    int number_steps = interval_size;
	    if(i == interval_number -1) number_steps += size%interval_number;//the few additional steps are put one the last interval
	 	while(number_steps > 0){
	    	//while(state_of_sim[i]) {}//lock, should compute only if there are new bounds
	    	//state_of_sim[i] = 1;
	    	//printf("Je suis la thread qui fait l'indice %d et j'utilise comme valeur de départ %d et %d pour simuler %d étapes\n",i,lb[0], ub[0], number_steps);
	    	int update_next_bounds = number_steps == interval_size; // do not update bounds when they are already optimal
	    	number_steps = simul_interval(lb, ub, number_steps, random);
		    if( i != interval_number - 1 && update_next_bounds){
		    //no bound to update in the last interval 
		    	STATE next_ub = transition(ub[interval_size -1],random[interval_size -1]);
		    	if(compare(next_ub, ub[interval_size]) == -1) {
		    		ub[interval_size] = next_ub;
		    		//state_of_sim[i+1] = 0;
		    		//printf("Thread %d update upper bound %d",i,next_ub);
		    	}
		    	if(number_steps != interval_size) { //the lower bound is not updated when there is coupling but is equal to the upper bound
		    		lb[interval_size] = next_ub;
		    		//state_of_sim[i+1] = 0;
		    		//printf("Thread %d update lower bound %d",i,next_ub);
		    	}
		    	else{
		    		STATE next_lb = transition(lb[interval_size -1],random[interval_size -1]);
		    		if(compare(next_lb,lb[interval_size]) == 1){
		    			lb[interval_size] = next_lb;
		    			//state_of_sim[i+1] = 0;
		    			//printf("Thread %d update lower bound %d",i,next_lb);
		    		}
		    	}
		    }
		}
	
	}
}



void par_sim_first_slices(STATE *trajectory, double *randomness, int size){ //give the task in order, so that no processor is idle
 	int interval_number = 4; //use OPENMP to determine that, should always be less than the number of threads  
	STATE *ub_trajectory = trajectory; //the final trajectory is stored here
	STATE *lb_trajectory = malloc(sizeof(STATE)*size);
	int *state_of_sim = calloc(sizeof(int),interval_number);
	//0 means to compute, 1 computing and original bounds, 2 computing but bound updated, 3 not computing but old bounds, 4 finished
	int interval_size = size/interval_number;
	//initialize the bounds on the first time of each slice
	for(int i = 1; i < interval_number; i++) lb_trajectory[i*interval_size] = min_state();
	lb_trajectory[0] = ub_trajectory[0];
	for(int i = 1; i < interval_number; i++) ub_trajectory[i*interval_size] = max_state(); 
	//we use a single lock, so that only one thread can look at potential 
  	// slice at some point and simulate it	
	omp_lock_t myLock;
	(void) omp_init_lock (&myLock);
	//store the size of the interval to simulate, so that the work is not done twice
	int *sizes = malloc(sizeof(int)*interval_number);
	for(int i = 0; i < interval_number; i++) sizes[i] = interval_size;
	sizes[interval_number-1]+= size%interval_number;//the few additional steps are put one the last interval
	#pragma omp parallel
  	{
  		while(1){
  			(void) omp_set_lock(&myLock);//acquire lock
  			int i, states_sum = 0;
  			for( i = 0; i < interval_number && state_of_sim[i]; i++){
  				states_sum += state_of_sim[i];
  			}							 //find slice
  			if(i == interval_number && states_sum == 4*interval_number) {
  				(void) omp_unset_lock (&myLock);//release the lock
  				break;
  			}
  			//we have finished to simulate every slice
  			if(i == interval_number) {
  				(void) omp_unset_lock (&myLock);//release the lock
  				continue;//no free slice wait for the next;
  			}
  			//when we arrive at this point we choose to simulate i
  			state_of_sim[i] = 1;//reserve the slice i
  			(void) omp_unset_lock (&myLock);//release the lock
  			STATE *ub = ub_trajectory + i*interval_size;
			STATE *lb = lb_trajectory + i*interval_size;
	   		double *random = randomness + i*interval_size;
	   		//printf("La thread %d simule la tranche %d sur %d étapes avec comme valeur de départ %d, %d.\n",omp_get_thread_num(),i,sizes[i],lb[0],ub[0]);
	   		int update_next_bounds = sizes[i] == interval_size; // do not update bounds when they are already optimal
  			sizes[i] = simul_interval(lb, ub, sizes[i], random);
  			//update next bounds
  			if( i != interval_number - 1 && update_next_bounds){
		    //no bound to update in the last interval or if they are already good 
		    	STATE next_ub = transition(ub[interval_size -1],random[interval_size -1]);
		    	update_next_bounds = 0; //use to update the state of i+1
		    	if(compare(next_ub, ub[interval_size]) == -1) {
		    		ub[interval_size] = next_ub;
		    		update_next_bounds = 1;
		    		//printf("Thread %d update upper bound %d",i,next_ub);
		    	}
		    	if(sizes[i] != interval_size) { //the lower bound is not updated when there is coupling but is equal to the upper bound
		    		lb[interval_size] = next_ub;
		    		update_next_bounds = 1;
		    		//printf("Thread %d update lower bound %d",i,next_ub);
		    	}
		    	else{
		    		STATE next_lb = transition(lb[interval_size -1],random[interval_size -1]);
		    		if(compare(next_lb,lb[interval_size]) == 1){
		    			lb[interval_size] = next_lb;
		    			update_next_bounds = 1;
		    			//printf("Thread %d update lower bound %d",i,next_lb);
		    		}
		    	}
		    }
		    //all this part could be under lock to make it harmless, concurrency here could be bad but very unlikely
		    if(update_next_bounds) state_of_sim[i+1] = (state_of_sim[i+1] == 1) ? 2 : 0;
		    //check if new bounds has been computed to update the state of i
		    if(state_of_sim[i] == 2) {state_of_sim[i] = 0;} //new bounds havec been computed
		    else{state_of_sim[i] = 3;} //no new bounds have been computed
		    if(sizes[i] == 0) state_of_sim[i] = 4; //detect if we have finished to simulate the slice


		    //printf("La thread %d a simule la tranche %d avec comme valeur de départ %d, %d et a couplé apres %d.\n",omp_get_thread_num(),i,lb[0],ub[0],sizes[i]);
		    //printf("État de la simul");
		    //for (int i = 0; i < interval_number; i++) printf(" %d",state_of_sim[i]);
		    //printf("\n");
  		}
  	}
  	(void) omp_destroy_lock(&myLock);
}


double time_diff(struct timeval tv1, struct timeval tv2)
{
    return (((double)tv2.tv_sec*(double)1000 +(double)tv2.tv_usec/(double)1000) - ((double)tv1.tv_sec*(double)1000 + (double)tv1.tv_usec/(double)1000));
}



int main(){
	srand(time(NULL));
	//lancer un timer
	STATE t = random_state();
	int size = 100000000;
	struct timeval start, end;
	STATE *ub_trajectory = malloc(sizeof(STATE)*size);
	ub_trajectory[0] = t;
	//to simplify, we generate the array of random values beforehand
	double *randomness = malloc(sizeof(double)*size);
	InitWELLRNG512a (time(NULL));
	for(int i = 0; i < size; i++) randomness[i] = WELLRNG512a();		
	printf("Random starting state %d, sequential simulation launched.\n ",t);
	gettimeofday (&start, NULL); 
	simul_interval(ub_trajectory,ub_trajectory,size,randomness); //sequential simulation
	gettimeofday (&end, NULL);
	printf("Temps écoulé pendant la simulation séquentielle %f millisecondes.\n",time_diff(start,end));
	//print_trajectory(ub_trajectory,size);
	//calculer le temps de couplage ici
	printf("Random starting state %d, parallel simulation launched.\n",t);
	gettimeofday (&start, NULL);
	balanced_par_sim(ub_trajectory, randomness, size);
	gettimeofday (&end, NULL);
	printf("Temps écoulé pendant la simulation parallèle sans lock %f millisecondes.\n",time_diff(start,end));
	//print_trajectory(ub_trajectory,size);
	printf("Random starting state %d, parallel simulation launched.\n",t);
	gettimeofday (&start, NULL);
	par_sim_first_slices(ub_trajectory, randomness, size);
	gettimeofday (&end, NULL);
	printf("Temps écoulé pendant la simulation parallèle avec locks %f millisecondes.\n",time_diff(start,end));
}