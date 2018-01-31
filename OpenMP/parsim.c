#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <omp.h>
#include <math.h>
#include "fsim.h"
#include "alea.h"


//simulate a lower bound and an upper bound on the random process. When both are equal, it is the process itself 
//and we store it in the upper bound only

int simul_interval(STATE *lb_trajectory, STATE* ub_trajectory, int size, double *randomness){
	int coupling_point = 0, i = 1;
	if(compare(lb_trajectory[0],ub_trajectory[0])){//the first elements are coupled
		for(; i < size && compare(lb_trajectory[i-1],ub_trajectory[i-1]); i++){ //simulate the two bounds while it is necessary
			ub_trajectory[i] = transition(ub_trajectory[i-1],randomness[i-1]);
			lb_trajectory[i] = transition(lb_trajectory[i-1],randomness[i-1]);
		}
		coupling_point = size;// value when no couplingno coupling
		if(!compare(lb_trajectory[i-1],ub_trajectory[i-1]))//we detect that we went out of the loops because of coupling
			coupling_point = i-1;	
	}
	for(i++; i < size; i++ ){//the simulation has coupled, simulate only the upper bound
		ub_trajectory[i] = transition(ub_trajectory[i-1],randomness[i-1]);
	}
	return coupling_point;
}//return the point at which it has coupled, size if not


int update_next_bounds(int interval_size, int number_steps, STATE *lb, STATE* ub, double *random){
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
		    if( i != interval_number - 1 && not_coupled) update_next_bounds(interval_size, number_steps, lb, ub, random);
		}
	}
	free(lb_trajectory);
}

typedef enum{ IDLE_NEW_BOUNDS = 0, IDLE_OLD_BOUNDS = 1, COMP_NEW_BOUNDS = 2, COMP_OLD_BOUNDS = 3, DONE = 4}
SIM_STATE;
	//state of a slice during the simulation
	//0 means to compute, 1 computing and original bounds, 2 computing but bounds updated, 3 computing finished and old bounds, 4 finished



void par_sim_first_slices(STATE *trajectory, double *randomness, int size){ //give the task in order, so that no processor is idle
 	int interval_number = omp_get_max_threads();//use a function of the number of cores, should be larger 
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
  			int i, states_sum = 0;
  			(void) omp_set_lock(&sim_state_lock);//acquire lock
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
  			if( i != interval_number - 1 && not_coupled) update = update_next_bounds(interval_size, sizes[i], lb, ub, random);
		    //all this part is under lock to make it harmless, but a concurrent access is unlikely 
		    (void) omp_set_lock (&sim_state_lock);//acquire the lock
		    if(update && (state_of_sim[i+1] % 2)) state_of_sim[i+1]--;
		    //check if new bounds has been computed to update the state of i
		    state_of_sim[i] -= 2;
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
  			int i, states_sum = 0;
  			(void) omp_set_lock(&sim_state_lock);//acquire lock
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
  			//when we arrive at this point we choose to simulate i
  			state_of_sim[i] = COMP_OLD_BOUNDS;
  			(void) omp_unset_lock (&sim_state_lock);//release the lock
  			current_meta_interval = (i/meta_interval_size) + 1;//we give the priority to the meta interval after the one we just used
  			if (current_meta_interval == meta_interval_number) current_meta_interval = 0;//change the meta interval when we have scheduled some work
  			STATE *ub = ub_trajectory + i*interval_size;
			STATE *lb = lb_trajectory + i*interval_size;
	   		double *random = randomness + i*interval_size;
	   		//printf("La thread %d simule la tranche %d sur %d étapes avec comme valeur de départ %d, %d.\n",omp_get_thread_num(),i,sizes[i],lb[0],ub[0]);
	   		int not_coupled = (sizes[i] == interval_size); // do not update bounds when they are already optimal
  			sizes[i] = simul_interval(lb, ub, sizes[i], random);
  			//update next bounds, can be done without lock
  			int update = 0;
  			if( i != interval_number - 1 && not_coupled) update = update_next_bounds(interval_size, sizes[i], lb, ub, random);
		    //all this part is under lock to make it harmless, but a concurrent access is unlikely 
		    (void) omp_set_lock (&sim_state_lock);//acquire the lock
		    if(update && (state_of_sim[i+1] % 2)) state_of_sim[i+1]--;
		    //check if new bounds has been computed to update the state of i
		    state_of_sim[i] -= 2;
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
	int size = 10000000;//length of the simulation
	int experiment_number = 100;
	struct timeval start, end;
	STATE *ub_trajectory = malloc(sizeof(STATE)*size);
	double **results = malloc(sizeof(double *)*4);
	for(int i = 0; i < 4; i++) results[i] = malloc(sizeof(double)*experiment_number);
	double *randomness = malloc(sizeof(double)*size);
	
	/*
	//Evaluation of the coupling time
	int coupling_time = 0;
	unsigned int *var_coupling_time = malloc(sizeof(unsigned int)*100);
	STATE *lb_trajectory = malloc(sizeof(STATE)*size);
	ub_trajectory[0] = max_state();
	lb_trajectory[0] = min_state();
	for(int i = 0; i < 100; i++){
		for(int i = 0; i < size; i++) randomness[i] = WELLRNG512a();
		var_coupling_time[i] = simul_interval(lb_trajectory,ub_trajectory,size,randomness); 
		coupling_time += var_coupling_time[i];
	}
	coupling_time /= 100;
	long unsigned int variance = 0;
	for(int i = 0; i < 100; i++) variance += (coupling_time-var_coupling_time[i])*(coupling_time-var_coupling_time[i]); 
	printf("Average coupling time of %d, variance of %d.\n",coupling_time, (int) sqrt(variance/99));
	*/
	// evaluation of the different methods of simulation
	for(int i = 0; i < experiment_number; i++){
		//init the instance of the simulation
		srand(time(NULL));
		InitWELLRNG512a(time(NULL));//init the two RNG generators, using a mostly random time
		ub_trajectory[0] =  random_state();
		for(int i = 0; i < size; i++) randomness[i] = WELLRNG512a();			
		//to simplify the algorithm and the performancem easure, we generate the array of random values before the simulation

		/**************** Sequential simulation ************************/
		//printf("Random starting state %d, sequential simulation launched.\n ",ub_trajectory[0]);
		gettimeofday (&start, NULL); 
		simul_interval(ub_trajectory,ub_trajectory,size,randomness); //sequential simulation
		gettimeofday (&end, NULL);
		//printf("Temps écoulé pendant la simulation séquentielle %f millisecondes.\n",time_diff(start,end));
		results[0][i] = time_diff(start,end); 
		//print_trajectory(ub_trajectory,size);
		
		/**************** Parallel simulation ************************/
		//printf("Random starting state %d, parallel simulation launched.\n\n\n",ub_trajectory[0]);
		gettimeofday (&start, NULL);
		par_sim_fixed_slices(ub_trajectory, randomness, size);
		gettimeofday (&end, NULL);
		//printf("Temps écoulé pendant la simulation parallèle sans lock %f millisecondes.\n",time_diff(start,end));
		results[1][i] = time_diff(start,end);
		//print_trajectory(ub_trajectory,size);
		
		//printf("Random starting state %d, parallel simulation launched.\n",ub_trajectory[0]);
		gettimeofday (&start, NULL);
		par_sim_first_slices(ub_trajectory, randomness, size);
		gettimeofday (&end, NULL);
		//printf("Temps écoulé pendant la simulation parallèle avec locks %f millisecondes.\n",time_diff(start,end));
		results[2][i] = time_diff(start,end);
		//print_trajectory(ub_trajectory,size);

		//printf("Random starting state %d, parallel simulation launched.\n",ub_trajectory[0]);
		gettimeofday (&start, NULL);
		par_sim_balanced_slices(ub_trajectory, randomness, size);
		gettimeofday (&end, NULL);
		//printf("Temps écoulé pendant la simulation parallèle avec locks et équilibrée %f millisecondes.\n",time_diff(start,end));
		results[3][i] = time_diff(start,end);
		//print_trajectory(ub_trajectory,size);
	}
	
	/************************* Compute and print the statistics ********************************/

	double mean[4] = {0,0,0,0};
	double variance[4] = {0,0,0,0};
	for(int i=0; i<experiment_number; i++){
		for(int j = 0; j < 4; j++){
			mean[j] += results[j][i];
		}
	}
	for(int j = 0; j < 4; j++) mean[j]/=experiment_number;
	for(int i=0; i<experiment_number; i++){
		for(int j = 0; j < 4; j++){
			variance[j] += (mean[j] -results[j][i])*(mean[j] -results[j][i]);
		}
	}
	for(int j = 0; j < 4; j++) variance[j] = sqrt(variance[j]/(experiment_number-1));
	printf("Temps moyen, écart type et accélération parallèle \n");
	printf("Algo séquentiel: %f, %f   \n", mean[0], variance[0]);
	printf("Algo parallèle sans lock %f, %f, %f \n", mean[1], variance[1],mean[0]/mean[1]);
	printf("Algo parallèle avec lock premières tranches en premier %f, %f, %f \n", mean[2], variance[2],mean[0]/mean[2]);
	printf("Algo parallèle avec lock tranches réparties %f, %f, %f \n", mean[3], variance[3],mean[0]/mean[3]);
	

	/************************Free the memory *************************/
	
	free(ub_trajectory);
	free(randomness);
}