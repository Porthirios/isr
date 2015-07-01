#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "bmp.h"

unsigned long long autocorr(bmpimage& img, int x, int y) {
  asm (
    "pxor %%xmm0, %%xmm0\n\t"
    : : : "%xmm0"
  );
  for(int i=0; i+y<=img.maxy(); i++)
    for(int j=0; j+x+16<=img.maxx(); j+=16)
      asm(
	"movdqu (%0,%2,1), %%xmm1\n\t"
	"movdqu (%1,%3,1), %%xmm2\n\t"
	"psadbw %%xmm2, %%xmm1\n\t"
	"paddq %%xmm1, %%xmm0\n\t"
	: : "r"(img[i]), "r"(img[i+y]), "r"(j), "r"(j+x)
      );
  unsigned long long sum[2];
  asm (
    "movdqu %%xmm0, %0"
    : : "m"(*sum) : "%xmm0"
  );
  return sum[0]+sum[1];
}

main(int argc, const char** argv) {
  if(!argv) return 1;
  bmpimage img;
  img.load(argv[1]);
  unsigned long long map[32][32],max=0;
  for(int i=0; i<32; i++)
    for(int j=0; j<32; j++) {
      std::cout << (map[i][j]=autocorr(img,j,i)) << (j==31?'\n':'\t');
      if(max<map[i][j]) max=map[i][j];
    }
  max/=254;
  bmpimage imap(8,32,32);
  for(int i=0; i<256; i++)
    imap.setcolor(i,i,i,i);
  for(int i=0; i<32; i++)
    for(int j=0; j<32; j++)
      imap.pset(i,j,int(map[i][j]/max));
  imap.save("map.bmp");
  return 0;
}
