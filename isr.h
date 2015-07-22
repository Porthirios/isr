#include <map>
#include <vector>
#include <string>
#include "bmp.h"

float autocorr(bmpimage& img, int x, int y);
float compare(bmpimage& a, bmpimage& b);
float compare(bmpimage& a, bmpimage& b, int sx, int sy);
void blur1(bmpimage& img);
void blur2(bmpimage& img);
void blur3(bmpimage& img);
void blur4(bmpimage& img);
void runblur(bmpimage& img, const char* arg);
void brightness(bmpimage& img);
void brightness_a(bmpimage& img);
int spectrum(bmpimage& img, int (&sp)[256]);
void bmpspectrum(bmpimage& img, bmpimage& spimg);
float autostep(bmpimage& img);

struct dmc_color {
  int code;
  unsigned char red, green, blue;
  std::string name;
  unsigned color() {
    return (red<<16)|(green<<8)|blue;
  }
};

struct sign {
  dmc_color* color;
  bmpimage* sample;
};

int load_dmctab(std::map<int,dmc_color>& tab, const char* fname);
int load_signs(std::vector<sign>& tab, const char* path, float sc, const char* blur);

extern std::vector<sign> sign_tab;
extern std::map<int,dmc_color> dmc_tab;
// extern bmpimage *img_flt;
