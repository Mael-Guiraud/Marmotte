#ifndef STRUCTS
#define STRUCTS
 
// Contenu du .h
 

//A structure for the two bounds "inf" and "sup"
typedef struct bounds
{
    int* lb;	//lower bound
    int* ub;	//upper bound
} Bounds;

//States of the intervals
typedef enum interval_state{
    SENT, UPDATED,FINISHED, VALIDATED
} Interval_state;

//Different kinds of messages sent by the servers
typedef enum message_kind{
    PAUSE, BOUNDS, TRAJECTORY
} Message_kind;

//Kind of messages sent by the master
typedef enum MESSAGE_HEAD{
    REINIT_SEED, INTERVAL,NEW_SIMUL, QUIT, SEND_MEASURES
} Message_head;

#endif

