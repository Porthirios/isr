#include <string,h>
#include "isr.h"

int spectrum(bmpimage& img, int (&sp)[256]) {
  bzero(sp,256*sizeof(int));
  for(int i=0; i<img.sizey(); i++)
    for(int j=0; j<img.sizex(); j++)
      sp[img[i][j]]++;
  int m=0;
  for(int c=0; c<256; c++)
    if(m<sp[c]) m=sp[c];
  return m;
}

void bmpspectrum(bmpimage& img, bmpimage& spimg) {
  int sp[256];
  int m=spectrum(img,sp);
  spimg.resize(256,256);
  for(int c=0; c<256; c++) {
    int cc=int(sp[c]*256./m);
    for(int i=0; i<cc; i++)
      spimg.pset(c,i,0);
    for(int i=cc; i<256; i++)
      spimg.pset(c,i,-1);
  }
}
