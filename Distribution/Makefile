all: gnuplot clean

gnuplot: calcul
	./scriptgplt.sh
	
calcul: occurences
	./occurences
occurences: distributions.c
	gcc -o occurences distributions.c
	
clean:
	rm -f *.ps
	rm -f occurences
	rm -f *.gplt
	rm -f *.data

