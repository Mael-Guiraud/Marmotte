#include <stdio.h>
#include <stdlib.h>

void print_gnuplot(char * name,int min_x,int max_x,int min_y, int max_y,int min_y2,int max_y2)
{

	FILE* f_GPLT = fopen("../results/inters.gplt","w");
	if(!f_GPLT){perror("Opening gplt file failure\n");exit(2);}

	fprintf(f_GPLT,
	//"replot 'inters.data' title \"BBU BE\" smooth cumulative\n"

	

	"set xrange [%d:%d] \n"
	"set xlabel \"Number of intervals\" \n"


	"set yrange [%d:%d] \n"	
	"set ylabel \"Number of intervals calculated\"\n"

	"plot '../results/inters.data' using 1:2 axis x1y1  title \" Intervals Calculated\" \n"


	"set y2range [%d:%d] \n"
	"set y2label \"Time (ms)\" \n"
	"replot '../results/inters.data' using 1:3 axis x1y2 title \"Time\" \n"

	"set key top left \n"
	"set title \"Impact of the size of the interval\"\n"

	"set term postscript color solid\n"
	
	
	"set output '| ps2pdf - %s'\n"
	"replot\n"
	,min_x,max_x,min_y,max_y,min_y2,max_y2,name);
	fclose(f_GPLT);

}