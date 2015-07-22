#include <string.h>
#include "isr.h"

void blur1(bmpimage& img) {
  bmpimage tmp(img.debth(),img.maxx()+1,img.maxy()+1);
  int l=img.maxx();
  for(int i=1; i<img.maxy(); i++)
    for(int j=0; j<=l; j+=16)
      asm(
        "movdqu %0, %%xmm0\n\t"
        "movdqu %2, %%xmm2\n\t"
        "movdqu %1, %%xmm1\n\t"
        "pavgb %%xmm2, %%xmm0\n\t"
        "pavgb %%xmm0, %%xmm1\n\t"
        "movdqu %%xmm1, %3"
        : : "m"(img[i-1][j]), "m"(img[i][j]), "m"(img[i+1][j]), "m"(tmp[i][j]): "%xmm0", "%xmm1", "%xmm2"
      );
  for(int i=0; i<=img.maxy(); i++)
    for(int j=1; j<l; j+=16)
      asm (
        "movdqu -1(%0,%1,1), %%xmm0\n\t"
        "movdqu 1(%0,%1,1), %%xmm1\n\t"
        "movdqu (%0,%1,1), %%xmm2\n\t"
        "pavgb %%xmm1, %%xmm0\n\t"
        "pavgb %%xmm0, %%xmm2\n\t"
        "movdqu %%xmm2, (%0,%1,1)"
        : : "r"(tmp[i]),"r"(j) : "%xmm0", "%xmm1", "%xmm2"
      );
  --l;
  for(int i=1; i<img.maxy(); i++)
    memcpy(img[i]+1, tmp[i]+1, l);
}

void blur2(bmpimage& img) {
  bmpimage tmp(img.debth(),img.maxx()+1,img.maxy()+1);
  int l=img.maxx();
  for(int i=1; i<img.maxy(); i++)
    for(int j=0; j<=l; j+=16)
      asm(
        "movdqu (%0,%4,1), %%xmm0\n\t"
        "movdqu (%2,%4,1), %%xmm1\n\t"
        "movdqu -1(%1,%4,1), %%xmm2\n\t"
        "movdqu 1(%1,%4,1), %%xmm3\n\t"
        "movdqu (%1,%4,1), %%xmm4\n\t"
        "pavgb %%xmm1, %%xmm0\n\t"
        "pavgb %%xmm3, %%xmm2\n\t"
        "pavgb %%xmm2, %%xmm0\n\t"
        "pavgb %%xmm0, %%xmm4\n\t"
        "movdqu %%xmm4, (%3,%4,1)"
        : : "r"(img[i-1]), "r"(img[i]), "r"(img[i+1]), "r"(tmp[i]), "r"(j): "%xmm0", "%xmm1", "%xmm2"
      );
  --l;
  for(int i=1; i<img.maxy(); i++)
    memcpy(img[i]+1, tmp[i]+1, l);
}

void blur4(bmpimage& img) {
  bmpimage tmp(img.debth(),img.maxx()+1,img.maxy()+1);
  int l=img.maxx();
  for(int i=1; i<img.maxy(); i++)
    for(int j=0; j<=l; j+=16)
      asm(
        "movdqu (%0,%4,1), %%xmm0\n\t"
        "movdqu (%2,%4,1), %%xmm1\n\t"
        "movdqu -1(%1,%4,1), %%xmm2\n\t"
        "movdqu 1(%1,%4,1), %%xmm3\n\t"
        "movdqu (%1,%4,1), %%xmm4\n\t"
        "pavgb %%xmm1, %%xmm0\n\t"
        "pavgb %%xmm3, %%xmm2\n\t"
        "pavgb %%xmm2, %%xmm0\n\t"
        "pavgb %%xmm4, %%xmm0\n\t"
        "pavgb %%xmm0, %%xmm4\n\t"
        "movdqu %%xmm4, (%3,%4,1)"
        : : "r"(img[i-1]), "r"(img[i]), "r"(img[i+1]), "r"(tmp[i]), "r"(j): "%xmm0", "%xmm1", "%xmm2"
      );
  --l;
  for(int i=1; i<img.maxy(); i++)
    memcpy(img[i]+1, tmp[i]+1, l);
}

void blur3(bmpimage& img) {
  bmpimage tmp(img.debth(),img.maxx()+1,img.maxy()+1);
  int l=img.maxx();
  for(int i=1; i<img.maxy(); i++)
    for(int j=0; j<=l; j+=16)
      asm(
        "movdqu %0, %%xmm0\n\t"
        "movdqu %2, %%xmm2\n\t"
        "movdqu %1, %%xmm1\n\t"
        "pavgb %%xmm2, %%xmm0\n\t"
        "pavgb %%xmm1, %%xmm0\n\t"
        "pavgb %%xmm0, %%xmm1\n\t"
        "movdqu %%xmm1, %3"
        : : "m"(img[i-1][j]), "m"(img[i][j]), "m"(img[i+1][j]), "m"(tmp[i][j]): "%xmm0", "%xmm1", "%xmm2"
      );
  for(int i=0; i<=img.maxy(); i++)
    for(int j=1; j<l; j+=16)
      asm (
        "movdqu -1(%0,%1,1), %%xmm0\n\t"
        "movdqu 1(%0,%1,1), %%xmm1\n\t"
        "movdqu (%0,%1,1), %%xmm2\n\t"
        "pavgb %%xmm1, %%xmm0\n\t"
        "pavgb %%xmm2, %%xmm0\n\t"
        "pavgb %%xmm0, %%xmm2\n\t"
        "movdqu %%xmm2, (%0,%1,1)"
        : : "r"(tmp[i]),"r"(j) : "%xmm0", "%xmm1", "%xmm2"
      );
  --l;
  for(int i=1; i<img.maxy(); i++)
    memcpy(img[i]+1, tmp[i]+1, l);
}

void runblur(bmpimage& img, const char* arg) {
  for(; *arg; arg++)
    switch(*arg) {
      case '1':
        blur1(img);
        break;
      case '2':
        blur2(img);
        break;
      case '3':
        blur3(img);
        break;
      case '4':
        blur4(img);
        break;
    }
}
