#include <sys/select.h>		//include fd_set struct

int initialize_set(fd_set *set, int nb_servers, int *servers_id);
void *server_listener(void *arg);
void *server_listener_optim(void *arg);
int* create_and_connect_sockets();
int create_threads(int *servers_id, argument * args);
void wait_all_threads_close();
void wait_threads(int nb_inter);
