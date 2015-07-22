#include "isr.h"

bmpimage *img_flt;

void filter() {
  int mx=sign_tab[0].sample->sizex(), my=sign_tab[0].sample->sizey();
  for(int i=1; i<sign_tab.size(); i++) {
    if(mx>sign_tab[i].sample->sizex())
      mx=sign_tab[i].sample->sizex();
    if(my>sign_tab[i].sample->sizey())
      my=sign_tab[i].sample->sizey();
  }
  unsigned sad[my][mx];
  bzero(sad,sizeof(sad));
  for(int i=1; i<sign_tab.size(); i++) {
    int sh1=(sign_tab[i].sample->sizex()-mx)/2, sv1=(sign_tab[i].sample->sizey()-my)/2;
    for(int j=0; j<i; j++) {
      int sh2=(sign_tab[j].sample->sizex()-mx)/2, sv2=(sign_tab[j].sample->sizey()-my)/2;
      for(int y=0; y<my; y++)
        for(int x=0; x<mx; x++)
          sad[y][x]+=abs(int((*sign_tab[i].sample)[y+sv1][x+sh1])-int((*sign_tab[j].sample)[y+sv2][x+sh2]));
    }
  }
  unsigned min=sad[0][0],max=0;
  for(int y=0; y<my; y++)
    for(int x=0; x<mx; x++) {
      if(min>sad[y][x]) min=sad[y][x];
      if(max<sad[y][x]) max=sad[y][x];
    }
  img_flt=new bmpimage(8,mx,my);
  for(int y=0; y<my; y++)
    for(int x=0; x<mx; x++)
      (*img_flt)[y][x]=(unsigned char)(float(sad[y][x]-min)*255/(max-min));
}
