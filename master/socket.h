#include <sys/select.h>		//include fd_set struct
#include "struct.h"
int initialize_set(fd_set *set, int nb_servers, int *servers_id);
int* create_and_connect_sockets(int nb_machines);
void destroy_sockets(int * sockets, int nb_machines);
void ask_for_time_display(double** times,int *message,int message_size,int *servers_id, int nb_machines,int nb_measures);
void build_bounds_message(int *message, Bounds *bounds, int interval, int interval_size, int seed, int nb_queues);
void build_seed_message(int *message, int nb_interval, Algo a);
void send_config(int * message,int message_size, int * servers_id, int nb_machines, int min, int max, float load, float p, float mu, int nb_queues);
void send_exit(int * message,int message_size, int * servers_id, int nb_machines);
void send_reinit_seeds(int * message,int* servers_id, int * seeds,int message_size, int nb_machines, int nb_inter,Algo a);
void build_bound_message(int *message, Bounds *bounds, int interval, int interval_size, int seed, int nb_queues);
void build_bound_traj_message(int *message, Bounds *bounds, int interval, int interval_size, int seed, int nb_queues);