#!/bin/bash



for X in '10' '20' '30' '40' '50' '60' '70' '80' '90' '100'
do

	echo "plot   'distribution_couplage$X'  using 1:2  notitle 
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