#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include "isr.h"

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

void printhelp() {
  std::cerr <<
  "ISR - Image Stitch Recognize v.0.0.4 (C) Porthirios, 2015\n"
  "Программа распознавания черно-белых схем для вышивки и перевода их в цветную картинку\n"
  "Использование: isr ключи исходный_файл.bmp\n"
  "Ключи:\n"
  "--help - этот текст\n"
  "-f float - масштаб значков (по умолчанию 0.6)\n"
  "-d int - ширина поиска значков (по умолчанию 2)\n"
  "-o file.bmp - выходной файл (по умолчанию icon.bmp)\n"
  "-w int - ширина схемы (если не задано - будет вычислено по автокорреляции)\n"
  "-h int - высота схемы (если не задано - будет вычислено по автокорреляции)\n"
  "-b digits - метод сглаживания (пока доступны 1-4; можно применять несколько подряд\n"
  "-s int - выводить каждый стежок как квадрат указанных размеров (по умолчанию 1)\n"
  "-p path - указать путь к каталогу с образцами значков (по умолчанию 'sign')\n"
  "Схема должна быть предварительно кадрирована, и в указанном каталоге должна находиться \nпапка с образцами значков, имена файлов со значками соответствуют кодам цветов по каталогу DMC\n";
}

main(int argc, const char** argv) {
  float coef=0.6;
  int dis=2, w=0, h=0, s=1;
  bmpimage img;
  const char* imgname=NULL, *outname="icon.bmp", *blur="3", *path="sign";
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
    if(!strcmp(argv[i],"-p")) path=argv[++i]; else
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
  } else std::cerr << "Таблица цветов DMC " << c << " записей\n";
  c=load_signs(sign_tab,path,coef,blur);
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
  brightness_a(img);
  if(!w || !h) {
    float a=autostep(img);
    if(!w) w=img.sizex()/a;
    if(!h) h=img.sizey()/a;
  }
  bmp24 icon(w*s,h*s);
  std::vector<int> sp;
  isr(icon,img,dis,sp,s);
  icon.save(outname);
  for(int c=0; c<sp.size(); c++)
    if(sp[c])
      std::cout << sign_tab[c].color->code << '\t' << sign_tab[c].color->name << '\t' << sp[c] << " ст.\n";
    return 0;
}
