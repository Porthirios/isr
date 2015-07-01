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

static const long long mask[4]={-1L,-1L,0,0};

unsigned compare(bmpimage& a, bmpimage& b) {
  int l=(a.maxx()<b.maxx()?a.maxx():b.maxx());
  asm (
    "pxor %%xmm0, %%xmm0\n\t"
    "and $15, %1\n\t"
    "sub $15, %1\n\t"
    "neg %1\n\t"
    "movdqu (%0,%1,1),%%xmm1\n\t"
    : : "r"(mask),"r"(l) : "%xmm0","%xmm1"
  );
  l-=16;
  for(int i=0; i<=a.maxy() && i<=b.maxy(); i++) {
    int j;
    for(j=0; j<l; j+=16)
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
	: : "r"(a[i]),"r"(b[i]),"r"(j): "%xmm0","%xmm1","%xmm2","%xmm3"
    );
  }
  unsigned sum[4];
  asm("movdqu %%xmm0, %0": :"m"(*sum):"%xmm0");
  return sum[0]+sum[2];
}

unsigned compare(bmpimage& a, bmpimage& b, int sx, int sy) {
  int l=(a.maxx()<b.maxx()+sx?a.maxx()-sx:b.maxx());
  asm (
    "pxor %%xmm0, %%xmm0\n\t"
    "and $15, %1\n\t"
    "sub $15, %1\n\t"
    "neg %1\n\t"
    "movdqu (%0,%1,1),%%xmm1\n\t"
    : : "r"(mask),"r"(l) : "%xmm0","%xmm1"
  );
  l-=16;
  for(int i=0; i<=a.maxy() && i+sy<=b.maxy(); i++) {
    int j;
    for(j=0; j<l; j+=16)
      asm (
	"movdqu (%0,%2,1), %%xmm2\n\t"
	"movdqu (%1,%2,1), %%xmm3\n\t"
	"psadbw %%xmm3, %%xmm2\n\t"
	"paddq %%xmm2, %%xmm0\n\t"
	: : "r"(a[i]),"r"(b[i+sy]),"r"(j): "%xmm0","%xmm2","%xmm3"
      );
    asm (
	"movdqu (%0,%2,1), %%xmm2\n\t"
	"movdqu (%1,%2,1), %%xmm3\n\t"
	"pand %%xmm1, %%xmm2\n\t"
	"pand %%xmm1, %%xmm3\n\t"
	"psadbw %%xmm3, %%xmm2\n\t"
	"paddq %%xmm2, %%xmm0\n\t"
	: : "r"(a[i]),"r"(b[i+sy]),"r"(j): "%xmm0","%xmm1","%xmm2","%xmm3"
    );
  }
  unsigned sum[4];
  asm("movdqu %%xmm0, %0": :"m"(*sum):"%xmm0");
  return sum[0]+sum[2];
}

main(int argc, const char** argv) {
  if(argc<2) return 1;
  int cnt=argc-1;
  bmpimage img[cnt];
  for(int i=1; i<argc; i++)
    img[i-1].load(argv[i]);
  unsigned dis[cnt][cnt],max=0,min=-1;
  for(int i=0; i<cnt; i++)
    for(int j=0; j<cnt; j++) if(i!=j) {
      std::cout << (dis[i][j]=compare(img[i],img[j])) << (j==cnt-1?'\n':'\t');
      if(max<dis[i][j]) max=dis[i][j];
      if(min>dis[i][j]) min=dis[i][j];
    }
  std::cout << "\nmin=" << min << "\tmax=" << max << '\n';
  return 0;
}
