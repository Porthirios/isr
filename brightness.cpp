#include "isr.h"

void brightness(bmpimage& img) {
  unsigned mi=255,ma=0;
  for(int i=0; i<img.sizey(); i++)
    for(int j=0; j<img.sizex(); j++) {
      if(img[i][j]>ma) ma=img[i][j];
      if(img[i][j]<mi) mi=img[i][j];
    }
  float k=255./(ma-mi);
  for(int i=0; i<img.sizey(); i++)
    for(int j=0; j<img.sizex(); j++)
      img[i][j]=unsigned((img[i][j]-mi)*k);
}

void brightness_a(bmpimage& img) {
  int cx=(img.maxx()>>4)+1,cy=(img.maxy()>>4)+1;
  std::pair<unsigned char, unsigned char> mm[cy][cx];
  for(int y=0; y<img.sizey(); y+=16)
    for(int x=0; x<img.sizex(); x+=16) {
      cx=x>>4; cy=y>>4;
      mm[y>>4][x>>4]=std::pair<unsigned char, unsigned char>((unsigned char)(255),(unsigned char)(0));
      for(int i=0; i<16 && i+y<img.sizey(); i++)
        for(int j=0; j<16 && j+x<img.sizex(); j++) {
          if(img[i+y][j+x]<mm[cy][cx].first)
            mm[cy][cx].first=img[i+y][j+x];
          if(img[i+y][j+x]>mm[cy][cx].second)
            mm[cy][cx].second=img[i+y][j+x];
        }
    }
  for(int y=0; y<img.sizey(); y+=16)
    for(int x=0; x<img.sizex(); x+=16) {
      cx=x>>4; cy=y>>4;
      unsigned min=mm[cy][cx].first, max=mm[cy][cx].second;
      for(int i=cy-1; i<=cy+1; i++)
        if(i>=0 && (i<<4)<img.sizey())
          for(int j=cx-1; j<=cx+1; j++)
            if(j>=0 && (j<<4)<img.sizex()) {
              if(mm[i][j].first<min) min=mm[i][j].first;
              if(mm[i][j].second>max) max=mm[i][j].second;
            }
      float a=min,b=255/(max-a);
      for(int i=0; i<16 && i+y<img.sizey(); i++)
        for(int j=0; j<16 && j+x<img.sizex(); j++) {
          img[y+i][x+j]=(unsigned char)((img[y+i][x+j]-a)*b);
        }
    }
}
