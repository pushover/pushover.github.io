#include "graphics.h"

graphics_c::graphics_c(void) {

  dominos.resize(numDominoTypes);

  for (unsigned int i = 0; i < numDominoTypes; i++)
    dominos[i].resize(numDominos[i]);

}

void graphics_c::setTheme(const char *name) {

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

  printf("curTheme = %i\n", curTheme);

  loadTheme(name);

}

void graphics_c::addBgTile(SDL_Surface * v) {
  bgTiles[curTheme].push_back(v);
}

void graphics_c::addFgTile(SDL_Surface * v) {
  fgTiles[curTheme].push_back(v);
}

void graphics_c::setDomino(unsigned int type, unsigned int num, SDL_Surface * v) {
  dominos[type][num] = v;
}

const unsigned char graphics_c::numDominoTypes = 18;
const unsigned char graphics_c::numDominos[numDominoTypes] = {
  15, 15, 14, 8, 15, 15, 15, 15, 15,
    17, 6, 6, 6, 6, 6, 6, 8, 1
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
}

