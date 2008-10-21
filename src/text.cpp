#include "text.h"

#include <SDL/SDL_ttf.h>

#include <iostream>
#include <vector>

std::vector<TTF_Font * > fonts;


void initText(std::string dir) {

  if (TTF_Init() == -1) {
    std::cout << "Oops could not initialite font engine\n";
    exit(1);
  }

  TTF_Font * ft;

  ft = TTF_OpenFont((dir+"/data/FreeSans.ttf").c_str(), 20);
  if (!ft) {
    std::cout << "Oops can not open Font file\n";
    exit(1);
  }

  fonts.push_back(ft);

  ft = TTF_OpenFont((dir+"/data/FreeSans.ttf").c_str(), 30);
  if (!ft) {
    std::cout << "Oops can not open Font file\n";
    exit(1);
  }

  fonts.push_back(ft);

  ft = TTF_OpenFont((dir+"/data/FreeSans.ttf").c_str(), 35);
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


std::vector<std::string> split(const std::string & text, char splitter)
{
  std::string current;
  std::vector<std::string> res;

  for (unsigned int i = 0; i < text.length(); i++)
  {
    if (text[i] == splitter)
    {
      res.push_back(current);
      current = "";
    }
    else
    {
      current += text[i];
    }
  }

  if (current.length()) res.push_back(current);

  return res;
}

void renderText(SDL_Surface * d, const fontParams_s * par, const std::string & t) {

  std::vector<std::string> words = split(t, ' ');

  int ypos = par->box.y;

  if (par->alignment == ALN_CENTER)
  {
    unsigned int word = 0;
    int height = 0;

    while (word < words.size()) {

      std::string curLine = words[word];
      word++;

      while (word < words.size())
      {
        int w;
        TTF_SizeUTF8(fonts[par->font], (curLine+words[word]).c_str(), &w, 0);

        if (w > par->box.w) break;

        curLine = curLine + " " + words[word];
        word++;
      }

      int h, w;
      TTF_SizeUTF8(fonts[par->font], curLine.c_str(), &w, &h);

      height += h;
    }

    ypos += (par->box.h-height)/2;
  }

  unsigned int word = 0;

  while (word < words.size()) {

    std::string curLine = words[word];
    word++;

    while (word < words.size())
    {
      int w;
      TTF_SizeUTF8(fonts[par->font], (curLine+words[word]).c_str(), &w, 0);

      if (w > par->box.w) break;

      curLine = curLine + " " + words[word];
      word++;
    }

    SDL_Surface * vv = TTF_RenderUTF8_Blended(fonts[par->font], curLine.c_str(), par->color);
    SDL_Surface * vb = NULL;

    if (par->shadow)
    {
      SDL_Color bg;
      bg.r = bg.g = bg.b = 0;
      vb = TTF_RenderUTF8_Blended(fonts[par->font], curLine.c_str(), bg);
    }

    if (par->alignment == ALN_TEXT) {

      SDL_Rect r = par->box;
      r.w = vv->w;
      r.h = vv->h;
      r.y = ypos;

      if (par->shadow)
      {
        int sa = 1;
        if (par->font == FNT_BIG) sa = 2;

        r.x-=sa; r.y-=sa; SDL_BlitSurface(vb, 0, d, &r);
        r.x+=2*sa;        SDL_BlitSurface(vb, 0, d, &r);
        r.y+=2*sa;        SDL_BlitSurface(vb, 0, d, &r);
        r.x-=2*sa;        SDL_BlitSurface(vb, 0, d, &r);
        r.y-=sa; r.x+=sa;
      }
      SDL_BlitSurface(vv, 0, d, &r);
    }
    else if (par->alignment == ALN_TEXT_CENTER || par->alignment == ALN_CENTER) {

      SDL_Rect r;

      r.x = par->box.x + (par->box.w - vv->w)/2;
      r.y = ypos;

      r.w = vv->w;
      r.h = vv->h;

      if (par->shadow)
      {
        int sa = 1;
        if (par->font == FNT_BIG) sa = 2;

        r.x-=sa; r.y-=sa; SDL_BlitSurface(vb, 0, d, &r);
        r.x+=2*sa;        SDL_BlitSurface(vb, 0, d, &r);
        r.y+=2*sa;        SDL_BlitSurface(vb, 0, d, &r);
        r.x-=2*sa;        SDL_BlitSurface(vb, 0, d, &r);
        r.y-=sa; r.x+=sa;
      }
      SDL_BlitSurface(vv, 0, d, &r);
    }

    ypos += vv->h;

    SDL_FreeSurface(vv);
    if (par->shadow) SDL_FreeSurface(vb);
  }
}
