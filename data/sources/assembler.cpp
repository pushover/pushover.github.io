/*
 * program to assemble a pile of little images into a bigger one to
 * calculate a common palette
 */

#include <SDL.h>
#include <SDL_image.h>

#include "pngsaver.h"

#include <vector>

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


typedef struct {
  int xs;
  int ys;
  int yclip;
  char file[200] ;
} image;

int main(int argn, char *args[]) {

  SDL_Rect r;

  r.x = 0;
  r.y = 0;

  printf("  loading images\n");

  int height = atoi(args[2]);
  int gap = atoi(args[3]);
  int outwidth = atoi(args[4]);

  FILE * in = fopen(args[5], "r");

  if (!in)
  {
    printf("could not open image file list %s\n", args[5]);
    return 1;
  }

  std::vector<image> images;

  char str[1000];

  while(fgets(str,sizeof(str),in) != NULL)
  {
    // strip trailing '\n' if it exists
    int len = strlen(str)-1;
    if(str[len] == '\n')
      str[len] = 0;

    if (str[0] != '#')
    {
      image i;

      sscanf(str, "%d %d %d %s", &i.xs, &i.ys, &i.yclip, i.file);

      images.push_back(i);
    }
  }

  int num_images = images.size();

  out = SDL_CreateRGBSurface(0, outwidth, (height+gap) * num_images, 32, 0xff000000, 0xff0000, 0xff00, 0xff);

  for (size_t i = 0; i < images.size(); i++) {
    printf("loading %s\n", images[i].file);

    inp = IMG_LoadPNG_RW(SDL_RWFromFile(images[i].file, "rb"));
    SDL_SetAlpha(inp, 0, 0);

    r.w = inp->w;
    r.h = inp->h;

    r.x = images[i].xs;

    int crop = height - images[i].yclip;

    SDL_Rect r2;

    r2.x = 0;
    r2.y = crop-images[i].ys;
    r2.w = inp->w;
    r2.h = inp->h-r2.y;

    int yyy = r.y;
    r.y += crop;
    SDL_BlitSurface(inp, &r2, out, &r);
    r.y = yyy;

    SDL_FreeSurface(inp);

    r.y += height;
    r.w = outwidth;
    r.x = 0;
    r.h = gap;

    SDL_FillRect(out, &r, SDL_MapRGB(out->format, 0, 0, 0));

    printf("%i %i %i %s\n", images[i].xs, images[i].ys, images[i].yclip, images[i].file);

    r.y += gap;
  }

  char s[500];

  printf("  saving\n");

  sprintf(s, "%s", args[1]);
  printf(" into %s\n", s);
  SavePNGImage(s, out);

  return 0;
}
