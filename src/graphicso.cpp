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
  printf("anim %i has %i sprites\n", anim, cnt);

  // after the cnt follow cnt values containing y-offset values used later on
  // when displaying the sprite
  unsigned int yOffset = offset;
  offset = (offset + cnt + 1) &0xFFFE;

  // now get the cnt sprites
  for (unsigned int j = 0; j < cnt; j++) {

    SDL_Surface * v = getSprite(data + offset, &offset, palette, scale);

    int ofs = ((signed char)data[yOffset+j]);

    addAnt(anim, scale*ofs, v);
  }

  return offset;
}

void graphicsO_c::loadGraphics(void) {

  char fname[200];

  // load a palette file, it doesn't matter which because all
  // sprites loded here use only colors that are identical on all
  // palettes
  snprintf(fname, 200, "%s/themes/AZTEC.PAL", dataPath.c_str());
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
  for (unsigned int i = 12; i < 18; i++)
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
      addAnt(i, getAntOfset(i-2, getAntImages(i-2)-j-1),
                getAnt     (i-2, getAntImages(i-2)-j-1), false);

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

  // 44 and 45 are again copied from somewhere else

  offset += getAnimation(a2+offset, 46, palette);

  // load images from last file

  offset = 0;

  for (unsigned int i = 47; i <= 49; i++)
    offset += getAnimation(a3+offset, i, palette);

  // 50 is copied it is the last images of the animation before
  addAnt(50, getAntOfset(49, getAntImages(49)-1),
             getAnt     (49, getAntImages(49)-1), false);

  for (unsigned int i = 51; i <= 65; i++)
    offset += getAnimation(a3+offset, i, palette);

  delete [] a0;
  delete [] a1;
  delete [] a2;
  delete [] a3;

  // load the images of carried cominoes

}

void graphicsO_c::loadTheme(const char *name) {

  char fname[200];

  snprintf(fname, 200, "%s/themes/%s.PAL", dataPath.c_str(), name);
  unsigned short palette[32];
  {
    FILE * f = fopen(fname, "rb");
    fread(palette, 1, 64, f);
    fclose(f);
  }

  snprintf(fname, 200, "%s/themes/%s.BCX", dataPath.c_str(), name);
  unsigned int bgSize;
  unsigned char * bg = decompress(fname, &bgSize);

  snprintf(fname, 200, "%s/themes/%s.PLX", dataPath.c_str(), name);
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
