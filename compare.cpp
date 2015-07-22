#include "isr.h"

static const long long mask[4]={-1L,-1L,0,0};

float compare(bmpimage& a, bmpimage& b) {
  int l=(a.maxx()<b.maxx()?a.maxx():b.maxx());
  int h=(a.maxy()<b.maxy()?a.maxy():b.maxy());
  asm (
    "pxor %%xmm0, %%xmm0\n\t"
    "and $15, %1\n\t"
    "sub $15, %1\n\t"
    "neg %1\n\t"
    "movdqu (%0,%1,1),%%xmm1\n\t"
    : : "r"(mask),"r"(l) : "%xmm0","%xmm1"
  );
  l-=16;
  for(int i=0; i<=h; i++) {
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
  return float(sum[0]+sum[2])/((l+17)*(h+1));
}

float compare(bmpimage& a, bmpimage& b, int sx, int sy) {
  int x0=(sx>=0?sx:0), y0=(sy>=0?sy:0);
  sx-=x0; sy-=y0;
  int l=(a.sizex()<b.sizex()+x0?a.sizex()-x0:b.sizex()+sx)-1;
  int h=(a.sizey()<b.sizey()+y0?a.sizey()-y0:b.sizey()+sy);
  asm (
    "pxor %%xmm0, %%xmm0\n\t"
    "and $15, %1\n\t"
    "sub $15, %1\n\t"
    "neg %1\n\t"
    "movdqu (%0,%1,1),%%xmm1\n\t"
    : : "r"(mask),"r"(l) : "%xmm0","%xmm1"
  );
  l-=16;
  for(int i=0; i<h; i++) {
    register unsigned char* aptr=a[i+y0]+x0, *bptr=b[i-sy]-sx;
    register int j;
    for(j=0; j<l; j+=16)
      asm (
        "movdqu (%0,%2,1), %%xmm2\n\t"
        "movdqu (%1,%2,1), %%xmm3\n\t"
        "psadbw %%xmm3, %%xmm2\n\t"
        "paddq %%xmm2, %%xmm0\n\t"
        : : "r"(aptr),"r"(bptr),"r"(j): "%xmm0","%xmm2","%xmm3"
      );
    asm (
        "movdqu (%0,%2,1), %%xmm2\n\t"
        "movdqu (%1,%2,1), %%xmm3\n\t"
        "pand %%xmm1, %%xmm2\n\t"
        "pand %%xmm1, %%xmm3\n\t"
        "psadbw %%xmm3, %%xmm2\n\t"
        "paddq %%xmm2, %%xmm0\n\t"
        : : "r"(aptr),"r"(bptr),"r"(j): "%xmm0","%xmm1","%xmm2","%xmm3"
    );
  }
  unsigned sum[4];
  asm("movdqu %%xmm0, %0": :"m"(*sum):"%xmm0");
  return float(sum[0]+sum[2])/((l+17)*h);
}
