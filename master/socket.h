#include <sys/select.h>		//include fd_set struct

int read_servers_adresses(char ** ip_adresses);	//return the number of ip red in the IP adress file
int initialize_set(fd_set *set, int nb_servers, int *servers_id);
int* create_and_connect_sockets();
