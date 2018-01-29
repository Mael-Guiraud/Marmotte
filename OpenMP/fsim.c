#include "fsim.h"
#include <stdlib.h>
#include <stdio.h>
#define MAX_BUFFER 10


void print_trajectory(STATE *traj, int size){
	printf("Trajectory : ");
	for(int i = 0; i < size; i++){
		printf("%d ",traj[i]);
	}
	printf("\n");
}

int transition(STATE t1, double random){

if(random < 0.5 && t1 >0)  t1--;
if(random >= 0.5 && t1 < MAX_BUFFER)  t1++;
return t1;
}

int compare(STATE t1, STATE t2){
	if( t1 == t2 ) return 0;
	return (t1 < t2) ? -1 : 1;
}

STATE random_state(){
	STATE t = rand()% (MAX_BUFFER +1);
	return t;
}


STATE min_state(){
	return 0;
}

STATE max_state(){
	return MAX_BUFFER;
}