all: gnuplot clean

gnuplot: temps.gplt results_moyennes
	gnuplot temps.gplt
	ps2pdf temps.ps
	evince temps.pdf
results_moyennes: convert_files
	./convert
convert_files: convert_files.c
	gcc -o convert convert_files.c
	
clean:
	rm -f temps.ps
	rm -f convert
