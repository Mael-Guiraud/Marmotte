#include <stdio.h>
#include <stdlib.h>

void print_gnuplot(char * namepdf, char* namegplt,char* title, char* titlex, char* titley,int min_x,int max_x,int min_y, int max_y,int col)
{

	FILE* f_GPLT = fopen(namegplt,"w");
	if(!f_GPLT){perror("Opening gplt file failure\n");exit(2);}

	fprintf(f_GPLT,


	"set xrange [%d:%d] \n"
	"set xlabel \"%s\" \n"


	"set yrange [%d:%d] \n"	
	"set ylabel \"%s\"\n"

	"plot '../results/inters1.data' using 1:%d title \" One bound \"  \n"
	"replot '../results/inters2.data' using 1:%d title \" Two bound grouped \" \n"
	"replot '../results/inters3.data' using 1:%d title \" Two bound splited\" \n"


	"set key top left \n"
	"set title \"%s\"\n"

	"set term postscript color solid\n"
	
	
	"set output '| ps2pdf - %s'\n"
	"replot\n"
	,min_x,max_x,titlex,min_y,max_y,titley,col,col,col,title,namepdf);
	fclose(f_GPLT);

}