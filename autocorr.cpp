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

static const long long mask[4]={-1,-1,0,0};

unsigned compare(bmpimage& a, bmpimage& b) {
  int l=(a.maxx()<b.maxx()?a.maxx():b.maxx());
  asm (
    "pxor %%xmm0, %%xmm0\n\t"
    "dec %1\n\t"
    "and $15, %1\n\t"
    "sub $15, %1\n\t"
    "neg %1\n\t"
    "movdqu (%0,%1,1),%%xmm1\n\t"
    : : "r"(mask),"r"(l) : "%xmm0","%xmm1"
  );
  for(int i=0; i<=a.maxy() && i<b.maxy(); i++) {
    for(int j=0; j+16<l; j+=16)
      asm (
	"movdqu (%0,%2,1), %%xmm2\n\t"
	"movdqu (%1,%2,1), %%xmm3\n\t"
	"psadbw %%xmm3, %%xmm2\n\t"
	"paddq %%xmm2, %%xmm0\n\t"
	: : "r"(a[i]),"r"(b[i]),"r"(j): "%xmm0","%xmm2","%xmm3"
      );
    asm (
	"movdqu (%0,%2,1), %%xmm2\n\t"
	"movdqu (%1,%2,1), %%xmm3\n\t"
	"pand %%xmm1, %%xmm2\n\t"
	"pand %%xmm1, %%xmm3\n\t"
	"psadbw %%xmm3, %%xmm2\n\t"
	"paddq %%xmm2, %%xmm0\n\t"
	: : "r"(a[i]),"r"(b[i]),"r"(l&~15): "%xmm0","%xmm1","%xmm2","%xmm3"
    );
  }
  unsigned sum[4];
  asm("movdqu %%xmm0, %0": :"m"(*sum):"%xmm0");
  return sum[0]+sum[2];
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
