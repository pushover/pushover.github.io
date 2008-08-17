#include "graphicso.h"

#include "decompress.h"

#include <SDL.h>

static SDL_Surface * getSprite(unsigned char * dat, unsigned int * offset, unsigned short * palette, unsigned int scale) {

  unsigned char width;
  unsigned char height;

  width = *dat; dat++;
  height = *dat; dat++;

  SDL_Surface * v = SDL_CreateRGBSurface(SDL_SRCALPHA, 160*scale, scale*height, 32, 0xff000000, 0xff0000, 0xff00, 0xff);

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

void graphicsO_c::loadGraphics(void) {

  /* load domino sprites */

  /* the number of sprited for each domino type is fixed */
  char fname[200];

  snprintf(fname, 200, "%s/data/DOMINOE0.SPX", dataPath.c_str());
  unsigned int d0Size;
  unsigned char * d0 = decompress(fname, &d0Size);

  snprintf(fname, 200, "%s/data/DOMINOE1.SPX", dataPath.c_str());
  unsigned int d1Size;
  unsigned char * d1 = decompress(fname, &d1Size);

  snprintf(fname, 200, "%s/themes/AZTEC.PAL", dataPath.c_str());
  unsigned short palette[32];
  {
    FILE * f = fopen(fname, "rb");
    fread(palette, 1, 64, f);
    fclose(f);
  }

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
