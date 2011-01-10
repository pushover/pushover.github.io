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

    180,   1,   0, 54,
    181,  -4,   0, 54,
    182, -19,   0, 54,
    183,   3,   0, 54,
    184,   1,   0, 54,
    185,   9,   1, 54,
    186, -11,   0, 54,
    187,  16,   0, 54,
      -1 // this is the end, keep it there
  };
#endif

#ifdef CARRIED
int specialCases[] =
    //Image number
        // X-Shift
             // Y-Shift
                  // Y-CLipping
  { 10,    6,   0, 54,
    11,    6,   0, 54,
    12,    6,   0, 54,
    13,    6,   0, 54,
    14,    6,   0, 54,
    15,    6,   0, 54,
    16,    6,   0, 54,
    17,    6,   0, 54,
    18,    6,   0, 54,
    19,    6,   0, 54,
    30,    6,   0, 54,
    31,    6,   0, 54,
    32,    6,   0, 54,
    33,    6,   0, 54,
    34,    6,   0, 54,
    35,    6,   0, 54,
    36,    6,   0, 54,
    37,    6,   0, 54,
    38,    6,   0, 54,
    39,    6,   0, 54,
    50,    6,   0, 54,
    51,    6,   0, 54,
    52,    6,   0, 54,
    53,    6,   0, 54,
    54,    6,   0, 54,
    55,    6,   0, 54,
    56,    6,   0, 54,
    57,    6,   0, 54,
    58,    6,   0, 54,
    59,    6,   0, 54,

    60,   -4,  -6, 54,
    61,   -4,  -6, 54,
    62,   -4,  -6, 54,
    63,   -4,  -6, 54,
    64,   -4,  -6, 54,
    65,   -4,  -6, 54,
    66,   -4,  -6, 54,
    67,   -4,  -6, 54,
    68,   -4,  -6, 54,
    69,   -4,  -6, 54,

    70,    9,  -1, 54,
    71,    9,  -1, 54,
    72,    9,  -1, 54,
    73,    9,  -1, 54,
    74,    9,  -1, 54,
    75,    9,  -1, 54,
    76,    9,  -1, 54,
    77,    9,  -1, 54,
    78,    9,  -1, 54,
    79,    9,  -1, 54,

    80,    1,   1, 54,
    81,    1,   1, 54,
    82,    1,   1, 54,
    83,    1,   1, 54,
    84,    1,   1, 54,
    85,    1,   1, 54,
    86,    1,   1, 54,
    87,    1,   1, 54,
    88,    1,   1, 54,
    89,    1,   1, 54,

    90,    4,  -1, 54,
    91,    4,  -1, 54,
    92,    4,  -1, 54,
    93,    4,  -1, 54,
    94,    4,  -1, 54,
    95,    4,  -1, 54,
    96,    4,  -1, 54,
    97,    4,  -1, 54,
    98,    4,  -1, 54,
    99,    4,  -1, 54,

   100,    8,  -2, 54,
   101,    8,  -2, 54,
   102,    8,  -2, 54,
   103,    8,  -2, 54,
   104,    8,  -2, 54,
   105,    8,  -2, 54,
   106,    8,  -2, 54,
   107,    8,  -2, 54,
   108,    8,  -2, 54,
   109,    8,  -2, 54,

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
