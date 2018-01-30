#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <omp.h>
#include "fsim.h"
#include "alea.h"


//simulate a lower bound and an upper bound on the random process. When both are equal, it is the process itself 
//and we store it in the upper bound only

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


int update_next_bounds(int interval_size, int number_steps, STATE *ub, STATE* lb, double *random){
	int updated = 0;
	STATE next_ub = transition(ub[interval_size -1],random[interval_size -1]);
	if(compare(next_ub, ub[interval_size]) == -1) {
		ub[interval_size] = next_ub;
		updated = 1;
	}
	if(number_steps != interval_size) { //the lower bound is not updated when there is coupling but is equal to the upper bound
		lb[interval_size] = next_ub;
		updated = 1;
	}
	else{
		STATE next_lb = transition(lb[interval_size -1],random[interval_size -1]);
		if(compare(next_lb,lb[interval_size]) == 1){
			lb[interval_size] = next_lb;
			updated = 1;
		}
	}
	return updated;
}


void par_sim_fixed_slices(STATE *trajectory, double *randomness, int size){ //the initial state is given in ub_trajectory[0]
	int interval_number = omp_get_max_threads(); //use the number of cores but can be modified 
	int interval_size = size/interval_number; 

	STATE *ub_trajectory = trajectory; //the final trajectory is stored here
	STATE *lb_trajectory = malloc(sizeof(STATE)*size);
	
	//initialize the bounds on the first time of each slice
	for(int i = 1; i < interval_number; i++) lb_trajectory[i*interval_size] = min_state();
	lb_trajectory[0] = ub_trajectory[0];//the initial state is given in ub_trajectory[0]
	for(int i = 1; i < interval_number; i++) ub_trajectory[i*interval_size] = max_state(); 
	
	#pragma omp parallel for schedule(monotonic:dynamic) 
	for(int i = 0; i < interval_number; i++){
		STATE *ub = ub_trajectory + i*interval_size;
		STATE *lb = lb_trajectory + i*interval_size;
	    double *random = randomness + i*interval_size;
	    int number_steps = interval_size;
	    if(i == interval_number -1) number_steps += size%interval_number;//the few additional steps are put one the last interval
	 	while(number_steps > 0){
	    	//on pourrait mettre un test des premières valeurs ici pour attendre l'update
	    	//printf("Je suis la thread %d qui fait l'indice %d et j'utilise comme valeur de départ %d et %d pour simuler %d étapes\n",omp_get_thread_num(),i,lb[0], ub[0], number_steps);
	    	int not_coupled = (number_steps == interval_size);
	    	number_steps = simul_interval(lb, ub, number_steps, random);
	    	 // do not update the next bounds when they are already optimal or when we compute the last slice
		    if( i != interval_number - 1 && not_coupled) update_next_bounds(interval_size, number_steps, ub, lb, random);
		}
	}
	free(lb_trajectory);
}

typedef enum{ IDLE_NEW_BOUNDS = 0, COMP_OLD_BOUNDS = 1, COMP_NEW_BOUNDS = 2, IDLE_OLD_BOUNDS = 3, DONE = 4}
SIM_STATE;
	//state of a slice during the simulation
	//0 means to compute, 1 computing and original bounds, 2 computing but bounds updated, 3 computing finished and old bounds, 4 finished



void par_sim_first_slices(STATE *trajectory, double *randomness, int size){ //give the task in order, so that no processor is idle
 	int interval_number = omp_get_max_threads()*2;//use a function of the number of cores, should be larger 
 	int interval_size = size/interval_number; 
	STATE *ub_trajectory = trajectory; //the final trajectory is stored here
	STATE *lb_trajectory = malloc(sizeof(STATE)*size);
	SIM_STATE *state_of_sim = calloc(sizeof(SIM_STATE),interval_number);	
	//initialize the bounds on the first time of each slice
	for(int i = 1; i < interval_number; i++) lb_trajectory[i*interval_size] = min_state();
	lb_trajectory[0] = ub_trajectory[0];
	for(int i = 1; i < interval_number; i++) ub_trajectory[i*interval_size] = max_state(); 
	
	//we use a  lock to keep the states of the sim coherent between threads	
	omp_lock_t sim_state_lock;
	(void) omp_init_lock (&sim_state_lock);
	//store the size of the interval to simulate, so that the work is not done twice
	int *sizes = malloc(sizeof(int)*interval_number);
	for(int i = 0; i < interval_number; i++) sizes[i] = interval_size;
	sizes[interval_number-1]+= size%interval_number;//the few additional steps are put one the last interval
	#pragma omp parallel
  	{
  		while(1){
  			(void) omp_set_lock(&sim_state_lock);//acquire lock
  			int i, states_sum = 0;
  			for( i = 0; i < interval_number && state_of_sim[i]; i++){//look for a free slice
  				states_sum += state_of_sim[i];
  			}							 
  			if(i == interval_number){
  				(void) omp_unset_lock (&sim_state_lock);//release the lock
  				if(states_sum == DONE*interval_number) break;//we have finished to simulate every slice
   				continue;//no slice with new bounds avalaible, wait for the next
  			}
  			//when we arrive at this point we choose to simulate i
  			state_of_sim[i] = COMP_OLD_BOUNDS;
  			(void) omp_unset_lock (&sim_state_lock);//release the lock
  			STATE *ub = ub_trajectory + i*interval_size;
			STATE *lb = lb_trajectory + i*interval_size;
	   		double *random = randomness + i*interval_size;
	   		//printf("La thread %d simule la tranche %d sur %d étapes avec comme valeur de départ %d, %d.\n",omp_get_thread_num(),i,sizes[i],lb[0],ub[0]);
	   		int not_coupled = (sizes[i] == interval_size); // do not update bounds when they are already optimal
  			sizes[i] = simul_interval(lb, ub, sizes[i], random);
  			//update next bounds, can be done without lock
  			int update = 0;
  			if( i != interval_number - 1 && not_coupled) update = update_next_bounds(interval_size, sizes[i], ub, lb, random);
		    //all this part is under lock to make it harmless, but a concurrent access is unlikely 
		    (void) omp_set_lock (&sim_state_lock);//acquire the lock
		    if(update) state_of_sim[i+1] = (state_of_sim[i+1] == COMP_OLD_BOUNDS) ? COMP_NEW_BOUNDS : IDLE_NEW_BOUNDS;
		    //check if new bounds has been computed to update the state of i
		    state_of_sim[i] = (state_of_sim[i] == COMP_NEW_BOUNDS) ? IDLE_NEW_BOUNDS : IDLE_OLD_BOUNDS;
		    if(sizes[i] == 0) state_of_sim[i] = DONE; //detect if we have finished to simulate the slice
		    (void) omp_unset_lock (&sim_state_lock);//release the lock
		    //printf("La thread %d a simule la tranche %d avec comme valeur de départ %d, %d et a couplé apres %d.\n",omp_get_thread_num(),i,lb[0],ub[0],sizes[i]);
		    //printf("État de la simul");
		    //for (int i = 0; i < interval_number; i++) printf(" %d",state_of_sim[i]);
		    //printf("\n");
  		}
  	}
  	(void) omp_destroy_lock(&sim_state_lock);
  	free(lb_trajectory);
  	free(state_of_sim);
}


void par_sim_balanced_slices(STATE *trajectory, double *randomness, int size){
	int meta_interval_size = 4;
	int meta_interval_number = omp_get_max_threads(); 
	int interval_number = meta_interval_number*meta_interval_size;
 	int current_meta_interval = 0;

 	int interval_size = size/interval_number; 
	STATE *ub_trajectory = trajectory; //the final trajectory is stored here
	STATE *lb_trajectory = malloc(sizeof(STATE)*size);
	SIM_STATE *state_of_sim = calloc(sizeof(SIM_STATE),interval_number);	
	//initialize the bounds on the first time of each slice
	for(int i = 1; i < interval_number; i++) lb_trajectory[i*interval_size] = min_state();
	lb_trajectory[0] = ub_trajectory[0];
	for(int i = 1; i < interval_number; i++) ub_trajectory[i*interval_size] = max_state(); 
	
	//we use a  lock to keep the states of the sim coherent between threads	
	omp_lock_t sim_state_lock;
	(void) omp_init_lock (&sim_state_lock);
	//store the size of the interval to simulate, so that the work is not done twice
	int *sizes = malloc(sizeof(int)*interval_number);
	for(int i = 0; i < interval_number; i++) sizes[i] = interval_size;
	sizes[interval_number-1]+= size%interval_number;//the few additional steps are put one the last interval
	#pragma omp parallel
  	{
  		while(1){
  			(void) omp_set_lock(&sim_state_lock);//acquire lock
  			int i, states_sum = 0;
  			for(i = current_meta_interval*meta_interval_size; i < interval_number && state_of_sim[i]; i++){//look for a free slice
  				states_sum += state_of_sim[i];
  			}	
  			if(i == interval_number){
  				for(i = 0; i <  current_meta_interval*meta_interval_size && state_of_sim[i]; i++){//look for a free slice
  					states_sum += state_of_sim[i];
  				}
  				if(i == current_meta_interval*meta_interval_size){
  					(void) omp_unset_lock (&sim_state_lock);//release the lock
  					if(states_sum == DONE*interval_number) break;//we have finished to simulate every slice
   					continue;//no slice with new bounds avalaible, wait for the next
  				}
  			}							 
  			current_meta_interval = (i/meta_interval_size) + 1;//we give the priority to the meta interval after the one we just used
  			if (current_meta_interval == meta_interval_number) current_meta_interval = 0;//change the meta interval when we have scheduled some work
  			//when we arrive at this point we choose to simulate i
  			state_of_sim[i] = COMP_OLD_BOUNDS;
  			(void) omp_unset_lock (&sim_state_lock);//release the lock
  			STATE *ub = ub_trajectory + i*interval_size;
			STATE *lb = lb_trajectory + i*interval_size;
	   		double *random = randomness + i*interval_size;
	   		//printf("La thread %d simule la tranche %d sur %d étapes avec comme valeur de départ %d, %d.\n",omp_get_thread_num(),i,sizes[i],lb[0],ub[0]);
	   		int not_coupled = (sizes[i] == interval_size); // do not update bounds when they are already optimal
  			sizes[i] = simul_interval(lb, ub, sizes[i], random);
  			//update next bounds, can be done without lock
  			int update = 0;
  			if( i != interval_number - 1 && not_coupled) update = update_next_bounds(interval_size, sizes[i], ub, lb, random);
		    //all this part is under lock to make it harmless, but a concurrent access is unlikely 
		    (void) omp_set_lock (&sim_state_lock);//acquire the lock
		    if(update) state_of_sim[i+1] = (state_of_sim[i+1] == COMP_OLD_BOUNDS) ? COMP_NEW_BOUNDS : IDLE_NEW_BOUNDS;
		    //check if new bounds has been computed to update the state of i
		    state_of_sim[i] = (state_of_sim[i] == COMP_NEW_BOUNDS) ? IDLE_NEW_BOUNDS : IDLE_OLD_BOUNDS;
		    if(sizes[i] == 0) state_of_sim[i] = DONE; //detect if we have finished to simulate the slice
		    (void) omp_unset_lock (&sim_state_lock);//release the lock
		    //printf("La thread %d a simule la tranche %d avec comme valeur de départ %d, %d et a couplé apres %d.\n",omp_get_thread_num(),i,lb[0],ub[0],sizes[i]);
		    //printf("État de la simul");
		    //for (int i = 0; i < interval_number; i++) printf(" %d",state_of_sim[i]);
		    //printf("\n");
  		}
  	}
  	(void) omp_destroy_lock(&sim_state_lock);
  	free(lb_trajectory);
  	free(state_of_sim);



}

double time_diff(struct timeval tv1, struct timeval tv2)
{
    return (((double)tv2.tv_sec*(double)1000 +(double)tv2.tv_usec/(double)1000) - ((double)tv1.tv_sec*(double)1000 + (double)tv1.tv_usec/(double)1000));
}



int main(){
	srand(time(NULL));
	//lancer un timer
	
	int size = 10000000;
	struct timeval start, end;
	STATE *ub_trajectory = malloc(sizeof(STATE)*size);
	
	double stat[4] = {0,0,0,0};
	//to simplify, we generate the array of random values beforehand
	double *randomness = malloc(sizeof(double)*size);
	for(int i = 0; i < 10; i++){
		STATE t = random_state();
		ub_trajectory[0] = t;
		InitWELLRNG512a (time(NULL));
		for(int i = 0; i < size; i++) randomness[i] = WELLRNG512a();
				
		printf("Random starting state %d, sequential simulation launched.\n ",t);
		gettimeofday (&start, NULL); 
		simul_interval(ub_trajectory,ub_trajectory,size,randomness); //sequential simulation
		gettimeofday (&end, NULL);
		printf("Temps écoulé pendant la simulation séquentielle %f millisecondes.\n",time_diff(start,end));
		stat[0]+=time_diff(start,end); 
		//print_trajectory(ub_trajectory,size);
		
		printf("Random starting state %d, parallel simulation launched.\n",t);
		gettimeofday (&start, NULL);
		par_sim_fixed_slices(ub_trajectory, randomness, size);
		gettimeofday (&end, NULL);
		printf("Temps écoulé pendant la simulation parallèle sans lock %f millisecondes.\n",time_diff(start,end));
		stat[1]+=time_diff(start,end);
		//print_trajectory(ub_trajectory,size);
		printf("Random starting state %d, parallel simulation launched.\n",t);
		gettimeofday (&start, NULL);
		par_sim_first_slices(ub_trajectory, randomness, size);
		gettimeofday (&end, NULL);
		printf("Temps écoulé pendant la simulation parallèle avec locks %f millisecondes.\n",time_diff(start,end));
		stat[2]+=time_diff(start,end);

		printf("Random starting state %d, parallel simulation launched.\n",t);
		gettimeofday (&start, NULL);
		par_sim_balanced_slices(ub_trajectory, randomness, size);
		gettimeofday (&end, NULL);
		printf("Temps écoulé pendant la simulation parallèle avec locks et équilibrée %f millisecondes.\n",time_diff(start,end));
		stat[3]+=time_diff(start,end);
	}
	printf("Temps moyen : Algo sequentiel %f \n Algo sans lock : %f \n Algo avec lock intervalles dans l'ordre %f \n Algo avec locks intervalles répartis %f",stat[0]/10,stat[1]/10,stat[2]/10,stat[3]/10);

	/************************Free the memory *************************/

	free(ub_trajectory);
	free(randomness);
}