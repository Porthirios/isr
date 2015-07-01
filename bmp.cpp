#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <math.h>
#include "bmp.h"

bmpimage::bmpimage(int b, int w, int h)
{
  if(w<0) w=0; if(h<0) h=0;
  reserve1=reserve2=0;
  val1=0x28;
  width=w; height=h;
  val2=1;
  bits=b;
  size=(rsize=4*((width*bits+31)/32))*height;
  wscale=hscale=0x2e23;
  plen1=plen2=1<<bits;
  offs=sizeof(BMP_header)+plen1*4+2;
  flen=offs+size;
  palette=new palette_t[plen1];
  for(int i=0; i<plen1; i++)
    palette[i][0]=palette[i][1]=palette[i][2]=i*255/(plen1-1),palette[i][3]=0;
  data=new unsigned char*[height];
  for(int i=0; i<height; i++)
    data[i]=new unsigned char[(rsize+15)&~15];
}

bool bmpimage::load(const char* fname)
{
  struct stat st;
  if(stat(fname,&st)) return false;
  FILE* file=fopen(fname,"r");
  if(!file) return false;
  clear();
  char id[2];
  fread(id,2,1,file);
  fread((BMP_header*)this,sizeof(BMP_header),1,file);
  if(strncmp(id,"BM",2) || flen!=offs+size || flen>st.st_size) return false;
  rsize=4*((width*bits+31)/32);
  if(rsize*height!=size || offs<sizeof(BMP_header)+plen1*4+2 || plen1!=(1<<bits)) return false;
  palette=(palette_t*)realloc(palette,plen1*4);
  fread(palette,plen1,4,file);
  fseek(file,offs,SEEK_SET);
  data=(unsigned char**)malloc(height*sizeof(unsigned char*));
  for(int i=0; i<height; i++) {
    data[i]=new unsigned char[(rsize+15)&~15];
    fread(data[i],rsize,1,file);
  }
  fclose(file);
  return true;
}

bool bmpimage::save(const char* fname)
{
  FILE* file=fopen(fname,"w");
  if(!file) return false;
  fwrite(BMPID,2,1,file);
  if(fwrite((BMP_header*)this,sizeof(BMP_header),1,file)!=1) return false;
  if(fwrite(palette,4,plen1,file)!=plen1) return false;
  fseek(file,offs,SEEK_SET);
  for(int i=0; i<height; i++)
    if(fwrite(data[i],1,rsize,file)!=rsize) return false;
  fclose(file);
  return true;
}

int bmpimage::pget(int x, int y) const
{
  if(x<0 || x>=width || y<0 || y>=height) return -1;
  return ((data[y][x*bits>>3])>>(8-(x*bits&7)-bits))&(plen2-1);
}

double bmpimage::pget(double x, double y) const
{
  if(int(x)<0 || int(x)>=width || int(y)<0 || int(y)>=height) return plen1-1;
  int i=int(floor(x)), j=int(floor(y));
  if(i<0) i=0; if(i>=width) i=width-1;
  if(j<0) j=0; if(j>=height) j=height-1;
  return (pget(i,j)*(i+1-x)+pget(i+1,j)*(x-i))*(j+1-y)+(pget(i,j+1)*(i+1-x)+pget(i+1,j+1)*(x-i))*(y-j);
}

bool bmpimage::pset(int x, int y, int c)
{
  if(x<0 || x>=width || y<0 || y>=height) return false;
  if(c<0) c=0; if(c>=plen1) c=plen1-1;
  int i=x*bits>>3,j=8-(x*bits&7)-bits;
  data[y][i]&=~(plen2-1<<j);
  data[y][i]|=((c&plen2-1)<<j);
  return true;
}

void bmpimage::clear()
{
  if(data)
  for(int i=0; i<height; i++) delete data[i];
  delete data;
  data=NULL;
  height=width=0;
}

void bmpimage::resize(int w, int h)
{
  rsize=4*(((width=w)*bits+31)/32);
  while(h<height) delete data[--height];
  data=(unsigned char**)realloc(data,h*sizeof(unsigned char*));
  for(int i=0; i<height; i++)
    data[i]=(unsigned char*)realloc(data,((rsize+15)&~15)*sizeof(unsigned char));
  while(height<h)
    data[height++]=new unsigned char[(rsize+15)&~15];
}

bmpimage* rotate(bmpimage& src, double angle)
{
  double co=cos(angle), si=sin(angle);
  double x1=src.maxx()*co, y1=src.maxx()*si, x2=-src.maxy()*si, y2=src.maxy()*co, x3=x1+x2, y3=y1+y2;
  double mx=0,my=0;
  if(x1<mx) mx=x1; if(x2<mx) mx=x2; if(x3<mx) mx=x3;
  if(y1<my) my=y1; if(y2<my) my=y2; if(y3<my) my=y3;
  double w=0,h=0;
  if(x1>w) w=x1; if(x2>w) w=x2; if(x3>w) w=x3;
  if(y1>h) h=y1; if(y2>h) h=y2; if(y3>h) h=y3;
  w-=mx; h-=my;
  bmpimage* dst=new bmpimage(src.debth(),int(w)+1,int(h)+1);
  for(int i=0; i<=dst->maxx(); i++)
    for(int j=0; j<=dst->maxy(); j++)
      dst->pset(i,j,int(src.pget(int((i+mx)*co+(j+my)*si),int(-(i+mx)*si+(j+my)*co))));
  return dst;
}

void bmpimage::setcolor(int c, int r, int g, int b)
{
  palette[c][0]=r;
  palette[c][1]=g;
  palette[c][2]=b;
}

bmpimage* scale(bmpimage& src, double f) {
  bmpimage* dst=new bmpimage(src.debth(),int((src.maxx()+1)*f),int((src.maxy()+1)*f));
  for(int i=0; i<=dst->maxy(); i++)
    for(int j=0; j<=dst->maxx(); j++)
      dst->pset(j,i,int(src.pget(j/f,i/f)));
  return dst;
}

bmp24::bmp24(int w, int h, unsigned bc) {
  width=(w+3)&~3; height=h;
  reserve1=reserve2=plen1=plen2=0;
  offs=0x36; val1=0x28;
  val2=1; bits=24;
  size=width*3*height;
  wscale=hscale=0xb13;
  flen=offs+size;
  data=new unsigned char[size];
  clear(bc);
}

bool bmp24::save(const char* fname) {
  FILE* file=fopen(fname,"w");
  if(!file) return false;
  bool success=true;
  fwrite(BMPID,2,1,file);
  if(fwrite((BMP_header*)this,sizeof(BMP_header),1,file)!=1) success=false;
  else {
    fseek(file,offs,SEEK_SET);
    if(fwrite(data,size,1,file)!=1) success=false;
  }
  fclose(file);
  return success;
}

void bmp24::pset(int x, int y, unsigned c, draw_mode mode) {
  unsigned char* p=data+3*(y*width+x);
  unsigned char* pc=(unsigned char*)(&c);
  switch(mode) {
    case COPY_PUT:
      p[0]=pc[0];
      p[1]=pc[1];
      p[2]=pc[2];
      break;
    case XOR_PUT:
      p[0]^=pc[0];
      p[1]^=pc[1];
      p[2]^=pc[2];
      break;
    case OR_PUT:
      p[0]|=pc[0];
      p[1]|=pc[1];
      p[2]|=pc[2];
      break;
    case AND_PUT:
      p[0]&=pc[0];
      p[1]&=pc[1];
      p[2]&=pc[2];
  }
}

void bmp24::hline(int l, int r, int y, unsigned c, draw_mode mode) {
  for(int x=l; x<=r; x++)
    pset(x,y,c,mode);
}

void bmp24::vline(int x, int t, int b, unsigned c, draw_mode mode) {
  for(int y=t; y<=b; y++)
    pset(x,y,c,mode);
}

void bmp24::clear(unsigned bc) {
  for(int y=0; y<height; y++)
    for(int x=0; x<width; x++)
      pset(x,y,bc);
}

void bmp24::resize(int w, int h, unsigned bc) {
  w=(w+3)&~3;
  size=w*3*h;
  unsigned char* nd=new unsigned char[size];
  for(int y=0; y<height && y<h; y++)
    memcpy(nd+y*w*3,data+y*width*3,(w<width?w:width)*3);
  delete data; data=nd;
  int oldw=width, oldh=height;
  width=w; height=h;
  for(int y=0; y<oldh; y++)
    for(int x=oldw; x<width; x++)
      pset(x,y,bc);
  for(int y=oldh; y<height; y++)
    for(int x=0; x<width; x++)
      pset(x,y,bc);
  flen=offs+size;
}
