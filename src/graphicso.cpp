#include "graphicso.h"

#include "decompress.h"

#include <SDL.h>

static SDL_Surface * getSprite(unsigned char * dat, unsigned int * offset, unsigned short * palette, unsigned int scale) {

  // this variable contains 2 values: the lower 3 bit contain the number of units the sprites
  // is wide, one unit is 16 pixels wide
  // the upper 5 bits contain the occupied units. A set bit results in some pixels not zero
  // so the number in the lower 3 bits contains the number of bits that are set in the upper
  // 5 bits. This is probably done to be able to shift the complete sprite to the right
  unsigned char width = *dat; dat++;
  unsigned char height = *dat; dat++;

  // maximally a sprite can be 5 units a scale times 16 pixel wide, more is simply not possible
  // later on we could change that by checking the exact units used and calculate the width
  SDL_Surface * v = SDL_CreateRGBSurface(SDL_SRCALPHA, 80*scale, scale*height, 32, 0xff000000, 0xff0000, 0xff00, 0xff);

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

          rt.x = scale*(16*i+j);
          rt.y = scale*y;
          rt.w = scale;
          rt.h = scale;

          SDL_FillRect(v, &rt, SDL_MapRGBA(v->format, 4*r, 4*g, 4*b, s==0?0:255));
        }

        bit++;
      }
    }
  }

  *offset += 2*5*height*(width & 7) + 2;

  return v;
}

graphicsO_c::graphicsO_c(const char * path, unsigned int sc) : dataPath(path), scale(sc) {
}

unsigned int graphicsO_c::getAnimation(unsigned char * data, unsigned char anim, unsigned short * palette) {

  unsigned int offset = 0;

  // get the number of sprites for this animation
  unsigned int cnt = (int)data[offset] << 8 | data[offset+1];
  offset += 2;

  // after the cnt follow cnt values containing y-offset values used later on
  // when displaying the sprite
  unsigned int yOffset = offset;
  offset = (offset + cnt + 1) &0xFFFE;

  // now get the cnt sprites
  for (unsigned int j = 0; j < cnt; j++) {

    SDL_Surface * v = getSprite(data + offset, &offset, palette, scale);

    int ofs = ((signed char)data[yOffset+j]);

    addAnt(anim, j, scale*ofs, v);
  }

  return offset;
}

void graphicsO_c::loadGraphics(void) {

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

  /* the number of sprited for each domino type is fixed */
  snprintf(fname, 200, "%s/data/DOMINOE0.SPX", dataPath.c_str());
  unsigned int d0Size;
  unsigned char * d0 = decompress(fname, &d0Size);

  snprintf(fname, 200, "%s/data/DOMINOE1.SPX", dataPath.c_str());
  unsigned int d1Size;
  unsigned char * d1 = decompress(fname, &d1Size);


  unsigned int offset = 0;
  for (unsigned int i = 0; i < 11; i++)
    for (unsigned int j = 0; j < numDominos[i]; j++) {

      SDL_Surface * v = getSprite(d0 + offset, &offset, palette, scale);

      setDomino(i, j, SDL_DisplayFormatAlpha(v));

      SDL_FreeSurface(v);
    }

  offset = 0;
  for (unsigned int i = 11; i < 18; i++)
    for (unsigned int j = 0; j < numDominos[i]; j++) {

      SDL_Surface * v = getSprite(d1 + offset, &offset, palette, scale);

      setDomino(i, j, SDL_DisplayFormatAlpha(v));

      SDL_FreeSurface(v);
    }

  delete [] d0;
  delete [] d1;


  /* load ant sprites */
  snprintf(fname, 200, "%s/data/ANT00_16.ANX", dataPath.c_str());
  unsigned char * a0 = decompress(fname, 0);

  snprintf(fname, 200, "%s/data/ANT17_30.ANX", dataPath.c_str());
  unsigned char * a1 = decompress(fname, 0);

  snprintf(fname, 200, "%s/data/ANT31_46.ANX", dataPath.c_str());
  unsigned char * a2 = decompress(fname, 0);

  snprintf(fname, 200, "%s/data/ANT47_65.ANX", dataPath.c_str());
  unsigned char * a3 = decompress(fname, 0);

  // load images from first file
  offset = 0;
  for (unsigned int i = 0; i <= 16; i++)
    offset += getAnimation(a0+offset, i, palette);

  // load images from second file
  offset = 0;
  for (unsigned int i = 17; i <= 27; i++)
    offset += getAnimation(a1+offset, i, palette);

  // load animation 28 and 29 these animations are the same as the 2 animations before but
  // the inverse order
  for (unsigned int i = 28; i <= 29; i++)
    for (unsigned int j = 0; j < getAntImages(i-2); j++)
      addAnt(i, j, getAntOffset(i-2, getAntImages(i-2)-j-1),
                   getAnt      (i-2, getAntImages(i-2)-j-1), false);

  // load animation 30
  offset += getAnimation(a1+offset, 30, palette);

  // load images from third file

  offset = 0;
  // load animation 31
  offset += getAnimation(a2+offset, 31, palette);

  for (unsigned int i = 33; i <= 35; i++)
    offset += getAnimation(a2+offset, i, palette);

  // TODO: there is some strange copying going on here in the assembler code

  for (unsigned int i = 36; i <= 43; i++)
    offset += getAnimation(a2+offset, i, palette);

  // 44 and 45 are again copied from for animations before
  for (unsigned int i = 44; i <= 45; i++)
    for (unsigned int j = 0; j < getAntImages(i-4); j++)
      addAnt(i, j, getAntOffset(i-4, j), getAnt(i-4, j), false);

  offset += getAnimation(a2+offset, 46, palette);

  // load images from last file

  offset = 0;

  for (unsigned int i = 47; i <= 49; i++)
    offset += getAnimation(a3+offset, i, palette);

  // 50 is copied it is the last images of the animation before
  addAnt(50, 0, getAntOffset(49, getAntImages(49)-1),
                getAnt      (49, getAntImages(49)-1), false);

  for (unsigned int i = 51; i <= 65; i++)
    offset += getAnimation(a3+offset, i, palette);

  delete [] a0;
  delete [] a1;
  delete [] a2;
  delete [] a3;

  // load the images of carried cominoes

  snprintf(fname, 200, "%s/data/ANT10B29.SPX", dataPath.c_str());
  unsigned char * d3 = decompress(fname, 0);

  offset = 0;

  for (unsigned int i = 0; i < 7; i++) {
      for (unsigned int j = 0; j < 10; j++) {
          SDL_Surface * v = getSprite(d3 + offset, &offset, palette, scale);
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
          SDL_Surface * v = getSprite(d3 + offset, &offset, palette, scale);
          setCarriedDomino(i, j, SDL_DisplayFormatAlpha(v));
          SDL_FreeSurface(v);
      }
  }

  delete [] d3;
}

void graphicsO_c::loadTheme(const std::string & name) {

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

  SDL_Surface * v = SDL_CreateRGBSurface(0, 16*scale, 16*scale, 32, 0xff000000, 0xff0000, 0xff00, 0);

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

        rt.x = scale*x;
        rt.y = scale*y;
        rt.w = scale;
        rt.h = scale;

        SDL_FillRect(v, &rt, SDL_MapRGB(v->format, 4*r, 4*g, 4*b));
      }

    addBgTile(SDL_DisplayFormat(v));
  }

  SDL_FreeSurface(v);

  v = SDL_CreateRGBSurface(SDL_SRCALPHA, 16*scale, 16*scale, 32, 0xff000000, 0xff0000, 0xff00, 0xff);

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

        rt.x = scale*x;
        rt.y = scale*y;
        rt.w = scale;
        rt.h = scale;

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

signed int graphicsO_c::getCarryOffsetX(unsigned int animation, unsigned int image) { return scale*offsets[animation][2*image+0]; }
signed int graphicsO_c::getCarryOffsetY(unsigned int animation, unsigned int image) { return scale*offsets[animation][2*image+1]; }

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

signed int graphicsO_c::getMoveOffsetX(unsigned int animation, unsigned int image) { return scale*moveOffsets[animation][4*image+0]; }
signed int graphicsO_c::getMoveOffsetY(unsigned int animation, unsigned int image) { return scale*moveOffsets[animation][4*image+1]; }
signed int graphicsO_c::getMoveImage(unsigned int animation, unsigned int image) { return moveOffsets[animation][4*image+2]; }

