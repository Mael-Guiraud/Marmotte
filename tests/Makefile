all: gnuplot clean

gnuplot: calcul
	./scriptgplt.sh
	
calcul: occurences
	./occurences
occurences: MaelTandem.c
	gcc -o occurences MaelTandem.c
	
clean:
	rm -f *.ps
	rm -f convert
