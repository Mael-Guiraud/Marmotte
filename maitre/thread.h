
void *server_listener(void *arg);
void *server_listener_optim(void *arg);
int* create_sockets(int * socket_desc);
int create_threads(int * servers_id,argument * args);
void wait_all_threads_close();
void wait_threads(int nb_inter);