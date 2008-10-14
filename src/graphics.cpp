#include "graphics.h"

const unsigned char graphics_c::numAntAnimations = 66;
const unsigned char numAntAnimationsImages[graphics_c::numAntAnimations] = {

 6, 6, 6, 6, 4, 4, 8, 8, 8, 8, 6, 6, 6, 6, 6,
 6, 8, 8, 8, 8, 1, 1, 15, 15, 16, 16, 2, 2, 2,
 2, 13, 17, 0, 1, 2, 6, 7, 7, 12, 12, 8, 8, 4,
 4, 8, 8, 6, 6, 4, 3, 1, 15, 2, 4, 7, 3, 7, 4,
 11, 11, 8, 8, 8, 1, 1, 13

};

graphics_c::graphics_c(void) {

  dominos.resize(numDominoTypes);
  carriedDominos.resize(numDominoTypes);

  for (unsigned int i = 0; i < numDominoTypes; i++) {
    dominos[i].resize(numDominos[i]);
    carriedDominos[i].resize(10);
  }

  ant.resize(numAntAnimations);

  for (unsigned int i = 0; i < numAntAnimations; i++) {
    ant[i].resize(numAntAnimationsImages[i]);
  }
}

void graphics_c::setTheme(const std::string & name) {

  for (unsigned int th = 0; th < themeNames.size(); th++) {
    if (themeNames[th] == name) {
      curTheme = th;
      return;
    }
  }

  themeNames.push_back(std::string(name));
  curTheme = themeNames.size()-1;

  bgTiles.resize(bgTiles.size()+1);
  fgTiles.resize(fgTiles.size()+1);

  loadTheme(name);

}

void graphics_c::addBgTile(SDL_Surface * v) {
  bgTiles[curTheme].push_back(v);
}

void graphics_c::addFgTile(SDL_Surface * v) {
  fgTiles[curTheme].push_back(v);
}

void graphics_c::addBgTile(unsigned int idx, SDL_Surface * v) {
  if (idx >= bgTiles[curTheme].size())
    bgTiles[curTheme].resize(idx+1);

  bgTiles[curTheme][idx] = v;
}

void graphics_c::addFgTile(unsigned int idx, SDL_Surface * v) {
  if (idx >= fgTiles[curTheme].size())
    fgTiles[curTheme].resize(idx+1);

  fgTiles[curTheme][idx] = v;
}

void graphics_c::setDomino(unsigned int type, unsigned int num, SDL_Surface * v) {
  dominos[type][num] = v;
}

void graphics_c::setCarriedDomino(unsigned int type, unsigned int num, SDL_Surface * v) {
  carriedDominos[type][num] = v;
}

void graphics_c::addAnt(unsigned int anim, unsigned int img, signed char yOffset, SDL_Surface * v, bool free) {

  antSprite s;
  s.v = v;
  s.ofs = yOffset;
  s.free = free;

  ant[anim][img] = s;
}

const unsigned char graphics_c::numDominoTypes = 18;
const unsigned char graphics_c::numDominos[numDominoTypes] = {
  15, 15, 14, 8, 15, 15, 15, 15, 15, 17, 6,
  6, 6, 6, 6, 6, 8, 1
};


graphics_c::~graphics_c(void) {

  for (unsigned int i = 0; i < bgTiles.size(); i++)
    for (unsigned int j = 0; j < bgTiles[i].size(); j++)
      SDL_FreeSurface(bgTiles[i][j]);

  for (unsigned int i = 0; i < fgTiles.size(); i++)
    for (unsigned int j = 0; j < fgTiles[i].size(); j++)
      SDL_FreeSurface(fgTiles[i][j]);

  for (unsigned int i = 0; i < dominos.size(); i++)
    for (unsigned int j = 0; j < dominos[i].size(); j++)
      if (dominos[i][j])
        SDL_FreeSurface(dominos[i][j]);

  for (unsigned int i = 0; i < ant.size(); i++)
    for (unsigned int j = 0; j < ant[i].size(); j++)
      if (ant[i][j].free)
        SDL_FreeSurface(ant[i][j].v);

  for (unsigned int i = 0; i < carriedDominos.size(); i++)
    for (unsigned int j = 0; j < carriedDominos[i].size(); j++)
      if (carriedDominos[i][j])
        SDL_FreeSurface(carriedDominos[i][j]);

  for(std::map<wchar_t, SDL_Surface*>::iterator i = bigFont.begin(); i != bigFont.end(); i++)
    SDL_FreeSurface(i->second);


}

void graphics_c::putText(SDL_Surface * target, int x, int y, const wchar_t * text, Uint8 r, Uint8 g, Uint8 b, bool shadow) {

  int pos = 0;

  while (text[pos]) {

    wchar_t c = text[pos];

    std::map<wchar_t, SDL_Surface *>::iterator i = bigFont.find(c);

    if (i != bigFont.end()) {

      SDL_Surface * w = i->second;

      SDL_Rect dst;
      dst.x = x;
      dst.y = y;
      dst.w = w->w;
      dst.h = w->h;

      if (shadow)
      {
        w->format->palette->colors[1].r = 0;
        w->format->palette->colors[1].g = 0;
        w->format->palette->colors[1].b = 0;

        SDL_Surface * y = SDL_DisplayFormatAlpha(w);

        dst.x -= 2;
        SDL_BlitSurface(y, 0, target, &dst);

        dst.x += 4;
        SDL_BlitSurface(y, 0, target, &dst);

        dst.x -= 2;

        dst.y -= 2;
        SDL_BlitSurface(y, 0, target, &dst);

        dst.y += 4;
        SDL_BlitSurface(y, 0, target, &dst);

        dst.y -= 2;

        SDL_FreeSurface(y);
      }

      w->format->palette->colors[1].r = r;
      w->format->palette->colors[1].g = g;
      w->format->palette->colors[1].b = b;

      SDL_Surface * y = SDL_DisplayFormatAlpha(w);
      SDL_BlitSurface(y, 0, target, &dst);
      SDL_FreeSurface(y);

      x += w->w + 1;
    }

    pos++;
  }
}

unsigned int graphics_c::textLen(const wchar_t * text, bool shadow) {

  int pos = 0;
  unsigned int width = 0;

  while (text[pos]) {

    wchar_t c = text[pos];

    std::map<wchar_t, SDL_Surface *>::iterator i = bigFont.find(c);

    if (i != bigFont.end()) {

      SDL_Surface * w = i->second;
      width += w->w + 1;
    }

    pos++;
  }

  return width;
}

void graphics_c::addBigGlyph(wchar_t c, SDL_Surface *v) {

  bigFont.insert(std::make_pair(c, v));
}

