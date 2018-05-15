#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <omp.h>
#include <math.h>
#include "fsim.h"
#include "alea.h"


//Options 

#define COUPLING_TIME 1

//Hack to test the local memory

STATE **lb_local;
STATE **ub_local;
double **random_local;

//simulate a lower bound and an upper bound on the random process. When both are equal, it is the process itself 
//and we store it in the upper bound only

int simul_interval(STATE *lb_trajectory, STATE* ub_trajectory, int size, double *randomness){
	int coupling_point = 0, i = 1;
	if(compare(lb_trajectory[0],ub_trajectory[0])){//the first elements are coupled
		for(; i < size && compare(lb_trajectory[i-1],ub_trajectory[i-1]); i++){ //simulate the two bounds while it is necessary
			ub_trajectory[i] = transition(ub_trajectory[i-1],randomness[i-1]);
			lb_trajectory[i] = transition(lb_trajectory[i-1],randomness[i-1]);
		}
		coupling_point = size;// value when no coupling
		if(!compare(lb_trajectory[i-1],ub_trajectory[i-1]))//we detect that we went out of the loops because of coupling
			coupling_point = i-1;	
	}
	for(i++; i < size; i++ ){//the simulation has coupled, simulate only the upper bound
		ub_trajectory[i] = transition(ub_trajectory[i-1],randomness[i-1]);
	}
	return coupling_point;
}//return the point at which it has coupled, size if not

void sequential_sim(STATE* lb_trajectory, STATE* ub_trajectory, double *randomness, int size, int interval_number){
	simul_interval(ub_trajectory, ub_trajectory, size, randomness);
}


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

void init_interval(int interval_size, int interval_number, STATE *lb_trajectory, STATE *ub_trajectory){
	//initialize the bounds on the first time of each slice
	for(int i = 1; i < interval_number; i++){
		lb_trajectory[i*interval_size] = min_state();	
		ub_trajectory[i*interval_size] = max_state(); 
	} 
}


//one thread by interval, no lock. 
// The code to wait when no new bounds are given is commented since it slows the code 
void par_sim_fixed_slices(STATE *lb_trajectory, STATE *ub_trajectory, double *randomness, int size, int interval_number){ 
	int interval_size = size/interval_number; 
	init_interval(interval_size,interval_number,lb_trajectory,ub_trajectory);	
	
	#pragma omp parallel for schedule(monotonic:dynamic) 
	for(int i = 0; i < interval_number; i++){
		STATE *ub = ub_trajectory + i*interval_size;
		STATE *lb = lb_trajectory + i*interval_size;
	    double *random = randomness + i*interval_size;
	    int coupling_point = interval_size;
	    //STATE old_lb = lb[0], old_ub = ub[0];
	    if(i == interval_number -1) coupling_point += size%interval_number;//the few additional steps are put one the last interval
	 	while(coupling_point > 0){
    	//printf("Je suis la thread %d qui fait l'indice %d et j'utilise comme valeur de départ %d et %d pour simuler %d étapes\n",omp_get_thread_num(),i,lb[0], ub[0], number_steps);
  			//old_lb = lb[0];
    		//old_ub = ub[0];
	    	int not_coupled = (coupling_point == interval_size);
	    	coupling_point = simul_interval(lb, ub, coupling_point, random);
	    	 // do not update the next bounds when they are already optimal or when we compute the last slice
		    if( i != interval_number - 1 && not_coupled) update_next_bounds(interval_size, coupling_point, lb, ub, random);
			//while(!compare(old_lb, lb[0]) && !compare(old_ub, ub[0]) && coupling_point > 0) {}//wait for new bounds
		}
	}
}
//one thread by interval, no lock. The memory is allocated as locally as possible

void par_sim_fixed_slices_local(STATE *lb_interval, STATE *ub_interval, double *randomness, int size, int interval_number){ 
	int interval_size = size/interval_number; 
	for(int i = 1; i < interval_number; i++){
		lb_interval[i] = min_state();	
		ub_interval[i] = max_state(); 
	}
	//the array containing the beginning state of each interval is allocated and initialized
	#pragma omp parallel for schedule(monotonic:dynamic) 
	for(int i = 0; i < interval_number; i++){
		int coupling_point = interval_size;
		int coupled = 0;
		double *random_l = random_local[i];
		STATE *lb_l = lb_local[i];
		STATE *ub_l = ub_local[i];
		if(i == interval_number -1) coupling_point += size%interval_number;//the few additional steps are put one the last interval
		memcpy(random_l, randomness + i*interval_size, coupling_point*sizeof(double));
	    
	    //STATE old_lb = lb[0], old_ub = ub[0];
	    while(coupling_point > 0){
    	//printf("Je suis la thread %d qui fait l'indice %d et j'utilise comme valeur de départ %d et %d pour simuler %d étapes\n",omp_get_thread_num(),i,lb[0], ub[0], number_steps);
  			//old_lb = lb[0];
    		//old_ub = ub[0];
	    	lb_l[0] = lb_interval[i];
	    	ub_l[0] = ub_interval[i];
	    	coupling_point = simul_interval(lb_l, ub_l, coupling_point, random_l);
		    if(!coupled && i != interval_number - 1 ){
		    	STATE next_ub = transition(ub_l[interval_size -1],random_l[interval_size -1]);
		    	if(compare(next_ub, ub_interval[i+1]) == -1) ub_interval[i+1] = next_ub;
		    	if(coupling_point < interval_size + size%interval_number){
		    		coupled = 1;
		    		lb_interval[i+1] = next_ub;
		    	}
		    	else{
		    		STATE next_lb = transition(lb_l[interval_size -1],random_l[interval_size -1]);			    	
			    	if(compare(next_lb, lb_interval[i+1]) == 1) lb_interval[i+1] = next_lb;
		    	}
		    }
		}
	}
}


//allowing processors to compute interval which are already being computed degrade the performance, because
//we can couple several time, which is wasteful. However the code is simpler when we do not deal with the array computing
//we select always the first avalaible non computing interval
void par_sim_first_slices_first(STATE *lb_trajectory, STATE *ub_trajectory, double *randomness, int size, int interval_number){ //use only two binary states by slice 
 	int interval_size = size/interval_number; 
	int *available_interval = malloc(sizeof(int)*interval_number);//boolean to store if an interval has new bounds	
	int *computing = malloc(sizeof(int)*interval_number);//boolean to store if a thread is computing an interval
	int *sizes = malloc(sizeof(int)*interval_number);//store the size of what is left to simulate for each interval
	init_interval(interval_size,interval_number,lb_trajectory,ub_trajectory);	

	//we use the following lock to keep the states of the sim coherent between threads	
	omp_lock_t sim_state_lock;
	(void) omp_init_lock (&sim_state_lock);
	for(int i = 0; i < interval_number; i++) {//initialization of the states of each interval
		sizes[i] = interval_size;
		available_interval[i] = 1;
		computing[i]= 0;
	}
	sizes[interval_number-1]+= size%interval_number;//the few additional steps are put one the last interval
	int interval_done = 0;//number of interval correctly simulated 
	#pragma omp parallel
  	{
  		while(interval_done < interval_number){
  			(void) omp_set_lock(&sim_state_lock);//acquire lock to read the states
  			int i; 
  			for( i = 0; i < interval_number && (!available_interval[i] || computing[i]); i++){//look for a slice with new bounds which is not being computed
  			}						 
  			if(i == interval_number){
  				(void) omp_unset_lock (&sim_state_lock);//release the lock, we have not found an interesting interval
  			}
  			else{//the interval i is avalaible and we simulate it
	  			available_interval[i] = 0;
	  			computing[i] = 1;
	  			(void) omp_unset_lock (&sim_state_lock);//release the lock
	  			STATE *ub = ub_trajectory + i*interval_size;
				STATE *lb = lb_trajectory + i*interval_size;
		   		double *random = randomness + i*interval_size;
		   		if(!compare(lb[0],ub[0])){//detect if it is the last time the slice is simulated
	  				#pragma omp atomic
	  				interval_done++;
	  			}
		   		//printf("La thread %d simule la tranche %d sur %d étapes avec comme valeur de départ %d, %d.\n",omp_get_thread_num(),i,sizes[i],lb[0],ub[0]);
	  			int not_coupled = (sizes[i] == interval_size); // do not update bounds when they are already optimal
	  			sizes[i] = simul_interval(lb, ub, sizes[i], random);
	  			if( i != interval_number - 1 && not_coupled) {
	  				if(update_next_bounds(interval_size, sizes[i], lb, ub, random)){
	  					if(sizes[i+1] == interval_number || sizes[i] != interval_size){
	  					 //do not recompute a slice if it has already coupled except for the final simulation. Should help slow coupling
		  					#pragma omp atomic write //atomic to not mess up with another thread which can put it to 0 
				    		available_interval[i+1] = 1;
				    	}
	  				}		
			    }
			    computing[i]=0;
			}
  		}
  	}
  	(void) omp_destroy_lock(&sim_state_lock);
  	free(computing);
  	free(available_interval);
  	free(sizes);
}


typedef struct{unsigned char load; unsigned char pos;} statistic;

void par_sim_balanced(STATE *lb_trajectory, STATE *ub_trajectory, double *randomness, int size, int interval_number, int meta_interval_number){ //use only two binary states by slice + stats
	int interval_size = size/interval_number; 
	int meta_interval_size = interval_number/meta_interval_number;//number of intervals in a meta-interval
 	int *available_interval = malloc(sizeof(int)*interval_number);//boolean to store if an interval has new bounds	
	int *computing = malloc(sizeof(int)*interval_number);//boolean to store if a thread is computing an interval
	int *sizes = malloc(sizeof(int)*interval_number);//store the size of what is left to simulate for each interval
	statistic *interval_stats = malloc(sizeof(statistic)*meta_interval_number);
 	for(int i = 0; i < meta_interval_number; i++){
 		interval_stats[i].load = 0;
 		interval_stats[i].pos = i;
 	}
	init_interval(interval_size,interval_number,lb_trajectory,ub_trajectory);	

	//we use the following lock to keep the states of the sim coherent between threads	
	omp_lock_t sim_state_lock;
	(void) omp_init_lock (&sim_state_lock);
	for(int i = 0; i < interval_number; i++) {//initialization of the states of each interval
		sizes[i] = interval_size;
		available_interval[i] = 1;
		computing[i]= 0;
	}
	sizes[interval_number-1]+= size%interval_number;//the few additional steps are put one the last interval
	int interval_done = 0;//number of interval correctly simulated 
	#pragma omp parallel
  	{
  		while(interval_done < interval_number){
  			(void) omp_set_lock(&sim_state_lock);//acquire lock to read the states
  			int i = 0, j = 0; 
			for( j = 0 ; j < meta_interval_number; j++){//look for a free slice inside the meta interval j
  				for(i = interval_stats[j].pos*meta_interval_size; i < (interval_stats[j].pos+1)*meta_interval_size && (!available_interval[i] || computing[i]); i++){}			 
  				if(i < (interval_stats[j].pos+1)*meta_interval_size) {break;}//we have found the correct i
  			}
  			if(j == meta_interval_number){
  				(void) omp_unset_lock (&sim_state_lock);//release the lock, we have not found an interesting interval
  			}
  			else{//the interval i is avalaible and we simutate it
	  			available_interval[i] = 0;
	  			computing[i] = 1;
	  			interval_stats[j].load++;//update the stats : meta interval j has been used once more
	  			while(j < meta_interval_number - 1 && (interval_stats[j].load > interval_stats[j+1].load ||(interval_stats[j].load == interval_stats[j+1].load && interval_stats[j].pos > interval_stats[j+1].pos))){
	  				statistic temp = interval_stats[j+1];
	  				interval_stats[j+1] = interval_stats[j];
	  				interval_stats[j] = temp;
	  				j++;
	  			} 
	  			(void) omp_unset_lock (&sim_state_lock);//release the lock
	  			STATE *ub = ub_trajectory + i*interval_size;
				STATE *lb = lb_trajectory + i*interval_size;
		   		double *random = randomness + i*interval_size;
		   		if(!compare(lb[0],ub[0])){//detect if it is the last time the slice is simulated
	  				 #pragma omp atomic
	  				interval_done++;
	  			}
		   		//printf("La thread %d simule la tranche %d sur %d étapes avec comme valeur de départ %d, %d.\n",omp_get_thread_num(),i,sizes[i],lb[0],ub[0]);
	  			int not_coupled = (sizes[i] == interval_size); // do not update bounds when they are already optimal
	  			sizes[i] = simul_interval(lb, ub, sizes[i], random);
	  			if( i != interval_number - 1 && not_coupled) {
	  				if(update_next_bounds(interval_size, sizes[i], lb, ub, random)){
	  					if(sizes[i+1] == interval_number || sizes[i] != interval_size){
	  					 //do not recompute a slice if it has already coupled except for the final simulation. Should help slow coupling
		  					#pragma omp atomic write //atomic to not mess up with another thread which can put it to 0 
				    		available_interval[i+1] = 1;
				    	}
	  				}		
			    }
			    computing[i]=0;
			}
  		}
  	}
  	(void) omp_destroy_lock(&sim_state_lock);
  	free(computing);
  	free(available_interval);
  	free(sizes);
  	free(interval_stats);
}



void par_sim_balanced_slices(STATE *lb_trajectory, STATE *ub_trajectory, double *randomness, int size, int interval_number){ 
	par_sim_balanced(lb_trajectory, ub_trajectory, randomness, size, interval_number, interval_number);//the last value is how much intervals are in a meta interval 
}


double time_diff(struct timeval tv1, struct timeval tv2){
    return ((double)tv2.tv_sec*(double)1000 +(double)tv2.tv_usec/(double)1000) - ((double)tv1.tv_sec*(double)1000 + (double)tv1.tv_usec/(double)1000);
}


void statistics(int experiment_number, int size, int interval_number, double *mean, double *var, unsigned int seed, STATE* lb_trajectory, STATE* ub_trajectory, void (*simulation)(STATE*, STATE*, double*, int, int)){
//experiment_number is the number of experiment must be larger than 1, size is the size of the simulation, mean and va are used to store the mean and var of the computaton time
	double *randomness = malloc(sizeof(double)*size);
	double *results = malloc(sizeof(double)*experiment_number);
	struct timeval start, end;
	srand(seed);
	InitWELLRNG512a(seed);//init the two RNG generators, using the seed
	for(int i = 0; i < experiment_number; i++){ //repeat the simulation experiment-number of times and times it
		//init the instance of the simulation
		ub_trajectory[0] =  random_state();
		lb_trajectory[0] = ub_trajectory[0]; 
		for(int i = 0; i < size; i++) randomness[i] = WELLRNG512a();	
		gettimeofday (&start, NULL); 
		simulation(lb_trajectory,ub_trajectory,randomness,size,interval_number);
		gettimeofday (&end, NULL);
		//printf("Temps écoulé pendant la simulation séquentielle %f millisecondes.\n",time_diff(start,end));
		results[i] = time_diff(start,end); 	
		fprintf(stdout,"\r %d  per cent of experiments done\n", ((i+1)*100)/(experiment_number));
		fflush(stdout); 
			}
	*mean = 0;
	*var = 0;
	for(int i=0 ; i < experiment_number; i++) *mean += results[i];
	*mean /= experiment_number;
	for(int i=0; i<experiment_number; i++) *var += (*mean - results[i])*(*mean -results[i]);
	*var = sqrt(*var/(experiment_number-1));
	free(randomness);
	free(results);
}


int main(){

	
	int size = 10000000;//length of the simulation
	int experiment_number = 100;//should be larger than one
	int interval_number = omp_get_max_threads();

	//memory used in all experiments
	STATE *lb_trajectory = malloc(sizeof(STATE)*size);
	STATE *ub_trajectory = malloc(sizeof(STATE)*size);//the initial state is in the first elements of these two arrays
	unsigned int seed = time(NULL);
	//hack to allocate the memory locally only once
	ub_local = malloc(sizeof(STATE*)*interval_number);
	lb_local = malloc(sizeof(STATE*)*interval_number);
	random_local = malloc(sizeof(double*)*interval_number);
	#pragma omp parallel for schedule(monotonic:dynamic) 
	for(int i = 0; i < interval_number; i++){
		 ub_local[i] =  malloc(sizeof(STATE)*size/interval_number);
		 lb_local[i] =  malloc(sizeof(STATE)*size/interval_number);
	     random_local[i] = malloc(sizeof(double)*size/interval_number);
	}

	double mean, var;
	statistics(experiment_number,size,interval_number,&mean,&var,seed,lb_trajectory,ub_trajectory,sequential_sim);
	printf("Algorithme séquentiel: temps moyen %f variance %f. \n\n",mean,var);
	double seq_speed = mean;

	statistics(experiment_number,size,interval_number,&mean,&var,seed,lb_trajectory,ub_trajectory,par_sim_fixed_slices);
	printf("Algorithme parallèle sans lock: temps moyen %f variance %f accélération %f\n\n",mean,var,seq_speed/mean);


	statistics(experiment_number,size,interval_number,&mean,&var,seed,lb_trajectory,ub_trajectory,par_sim_fixed_slices_local);
	printf("Algorithme parallèle sans lock à mémoire locale: temps moyen %f variance %f accélération %f\n\n",mean,var,seq_speed/mean);

	statistics(experiment_number,size,interval_number,&mean,&var,seed,lb_trajectory,ub_trajectory,par_sim_first_slices_first);
	printf("Algorithme parallèle premières tranches en premiers : temps moyen %f variance %f accélération %f\n\n",mean,var,seq_speed/mean);


	statistics(experiment_number,size,interval_number,&mean,&var,seed,lb_trajectory,ub_trajectory,par_sim_balanced_slices);
	printf("Algorithme parallèle tranche dans les zones les moins traitées en priorité (peu d'intervalles): temps moyen %f variance %f accélération %f\n\n",mean,var,seq_speed/mean);


	statistics(experiment_number,size,4*interval_number,&mean,&var,seed,lb_trajectory,ub_trajectory,par_sim_balanced_slices);
	printf("Algorithme parallèle tranche dans les zones les moins traitées en priorité (bcp d'intervalles): temps moyen %f variance %f accélération %f\n\n",mean,var,seq_speed/mean);

	/*************************Evaluation of the coupling time************/
	if(COUPLING_TIME){
		int coupling_time = 0;
		unsigned int *var_coupling_time = malloc(sizeof(unsigned int)*100);
		double *randomness = malloc(sizeof(double)*size);
		STATE *lb_trajectory = malloc(sizeof(STATE)*size);
		ub_trajectory[0] = max_state();
		lb_trajectory[0] = min_state();
		for(int i = 0; i < 100; i++){
			InitWELLRNG512a(time(NULL));//init the RNG generator, using a mostly random time
			for(int j = 0; j < size; j++) randomness[j] = WELLRNG512a();
			var_coupling_time[i] = simul_interval(lb_trajectory,ub_trajectory,size,randomness); 
			coupling_time += var_coupling_time[i];
		}
		coupling_time /= 100;
		long unsigned int variancect = 0;
		for(int i = 0; i < 100; i++) variancect += (coupling_time-var_coupling_time[i])*(coupling_time-var_coupling_time[i]); 
		printf("Average coupling time of %d, variance of %d.\n",coupling_time, (int) sqrt(variancect/99));
		free(var_coupling_time);
		free(lb_trajectory);
	}
	free(lb_trajectory);
	free(ub_trajectory);
}