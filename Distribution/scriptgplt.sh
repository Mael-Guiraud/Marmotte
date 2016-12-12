#!/bin/bash



for X in `seq -f 0.%g 1 9` `seq -f 1.%g 0 5`
do

	echo "plot   'res/distribution_couplage$X.data'  using 1:2  notitle with lines
	set title \"distribution des temps de couplage\"
	set ylabel \"nb occurences(pour 1 million)\"
	set xlabel \"temps de couplage \"
	set term postscript color solid
	set output \"distribs$X.ps\"
	set xrange [0:40000]
	replot
	" > /home/merelas/Bureau/parallel/Distribution/distribs$X.gplt
	gnuplot distribs$X.gplt
	ps2pdf distribs$X.ps
done