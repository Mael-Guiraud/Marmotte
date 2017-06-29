#include <sys/select.h>		//include fd_set struct

int initialize_set(fd_set *set, int nb_servers, int *servers_id);
int* create_and_connect_sockets();
