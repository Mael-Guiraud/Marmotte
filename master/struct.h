#include "../const.h"

//A structure for the two bounds "inf" and "sup"
typedef struct bounds{
    int* x0;
    int* y0;
} Bounds;


 //Thread argument
typedef struct argument{
    int id_socket;     
    int id_machine;	   
} argument;

//States of the threads
typedef enum thread_state{
    OPEN, CLOSED
} Thread_state;

//States of the intervals
typedef enum interval_state{
    SENT, UPDATED,FINISHED, VALIDATED
} Interval_state;

//Different kinds of messages sent by the servers
typedef enum message_kind{
    PAUSE, BOUNDS, TRAJECTORY
} Message_kind;


//Communication between threads and main 
volatile Thread_state thread_activity[NB_MACHINES];
volatile Interval_state * interval_state;
volatile Message_kind what_do_i_read[NB_MACHINES];


int nb_inter;
int quit_threads;
int interval_size;
int** final_result;
Bounds * res;

