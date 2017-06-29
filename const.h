

////////////////COMMON PART//////////////////////
#define NB_QUEUES 2


/////////////////FOR MASTER/////////////////////
//Number of servers connected ot the master
#define NB_MACHINES 1


//The parameters of the simulation loop
#define SEQUENCE_SIZE 300000
#define NB_SIMULS 100
#define INTERVAL_SIZE_MIN 5000
#define INTERVAL_SIZE_MAX 60000

#define STEP 5000

#define MOD 0// 0 For round count, 1 for optimized version


//The maximum number of clients in a queue
#define BUFF_MAX 100

#define INIT 0 //0 initial states = 0 on all queues, 1 = 100, 2 = rand

////////////////////For servers///////////////////////

#define EXEC_TYPE 1// 0 = 127.0.0.1 / 1=192.168.90.178
