isr: isr.o bmp.o blur.o brightness.o compare.o dmc.o signs.o autocorr.o
	g++ -o isr isr.o bmp.o blur.o brightness.o compare.o dmc.o signs.o autocorr.o

isr.o: isr.cpp bmp.h isr.h
	g++ -c isr.cpp -msse2

bmp.o: bmp.cpp bmp.h
	g++ -c bmp.cpp -msse2

blur.o: blur.cpp bmp.h isr.h
	g++ -c blur.cpp -msse2

brightness.o: brightness.cpp bmp.h isr.h
	g++ -c brightness.cpp -msse2

compare.o: compare.cpp bmp.h isr.h
	g++ -c compare.cpp -msse2

dmc.o: dmc.cpp bmp.h isr.h
	g++ -c dmc.cpp

sings.o: signs.cpp bmp.h isr.h
	g++ -c signs.cpp

autocorr.o: autocorr.cpp bmp.h isr.h
	g++ -c autocorr.cpp -msse2

scale: scale.cpp bmp.o
	g++ -o scale scale.cpp bmp.o -msse2
