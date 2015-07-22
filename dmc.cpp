#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "isr.h"

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
