#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "isr.h"

float autocorr(bmpimage& img, int x, int y) {
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
  return float(sum[0]+sum[1])/((img.sizex()-x)*(img.sizey()-y));
}

float autostep(bmpimage& img) {
  float a0=autocorr(img,10,10), a1=autocorr(img,11,11), s=a0+a1;
  for(int i=12; i<100; i++) {
    float a=autocorr(img,i,i);
    s+=a;
    if(a0>a1 && a1<a && a1<s/(i-9)*0.85) {
      return i-1+(a-a0)/(2*a0-4*a1+2*a);
    }
    a0=a1; a1=a;
  }
}
