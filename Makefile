isr: isr.o bmp.o
	g++ -o isr isr.o bmp.o

isr.o: isr.cpp bmp.h
	g++ -c isr.cpp -msse2

bmp.o: bmp.cpp bmp.h
	g++ -c bmp.cpp -msse2

autocorr: autocorr.cpp bmp.o
	g++ -o autocorr autocorr.cpp bmp.o -msse2

compare: compare.cpp bmp.o
	g++ -o compare compare.cpp bmp.o -msse2

scale: scale.cpp bmp.o
	g++ -o scale scale.cpp bmp.o -msse2
