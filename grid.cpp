#include <iostream>
#include "isr.h"

float cmpdark(bmpimage& a, bmpimage& b, int sx, int sy) {
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
        "pmaxub %%xmm3, %%xmm2\n\t"
        "psadbw %%xmm3, %%xmm2\n\t"
        "paddq %%xmm2, %%xmm0\n\t"
        : : "r"(aptr),"r"(bptr),"r"(j): "%xmm0","%xmm2","%xmm3"
      );
    asm (
        "movdqu (%0,%2,1), %%xmm2\n\t"
        "movdqu (%1,%2,1), %%xmm3\n\t"
        "pand %%xmm1, %%xmm2\n\t"
        "pand %%xmm1, %%xmm3\n\t"
        "pmaxub %%xmm3, %%xmm2\n\t"
        "psadbw %%xmm3, %%xmm2\n\t"
        "paddq %%xmm2, %%xmm0\n\t"
        : : "r"(aptr),"r"(bptr),"r"(j): "%xmm0","%xmm1","%xmm2","%xmm3"
    );
  }
  unsigned sum[4];
  asm("movdqu %%xmm0, %0": :"m"(*sum):"%xmm0");
  return float(sum[0]+sum[2])/((l+17)*h);
}

void grid(bmpimage& src, bmpimage& cell, bmpimage& dst) {
  int h=src.sizey()-cell.maxy(), w=src.sizex()-cell.maxx();
  dst.resize(w,h);
  for(int c=0; c<256; c++)
    dst.setcolor(c,c,c,c);
  float min=256.,max=0.,minimax=256.,maximin=0.;
  std::pair<float,float> cmm[src.sizey()/cell.sizey()][src.sizex()/cell.sizex()];
  for(int y=0; y<h; y++)
    for(int x=0; x<w; x++) {
      int cx=x/cell.sizex(), cy=y/cell.sizey();
      if(!(x%cell.sizex()) && !(y%cell.sizey()))
        cmm[cy][cx]=std::pair<float,float>(256.,0.);
      float d=cmpdark(src,cell,x,y);
      if(d<cmm[cy][cx].first) cmm[cy][cx].first=d;
      if(d>cmm[cy][cx].second) cmm[cy][cx].second=d;
      if(d<min) min=d;
      if(d>max) max=d;
      dst.pset(x,y,int((d-1.5)/46.5*255));
    }
  for(int y=0; y<sizeof(cmm)/sizeof(*cmm); y++)
    for(int x=0; x<sizeof(*cmm)/sizeof(**cmm); x++) {
      if(cmm[y][x].first>maximin) maximin=cmm[y][x].first;
      if(cmm[y][x].second<minimax) minimax=cmm[y][x].second;
    }
  std::cout << "min=" << min << "\tmax=" << max << "\nminimax=" << minimax << "\tmaximin=" << maximin << '\n';
}

void vlines(bmpimage& src, bmpimage& dst, int l=32) {
  dst.resize(src.sizex(),src.sizey()-l+1);
  for(int x=0; x<src.sizex(); x++) {
    unsigned s=0;
    for(int y=0; y<l; y++)
      s+=src[y][x];
    dst[0][x]=s/l;
    for(int y=l; y<src.sizey(); y++) {
      s+=src[y][x]; s-=src[y-l][x];
      dst[y-l+1][x]=s/l;
    }
  }
}

void hlines(bmpimage& src, bmpimage& dst, int l=31) {
  dst.resize(src.sizex()-l+1,src.sizey());
  for(int y=0; y<src.sizey(); y++) {
    unsigned s=0;
    for(int x=0; x<l; x++)
      s+=src[y][x];
    dst[y][0]=s/l;
    for(int x=l; x<src.sizex(); x++) {
      s+=src[y][x]; s-=src[y][x-l];
      dst[y][x-l+1]=s/l;
    }
  }
}
