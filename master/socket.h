#include <sys/select.h>		//include fd_set struct
#include "struct.h"
int initialize_set(fd_set *set, int nb_servers, int *servers_id);
int* create_and_connect_sockets(int nb_machines);
void destroy_sockets(int * sockets, int nb_machines);
void ask_for_time_display(int *message,int message_size,int *servers_id, int nb_machines);
void build_bounds_message(int *message, Bounds *bounds, int interval, int interval_size, int seed, int nb_queues);
void build_seed_message(int *message, int nb_interval);
void build_config_message(int *message, int min, int max, double load, double rho, double mu);
void send_config(int * message,int message_size, int * servers_id, int nb_machines, int min, int max, float load, float rho, float mu);
