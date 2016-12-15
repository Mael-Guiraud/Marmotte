all: gnuplot clean

gnuplot: temps.gplt 
	gnuplot temps.gplt
	ps2pdf temps.ps
	evince temps.pdf

clean:
	rm -f temps.ps