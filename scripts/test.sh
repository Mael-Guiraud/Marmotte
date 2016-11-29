
for X in `seq 1 12`;
do
	echo "#define NComponent 10
#define NB_MACHINES 6
#define X $X
#define TAILLE_SEQUENCE 120000
" > test
done