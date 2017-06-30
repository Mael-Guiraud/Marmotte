#include <sys/select.h>		//include fd_set struct

int initialize_set(fd_set *set, int nb_servers, int *servers_id);
int* create_and_connect_sockets();
void destroy_sockets(int *sockets);
void ask_for_time_display(int *servers_id);
