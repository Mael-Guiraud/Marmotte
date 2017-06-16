
int initialize_set(fd_set *set, int server_socket, int master_socket);
double intToDouble(int* tab,int indice);
double intToDoubleLoad(int* tab,int indice);
int** init_random_sequences(int nb_inter);
void free_random_sequences(int** tab,int nb_inter);
int * gives_un(int ** seeds, int inter_size,int inter_id,int seed);