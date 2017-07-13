
int initialize_set(fd_set *set, int server_socket, int master_socket);
double intToDouble(int* tab,int indice);
double intToDoubleLoad(int* tab,int indice);
double** init_random_sequences(int nb_inter);
void free_random_sequences(double** tab,int nb_inter);
double * gives_un(double ** seeds, int inter_size,int inter_id,int seed);