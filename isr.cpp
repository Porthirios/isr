#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
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

struct dmc_color {
  int code;
  unsigned char red, green, blue;
  std::string name;
  unsigned color() {
    return (red<<16)|(green<<8)|blue;
  }
};

int load_dmctab(std::map<int,dmc_color>& tab, const char* fname) {
  FILE* file=fopen(fname,"r");
  if(!file) return -1;
  tab.clear();
  char* line=NULL;
  static const char* delim="\t\n\r";
  size_t size=0;
  while(getline(&line,&size,file)>0) {
    char* p=strtok(line,delim);
    if(!p) continue;
    dmc_color c;
    c.code=atoi(p);
    p=strtok(NULL,delim);
    if(!p) continue;
    c.name=std::string(p);
    p=strtok(NULL,delim);
    if(!p) continue;
    c.red=atoi(p);
    p=strtok(NULL,delim);
    if(!p) continue;
    c.green=atoi(p);
    p=strtok(NULL,delim);
    if(!p) continue;
    c.blue=atoi(p);
    tab[c.code]=c;
  }
  fclose(file);
  delete line;
  return tab.size();
}

std::map<int,dmc_color> dmc_tab;

struct sign {
  dmc_color* color;
  bmpimage* sample;
};

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

int load_signs(std::vector<sign>& tab, const char* path, float sc, const char* blur) {
  DIR* dir=opendir(path);
  if(!dir) return -1;
  tab.clear();
  bmpimage img;
  sign s;
  for(dirent* ent=readdir(dir); ent; ent=readdir(dir))
  if(isdigit(ent->d_name[0]) && strcasestr(ent->d_name,".bmp")) {
    s.color = &dmc_tab[atoi(ent->d_name)];
    char name[strlen(path)+strlen(ent->d_name)+2];
    strcat(strcat(strcpy(name,path),"/"),ent->d_name);
    img.load(name);
    runblur(img,blur);
    s.sample=scale(img,sc);
    brightness(*s.sample);
    tab.push_back(s);
  }
  closedir(dir);
  return tab.size();
}

std::vector<sign> sign_tab;

void isr(bmp24& dst, bmpimage& src, int d, std::vector<int>& sp, int sb=1) {
  int w=dst.sizex()/sb, h=dst.sizey()/sb;
  float stepx=float(src.sizex())/w, stepy=float(src.sizey())/h;
  float qavg=0, qmin=1., qmax=0.;
  int unsure=0;
  sp.clear();
  sp.insert(sp.begin(),sign_tab.size(),0);
//  for(int s=0; s<sign_tab.size(); s++)
//    dst.setcolor(s+1,sign_tab[s].color->red,sign_tab[s].color->green,sign_tab[s].color->blue);
  for(int i=0; i<h; i++)
    for(int j=0; j<w; j++) {
      float cx=(j+0.5)*stepx, cy=(i+0.5)*stepy, dis=255., last=255.;
      int c=0;
      for(int s=0; s<sign_tab.size(); s++) {
        int x=int(cx-sign_tab[s].sample->sizex()/2.), y=int(cy-sign_tab[s].sample->sizey()/2.);
        float md=255.;
        for(int sy=-d; sy<=d; sy++)
          for(int sx=-d; sx<=d; sx++) {
            float cd=compare(src,*sign_tab[s].sample,x+sx,y+sy);
            if(cd<md) md=cd;
          }
        if(dis>md) {
          if(dis<last) last=dis;
          dis=md;
          c=s;
        }
      }
      dst.bar(j*sb,i*sb,(j+1)*sb-1,(i+1)*sb-1,sign_tab[c].color->color());
      sp[c]++;
      float q=1.-dis/last;
      qavg+=q;
      if(q<qmin) qmin=q;
      if(q>qmax) qmax=q;
      if(q<0.05) unsure++;
    }
  std::cerr << "Точность распознавания: min=" << qmin << "\tmax=" << qmax << "\tavg=" << qavg/(w*h) << '\n'
  << "Неуверенно распознано " << unsure << " знаков\n";
}

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

void printhelp() {
  std::cerr <<
  "ISR - Image Stitch Recognize v.0.0.3 (C) Porthirios, 2015\n"
  "Программа распознавания черно-белых схем для вышивки и перевода их в цветную картинку\n"
  "Использование: isr ключи исходный_файл.bmp\n"
  "Ключи:\n"
  "--help - этот текст\n"
  "-f float - масштаб значков (по умолчанию 0.6)\n"
  "-d int - ширина поиска значков (по умолчанию 2)\n"
  "-o file.bmp - выходной файл (по умолчанию icon.bmp)\n"
  "-w int - ширина схемы (по умолчанию 120)\n"
  "-h int - высота схемы (по умолчанию 160)\n"
  "-b digits - метод сглаживания (пока доступны 1-4; можно применять несколько подряд\n"
  "Схема должна быть предварительно кадрирована, и в текущем каталоге должна находиться \nпапка sign с образцами значков, имена файлов со значками соответствуют кодам цветов по каталогу DMC\n";
}

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

main(int argc, const char** argv) {
  float coef=0.6;
  int dis=2, w=120, h=160, s=1;
  bmpimage img;
  const char* imgname=NULL, *outname="icon.bmp", *blur="";
  if(argc<2) {
    printhelp();
    return 0;
  }
  for(int i=1; i<argc; i++)
    if(!strcmp(argv[i],"-f")) coef=atof(argv[++i]); else
    if(!strcmp(argv[i],"-d")) dis=atoi(argv[++i]); else
    if(!strcmp(argv[i],"-o")) outname=argv[++i]; else
    if(!strcmp(argv[i],"-w")) w=atoi(argv[++i]); else
    if(!strcmp(argv[i],"-h")) h=atoi(argv[++i]); else
    if(!strcmp(argv[i],"-s")) s=atoi(argv[++i]); else
    if(!strcmp(argv[i],"--help")) {
      printhelp();
      return 0;
    } else
    if(!strcmp(argv[i],"-b")) blur=argv[++i];
    else imgname=argv[i];
  int c=load_dmctab(dmc_tab,"DMC-tab.txt");
  if(c<=0) {
    std::cerr << "Не удалось загрузить таблицу цветов DMC\n";
    return 2;
  } else std::cerr << "Таблица цветов DMC " << c << "записей\n";
  c=load_signs(sign_tab,"sign",coef,blur);
  if(c<=0) {
    std::cerr << "Не удалось загрузить сигнатуру\n";
    return 3;
  } else std::cerr << "Загружено " << c << " знаков\n";
//  filter();
//  img_flt->save("filter.bmp");
  if(!imgname) {
    std::cerr << "Не указано имя файла\n";
    return 1;
  }
  img.load(imgname);
  runblur(img,blur);
  bmp24 icon(w*s,h*s);
  brightness_a(img);
  std::vector<int> sp;
  isr(icon,img,dis,sp,s);
  icon.save(outname);
  for(int c=0; c<sp.size(); c++)
    if(sp[c])
      std::cout << sign_tab[c].color->code << '\t' << sign_tab[c].color->name << '\t' << sp[c] << " ст.\n";
    return 0;
}
