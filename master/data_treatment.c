#include <stdio.h>
#include <stdlib.h>

void print_gnuplot(char * name,int min_x,int max_x,int min_y, int max_y)
{

	FILE* f_GPLT = fopen("../results/inters.gplt","w");
	if(!f_GPLT){perror("Opening gplt file failure\n");exit(2);}

	fprintf(f_GPLT,"plot '../results/inters.data' notitle \n"
	//"replot 'inters.data' title \"BBU BE\" smooth cumulative\n"

	"set term postscript color solid\n"

	"set xrange [%d:%d] \n"
	"set xrange [%d:%d] \n"
	"set title \"Impact of the size of the interval\"\n"
	"set xlabel \"Interval size\" \n"
	"set ylabel \"Number of intervals calculated\"\n"

	"set key top right \n"
	
	"set output '| ps2pdf - %s'\nreplot\n",min_x,max_x,min_y,max_y,name);
	fclose(f_GPLT);

}