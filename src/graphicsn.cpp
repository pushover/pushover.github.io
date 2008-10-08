#include "graphicsn.h"

#include "decompress.h"
#include "pngloader.h"

#include <SDL.h>

static SDL_Surface * getSprite(unsigned char * dat, unsigned int * offset, unsigned short * palette) {

  // this variable contains 2 values: the lower 3 bit contain the number of units the sprites
  // is wide, one unit is 16 pixels wide
  // the upper 5 bits contain the occupied units. A set bit results in some pixels not zero
  // so the number in the lower 3 bits contains the number of bits that are set in the upper
  // 5 bits. This is probably done to be able to shift the complete sprite to the right
  unsigned char width = *dat; dat++;
  unsigned char height = *dat; dat++;

  // maximally a sprite can be 5 units a scale times 16 pixel wide, more is simply not possible
  // later on we could change that by checking the exact units used and calculate the width
  SDL_Surface * v = SDL_CreateRGBSurface(0, 80*5/2, 3*height, 32, 0xff000000, 0xff0000, 0xff00, 0xff);

  for (unsigned int y = 0; y < height; y++) {

    unsigned int bit = 0;

    for (int i = 0; i < 5; i++) {

      if ((width & (0x80 >> i)) != 0) {

        for (int j = 0; j < 16; j++) {

          unsigned char s = 0;

          for (int k = 0; k < 5; k++) {

            unsigned int pos = k*height*2*(width & 7) + y*2*(width & 7) + j/8 + bit*2;

            s >>= 1;
            if (dat[pos] & 0x80) s |= 0x80;
            dat[pos] <<= 1;
          }

          s >>= 3;

          unsigned char r = ((palette[s] >>  0) & 0xF);
          unsigned char g = ((palette[s] >> 12) & 0xF);
          unsigned char b = ((palette[s] >>  8) & 0xF);

          r = (r << 2) | r;
          g = (g << 2) | g;
          b = (b << 2) | b;

          SDL_Rect rt;

          rt.x = 5*(16*i+j)/2;
          rt.y = 3*y;
          rt.w = 3;
          rt.h = 3;

          SDL_FillRect(v, &rt, SDL_MapRGBA(v->format, 4*r, 4*g, 4*b, s==0?0:255));
        }

        bit++;
      }
    }
  }

  *offset += 2*5*height*(width & 7) + 2;

  return v;
}

graphicsN_c::graphicsN_c(const char * path) : dataPath(path) {
}

void graphicsN_c::getAnimation(int anim, SDL_Surface * v, pngLoader_c * png) {

  for (unsigned int j = 0; j < getAntImages(anim); j++) {

    png->getPart(v);

    signed char ofs = (signed char)((*((uint32_t*)v->pixels) & v->format->Rmask) >> v->format->Rshift);

    SDL_Rect dst;

    dst.x = 0;
    dst.y = 0;
    dst.w = 1;
    dst.h = 75;

    SDL_FillRect(v, &dst, SDL_MapRGBA(v->format, 0, 0, 0, 0));

    addAnt(anim, j, ofs, SDL_DisplayFormatAlpha(v));
  }
}

void graphicsN_c::loadGraphics(void) {

  char fname[200];

  // load a palette file, it doesn't matter which because all
  // sprites loded here use only colors that are identical on all
  // palettes
  snprintf(fname, 200, "%s/themes/SPACE.PAL", dataPath.c_str());
  unsigned short palette[32];
  {
    FILE * f = fopen(fname, "rb");
    fread(palette, 1, 64, f);
    fclose(f);
  }

  /* load domino sprites */

  /* the number of sprites for each domino type is fixed */

  // all domino sprites are in a png image load the image and then copy
  // the information to the destination sprites

  {
    pngLoader_c png(dataPath+"/data/dominos.png");

    SDL_Surface * v = SDL_CreateRGBSurface(0, png.getWidth(), 58, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
    SDL_SetAlpha(v, SDL_SRCALPHA | SDL_RLEACCEL, 0);

    for (unsigned int i = 0; i < 17; i++)
      for (unsigned int j = 0; j < numDominos[i]; j++) {

        png.getPart(v);
        setDomino(i, j, SDL_DisplayFormatAlpha(v));
        png.skipLines(2);

      }

    SDL_FreeSurface(v);
  }

  // load the ant images

  {
    pngLoader_c png(dataPath+"/data/ant.png");
    png.skipLines(15);

    SDL_Surface * v = SDL_CreateRGBSurface(0, png.getWidth(), 75, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
    SDL_SetAlpha(v, SDL_SRCALPHA | SDL_RLEACCEL, 0);

    // load images from first file
    for (unsigned int i = 0; i <= 27; i++)
      getAnimation(i, v, &png);

    // load animation 28 and 29 these animations are the same as the 2 animations before but
    // the inverse order
    for (unsigned int i = 28; i <= 29; i++)
      for (unsigned int j = 0; j < getAntImages(i-2); j++)
        addAnt(i, j, getAntOffset(i-2, getAntImages(i-2)-j-1),
            getAnt      (i-2, getAntImages(i-2)-j-1), false);

    for (unsigned int i = 30; i <= 43; i++)
      getAnimation(i, v, &png);

    // 44 and 45 are again copied from for animations before
    for (unsigned int i = 44; i <= 45; i++)
      for (unsigned int j = 0; j < getAntImages(i-4); j++)
        addAnt(i, j, getAntOffset(i-4, j), getAnt(i-4, j), false);

    for (unsigned int i = 46; i <= 49; i++)
      getAnimation(i, v, &png);

    // 50 is copied it is the last images of the animation before
    addAnt(50, 0, getAntOffset(49, getAntImages(49)-1),
        getAnt      (49, getAntImages(49)-1), false);

    for (unsigned int i = 51; i <= 65; i++)
      getAnimation(i, v, &png);

    SDL_FreeSurface(v);
  }

  // load the images of carried cominoes
  unsigned int offset = 0;

  snprintf(fname, 200, "%s/data/ANT10B29.SPX", dataPath.c_str());
  unsigned char * d3 = decompress(fname, 0);

  offset = 0;

  for (unsigned int i = 0; i < 7; i++) {
      for (unsigned int j = 0; j < 10; j++) {
          SDL_Surface * v = getSprite(d3 + offset, &offset, palette);
          setCarriedDomino(i, j, SDL_DisplayFormatAlpha(v));
          SDL_FreeSurface(v);
      }
  }

  // copy some surfaces surfaces
  for (unsigned int i = 7; i < 10; i++) {
      for (unsigned int j = 0; j < 10; j++) {
        setCarriedDomino(i, j, SDL_DisplayFormatAlpha(getCarriedDomino(i-1, j)));
      }
  }

  for (unsigned int i = 10; i < 12; i++) {
      for (unsigned int j = 0; j < 10; j++) {
        setCarriedDomino(i, j, SDL_DisplayFormatAlpha(getCarriedDomino(i & 1, j)));
      }
  }

  // load the final surfaces
  for (unsigned int i = 12; i < 16; i++) {
      for (unsigned int j = 0; j < 10; j++) {
          SDL_Surface * v = getSprite(d3 + offset, &offset, palette);
          setCarriedDomino(i, j, SDL_DisplayFormatAlpha(v));
          SDL_FreeSurface(v);
      }
  }

  delete [] d3;

  // generate the big font

  static Uint8 fnt1[] = {
    0x78,0xFC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xFC,0x78,
    0x18,0x38,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
    0x78,0xFC,0xCC,0x0C,0x18,0x18,0x30,0x60,0x60,0xC0,0xFC,0xFC,
    0x78,0xFC,0xCC,0x0C,0x38,0x38,0x0C,0x0C,0x0C,0xCC,0xFC,0x78,
    0x0C,0x1C,0x3C,0x6C,0xCC,0xFE,0xFE,0x0C,0x0C,0x0C,0x0C,0x0C,
    0xFC,0xFC,0xC0,0xC0,0xF8,0xFC,0x0C,0x0C,0x0C,0x8C,0xFC,0x78,
    0x38,0x78,0xC0,0xC0,0xF8,0xFC,0xCC,0xCC,0xCC,0xCC,0xFC,0x78,
    0xFC,0xFC,0x0C,0x18,0x18,0x30,0x30,0x30,0x60,0x60,0x60,0x60,
    0x78,0xFC,0xCC,0xCC,0x78,0x78,0xCC,0xCC,0xCC,0xCC,0xFC,0x78,
    0x78,0xFC,0xCC,0xCC,0xCC,0xFC,0x7C,0x0C,0x0C,0x8C,0xFC,0x78,
    0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x18,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  };

  static wchar_t fnt1c[] = L"0123456789: ";

  for (int c = 0; c < 12; c++) {

    SDL_Surface * v = SDL_CreateRGBSurface(SDL_SRCCOLORKEY, 5*8/2, 3*12, 8, 0, 0, 0, 0);
    SDL_SetColorKey(v, SDL_SRCCOLORKEY, 0);

    for (int y = 0; y < 12; y++) {
      for (int x = 0; x < 8; x++) {

        SDL_Rect r;
        r.x = 5*x/2;
        r.y = 3*y;

        r.w = r.h = 3;

        if ((fnt1[12*c+y] << x) & 0x80)
        {
          SDL_FillRect(v, &r, 1);
        }
        else
        {
          SDL_FillRect(v, &r, 0);
        }
      }
    }
    addBigGlyph(fnt1c[c], v);
  }
}

void graphicsN_c::loadTheme(const std::string & name) {

  char fname[200];

  snprintf(fname, 200, "%s/themes/%s.PAL", dataPath.c_str(), name.c_str());
  unsigned short palette[32];
  {
    FILE * f = fopen(fname, "rb");
    fread(palette, 1, 64, f);
    fclose(f);
  }

  snprintf(fname, 200, "%s/themes/%s.BCX", dataPath.c_str(), name.c_str());
  unsigned int bgSize;
  unsigned char * bg = decompress(fname, &bgSize);

  snprintf(fname, 200, "%s/themes/%s.PLX", dataPath.c_str(), name.c_str());
  unsigned int fgSize;
  unsigned char * fg = decompress(fname, &fgSize);

  unsigned int bgTiles = bgSize / 160;
  unsigned int fgTiles = fgSize / 160;

  SDL_Surface * v = SDL_CreateRGBSurface(0, 16*5/2, 16*3, 32, 0xff000000, 0xff0000, 0xff00, 0);

  for (unsigned int tile = 0; tile < bgTiles; tile++) {

    for (unsigned int y = 0; y < 16; y++)
      for (unsigned int x = 0; x < 16; x++) {

        unsigned int idx = 0;

        for (unsigned int i = 0; i < 5; i++) {

          idx >>= 1;
          idx |= ((bg[tile*160 + 2*y + x/8 + 32*i] << (x%8)) & 0x80);
        }

        idx >>= 3;

        unsigned char r = ((palette[idx] >>  0) & 0xF);
        unsigned char g = ((palette[idx] >> 12) & 0xF);
        unsigned char b = ((palette[idx] >>  8) & 0xF);

        r = (r << 2) | r;
        g = (g << 2) | g;
        b = (b << 2) | b;

        SDL_Rect rt;

        rt.x = 5*x/2;
        rt.y = 3*y;
        rt.w = 3;
        rt.h = 3;

        SDL_FillRect(v, &rt, SDL_MapRGB(v->format, 4*r, 4*g, 4*b));
      }

    addBgTile(SDL_DisplayFormat(v));
  }

  SDL_FreeSurface(v);

  v = SDL_CreateRGBSurface(SDL_SRCALPHA, 16*5/2, 16*3, 32, 0xff000000, 0xff0000, 0xff00, 0xff);

  for (unsigned int tile = 0; tile < fgTiles; tile++) {

    for (unsigned int y = 0; y < 16; y++)
      for (unsigned int x = 0; x < 16; x++) {

        unsigned int idx = 0;

        for (unsigned int i = 0; i < 5; i++) {

          idx >>= 1;
          idx |= ((fg[tile*160 + 2*y + x/8 + 32*i] << (x%8)) & 0x80);
        }

        idx >>= 3;

        unsigned char r = ((palette[idx] >>  0) & 0xF);
        unsigned char g = ((palette[idx] >> 12) & 0xF);
        unsigned char b = ((palette[idx] >>  8) & 0xF);

        r = (r << 2) | r;
        g = (g << 2) | g;
        b = (b << 2) | b;

        SDL_Rect rt;

        rt.x = 5*x/2;
        rt.y = 3*y;
        rt.w = 3;
        rt.h = 3;

        SDL_FillRect(v, &rt, SDL_MapRGBA(v->format, 4*r, 4*g, 4*b, idx==0?0:255));
      }

    addFgTile(SDL_DisplayFormatAlpha(v));
  }

  SDL_FreeSurface(v);

  delete [] bg;
  delete [] fg;

}


static signed int offsets[12][16] = {
  {     -7, -3,  -8, -3, -11, -3, -14, -3, -16, -3, -20, -3,
  }, {   5, -3,   6, -3,   9, -3,  12, -3,  14, -3,  18, -3,
  }, {  -4, -3,  -6, -5,  -7, -7, -11, -5, -16, -4, -20, -3,
  }, {   2, -3,   4, -5,   5, -7,   9, -5,  14, -4,  18, -3,
  }, { -12, -3, -16, -4, -19, -1, -19,  0, -19,  0, -20, -1,
  }, {   9, -3,  13, -4,  16, -1,  16,  0,  16,  0,  17, -1,
  }, {  -8, -3,  -8, -2,  -7, -3,  -7, -2,  -8, -3,  -8, -2, -7, -3, -7, -2,
  }, {  -8, -3,  -8, -2,  -7, -3,  -7, -2,  -8, -3,  -8, -2, -7, -3, -7, -2,
  }, {  -7, -2,  -7, -3,  -8, -2,  -8, -3,  -7, -2,  -7, -3, -8, -2, -8, -3,
  }, {  -7, -2,  -7, -3,  -8, -2,  -8, -3,  -7, -2,  -7, -3, -8, -2, -8, -3,
  }, {  -5, -3,
  }, {   5, -3,
  }
};

signed int graphicsN_c::getCarryOffsetX(unsigned int animation, unsigned int image) { return 5*offsets[animation][2*image+0]/2; }
signed int graphicsN_c::getCarryOffsetY(unsigned int animation, unsigned int image) { return 3*offsets[animation][2*image+1]; }

static signed int moveOffsets[10][64] = {
  {    0, -2, 7, 0, 0, -2, 7, 0, 0, -2, 7, 0, -1, -1, 7, 0, -2, 0, 7, 0, -2, 0, 7, 0, -2, 0, 7, 0, -2, 0, 7, 0,
       -2, 0, 7, 0, -2, 0, 7, 0, -2, 0, 7, 0, -2, -1, 9, 0, 0, -3, 32, 0, -1, -3, 32, 0, -4, -3, 32, 0,
  }, { 0, -2, 7, 0, 0, -2, 7, 0, 0, -2, 7, 0, -1, -1, 7, 0, -2, 0, 7, 0, -2, 0, 7, 0, -2, 0, 7, 0, -2, 0, 7, 0,
       -2, 0, 7, 0, -2, 0, 7, 0, -2, 0, 7, 0, -2, -1, 5, 0, -3, -3, 33, 0, -2, -3, 33, 0, -1, -3, 33, 0,
  }, { -7, -3, 32, 0, -8, -3, 32, 0, -11, -3, 32, 0, -14, -3, 32, 0, -17, -3, 32, 0, -17, -1, 32, 0, -18, 0, 9, 0,
       -18, 0, 7, 0, -18, 0, 7, 0, -18, 0, 7, 0, -18, 0, 7, 0, -18, 0, 7, 0, -17, -1, 7, 0, -16, -2, 7, 0,
       -16, -2, 7, 0, -16, -2, 7, 0,
  }, { 5, -3, 33, 0, 6, -3, 33, 0, 10, -2, 33, 0, 11, -3, 33, 0, 14, -3, 33, 0, 14, -1, 33, 0, 14, 0, 5, 0,
       14, 0, 7, 0, 14, 0, 7, 0, 14, 0, 7, 0, 14, 0, 7, 0, 14, 0, 7, 0, 15, -1, 7, 0, 16, -2, 7, 0,
       16, -2, 7, 0, 16, -2, 7, 0,
  }, { 0, -3, 44, 0, 0, -3, 45, 0,
  }, { -3, -3, 46, 0, -3, -3, 47, 0,
  }, { 0, -3, 45, 0, 0, -3, 44, 0,
  }, { -3, -3, 47, 0, -3, -3, 46, 0,
  }, { 16, -3, 7, 0, 16, -3, 7, 0, 16, -3, 7, 0, 16, -3, 7, 0,
  }, { -7, -3, 32, 0, -8, -3, 32, 0, -11, -3, 32, 0, -14, -3, 32, 0, 0, -3, 7, 0, 0, -3, 7, 0, 0, -3, 7, 0, 0, -3, 7, 0,
  }
};

signed int graphicsN_c::getMoveOffsetX(unsigned int animation, unsigned int image) { return 5*moveOffsets[animation][4*image+0]/2; }
signed int graphicsN_c::getMoveOffsetY(unsigned int animation, unsigned int image) { return 3*moveOffsets[animation][4*image+1]; }
signed int graphicsN_c::getMoveImage(unsigned int animation, unsigned int image) { return moveOffsets[animation][4*image+2]; }

