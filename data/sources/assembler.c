/*
 * program to assemble a pile of little images into a bigger one to
 * calculate a common palette
 */

#include <SDL.h>
#include <SDL_image.h>

#include "pngsaver.h"

SDL_Surface *out;
SDL_Surface *inp;

/*
 * first argument: horizontal or vertial (h/v/hm/vm)
 * second argument: output filename
 * 3rd- argument: input filenames
 *
 * the first halve of the images must be the color images and the second halve
 * the mask images
 *
 * all images should have same size otherwise strange things
 * may happen
 */

// for the raiser images we need special placement of each and every image
// this array defines those positions first entry is the image number, then a relative x and y shift, relative to the normal position

#ifdef DOMINOS

#define SPECIALSTART 172

int specialCases[] =
    //Image number
        // X-Shift
             // Y-Shift
                  // Y-CLipping
  { 127,   0, -32, 36,
    128, -40, -24, 36,
    129, -31, -10, 36,
    130, -23,  -4, 36,
    131, -10,   0, 36,
    132,  -4,   0, 36,  // wrong image
    133,  -2,   0, 36,
    134,   0,  -1, 54,
    135,   2,   0, 36,
    136,   4,   0, 36,
    137,  10,   0, 36,
    138,  23,  -4, 36,
    139,  31, -10, 36,
    140,  40, -24, 36,
    141,   0, -32, 36,
    142,   0,   0, 36,
    143,   0,   0, 42,

    SPECIALSTART+53,   1,   0, 54,
    SPECIALSTART+54,  -4,   0, 54,
    SPECIALSTART+55, -19,   0, 54,
    SPECIALSTART+56,   3,   0, 54,
    SPECIALSTART+57,   1,   0, 54,
    SPECIALSTART+58,   9,   1, 54,
    SPECIALSTART+59, -11,   0, 54,
    SPECIALSTART+60,  16,   0, 54,
      -1 // this is the end, keep it there
  };
#endif

#ifdef CARRIED

#define DS(x, y) (15*(x)+(y))  // calculate domino number y out of set x

#define DOM(x, sx, sy) \
    DS((x), 0), (sx), (sy), 54, \
    DS((x), 1), (sx), (sy), 54, \
    DS((x), 2), (sx), (sy), 54, \
    DS((x), 3), (sx), (sy), 54, \
    DS((x), 4), (sx), (sy), 54, \
    DS((x), 5), (sx), (sy), 54, \
    DS((x), 6), (sx), (sy), 54, \
    DS((x), 7), (sx), (sy), 54, \
    DS((x), 8), (sx), (sy), 54, \
    DS((x), 9), (sx), (sy), 54, \
    DS((x),10), (sx), (sy), 54, \
    DS((x),11), (sx), (sy), 54,

int specialCases[] =
    //Image number
        // X-Shift
             // Y-Shift
                  // Y-CLipping
  {
    DOM( 1,  6,  0)
    DOM( 3,  6,  0)
    DOM( 3,  5,  0)
    DOM( 6, -4, -6)
    DOM( 7,  9, -1)
    DOM( 8,  1,  1)
    DOM( 9,  4, -1)
    DOM(10,  8, -2)
      -1 // this is the end, keep it there
  };
#endif


int main(int argn, char *args[]) {

  if (argn < 4)
    return 0;

  SDL_Rect r;

  r.x = 0;
  r.y = 0;

  printf("  loading images\n");

  int height = atoi(args[2]);
  int gap = atoi(args[3]);
  int shiftX = atoi(args[4]);
  int shiftY = atoi(args[5]);
  int outwidth = atoi(args[6]);

  int imageStart = 7;
  int arg = imageStart;
  int images = argn - imageStart;
  int addYShift;

  while (arg < argn) {
    printf("loading %s\n", args[arg]);

    inp = IMG_LoadPNG_RW(SDL_RWFromFile(args[arg], "rb"));
    SDL_SetAlpha(inp, 0, 0);

    if (arg == imageStart) {
      out = SDL_CreateRGBSurface(0, outwidth, (height+gap) * images, 32, 0xff000000, 0xff0000, 0xff00, 0xff);
    }

    int sp = 0;
    while (specialCases[4*sp] != -1)
    {
      if ((arg-imageStart) == specialCases[4*sp])
      {
        r.w = inp->w;
        r.h = inp->h;

        r.x = shiftX + specialCases[4*sp+1];

        int crop = height - specialCases[4*sp+3];

        SDL_Rect r2;

        r2.x = 0;
        r2.y = crop-specialCases[4*sp+2]-shiftY;
        r2.w = inp->w;
        r2.h = inp->h-r2.y;

        int yyy = r.y;
        r.y += crop;
        SDL_BlitSurface(inp, &r2, out, &r);
        r.y = yyy;

        break;
      }
      sp++;
    }

    if (specialCases[4*sp] == -1)
    {
      r.w = inp->w;
      r.h = inp->h;
      r.x = shiftX;

      r.y += shiftY;
      SDL_BlitSurface(inp, NULL, out, &r);
      r.y -= shiftY;
    }

    SDL_FreeSurface(inp);

    r.y += height;
    r.w = outwidth;
    r.x = 0;
    r.h = gap;

    SDL_FillRect(out, &r, SDL_MapRGB(out->format, 0, 0, 0));

    r.y += gap;

    arg++;
  }


  char s[500];

  printf("  saving\n");

  sprintf(s, "%s", args[1]);
  printf(" into %s\n", s);
  SavePNGImage(s, out);

  return 0;
}
