#include "text.h"

#include <SDL/SDL_ttf.h>

#include <iostream>
#include <vector>

std::vector<TTF_Font * > fonts;


void initText(void) {

  if (TTF_Init() == -1) {
    std::cout << "Oops could not initialite font engine\n";
    exit(1);
  }

  TTF_Font * ft;

  ft = TTF_OpenFont("/usr/share/fonts/freefont-ttf/FreeSans.ttf", 10);
  if (!ft) {
    std::cout << "Oops can not open Font file\n";
    exit(1);
  }

  fonts.push_back(ft);

  ft = TTF_OpenFont("/usr/share/fonts/freefont-ttf/FreeSans.ttf", 15);
  if (!ft) {
    std::cout << "Oops can not open Font file\n";
    exit(1);
  }

  fonts.push_back(ft);

  ft = TTF_OpenFont("/usr/share/fonts/freefont-ttf/FreeSans.ttf", 35);
  if (!ft) {
    std::cout << "Oops can not open Font file\n";
    exit(1);
  }

  fonts.push_back(ft);
}

void deinitText(void) {

  for (unsigned int i = 0; i < fonts.size(); i++)
    TTF_CloseFont(fonts[i]);

  TTF_Quit();
}

void renderText(SDL_Surface * d, const fontParams_s * par, const std::string & t) {

  SDL_Surface * vv = TTF_RenderUTF8_Blended(fonts[par->font], t.c_str(), par->color);
  SDL_Surface * vb;

  if (par->shadow)
  {
    SDL_Color bg;
    bg.r = bg.g = bg.b = 0;
    vb = TTF_RenderUTF8_Blended(fonts[par->font], t.c_str(), bg);
  }

  SDL_Rect r = par->box;
  r.w = vv->w;
  r.h = vv->h;

  if (par->alignment == ALN_TEXT) {
    if (par->shadow)
    {
      r.x-=2; r.y-=2; SDL_BlitSurface(vb, 0, d, &r);
      r.x+=4;         SDL_BlitSurface(vb, 0, d, &r);
      r.y+=4;         SDL_BlitSurface(vb, 0, d, &r);
      r.x-=4;         SDL_BlitSurface(vb, 0, d, &r);
      r.y-=2; r.x+=2;
    }
    SDL_BlitSurface(vv, 0, d, &r);
  }
  else if (par->alignment == ALN_CENTER) {
  }

  SDL_FreeSurface(vv);
  if (par->shadow) SDL_FreeSurface(vb);
}
