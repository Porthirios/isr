#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "isr.h"

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
