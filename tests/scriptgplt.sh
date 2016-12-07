#!/bin/bash



for X in '1.0'
do

	echo "plot   'distribution_couplage$X.data'  using 1:2  notitle 
	set title \"distribution des temps de couplage\"
	set ylabel \"nb occurences(pour 1 million)\"
	set xlabel \"temps de couplage \"
	set term postscript color solid
	set output \"distribs$X.ps\"
	replot
	" > /home/merelas/Bureau/parallel/tests/distribs$X.gplt
	gnuplot distribs$X.gplt
	ps2pdf distribs$X.ps
done