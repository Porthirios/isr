#ifndef _BMP_H
#define _BMP_H

typedef unsigned char palette_t[4];

#define BMPID ("BM")

struct BMP_header
{
  long flen,reserve1,offs,val1,width,height;
  short val2, bits;
  long reserve2, size, wscale,hscale,plen1,plen2;
};

class bmpimage : BMP_header
{
  int rsize;
  palette_t *palette;
  unsigned char** data;
public:
  bmpimage(int b=1, int w=0, int h=0);
  ~bmpimage()
  {
    clear();
    delete palette;
  }
  bool load(const char* fname);
  bool save(const char* fname);
  int pget(int x, int y) const;
  double pget(double x, double y) const;
  bool pset(int x, int y, int c);
  void clear();
  void resize(int w, int h);
  int maxx() const
  { return width-1; }
  int maxy() const
  { return height-1; }
  int debth() const
  { return bits; }
  int sizex() { return width; }
  int sizey() { return height; }
  void setcolor(int,int,int,int);
  unsigned char* operator[](int n) {
    return data[n];
  }
  void bar(int x1, int y1, int x2, int y2, unsigned c);
};

enum draw_mode { COPY_PUT=0, XOR_PUT=1, OR_PUT=2, AND_PUT=3 };

class bmp24 : BMP_header
{
  unsigned char* data;
public:
  bmp24(int w, int h, unsigned bc=0);
  ~bmp24() { delete data; }
  unsigned char* operator[](int n) {
    return data+(height-n-1)*3*width;
  }
  bool save(const char* fname);
  void pset(int x, int y, unsigned c, draw_mode mode=COPY_PUT);
  void hline(int l, int r, int y, unsigned c, draw_mode mode=COPY_PUT);
  void vline(int x, int t, int b, unsigned c, draw_mode mode=COPY_PUT);
  void bar(int x1, int y1, int x2, int y2, unsigned c);
  void resize(int w, int h, unsigned bc);
  void clear(unsigned bc);
  int maxx() const
  { return width-1; }
  int maxy() const
  { return height-1; }
  int debth() const
  { return bits; }
  int sizex() { return width; }
  int sizey() { return height; }
};

bmpimage* rotate(bmpimage& src, double angle);
bmpimage* scale(bmpimage& src, double f);

#endif
