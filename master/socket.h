#include <sys/select.h>		//include fd_set struct

int initialize_set(fd_set *set, int nb_servers, int *servers_id);
int* create_and_connect_sockets();
void destroy_sockets(int *sockets);
void ask_for_time_display(int *servers_id);
void build_bounds_message(int *message, Bounds *bounds, int interval, int interval_size, int seed);
void build_seed_message(int *message, int nb_interval);
void build_config_message(int *message, int min, int max, double load, double rho, double mu);
