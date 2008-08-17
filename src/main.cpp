#include "graphicso.h"
#include "level.h"

#include <SDL.h>

const char * themes[] = {
  "AZTEC",
  "CASTLE",
  "CAVERN",
  "DUNGEON",
  "ELECTRO",
  "GREEK",
  "JAPANESE",
  "MECHANIC",
  "SPACE",
  "TOXCITY",
  "OPTION"
};


int main(int argn, char * argv[]) {

  graphics_c * gr = new graphicsO_c("/home/andy/.wine/drive_c/pushover", 2);

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *video = SDL_SetVideoMode(gr->resolutionX(), gr->resolutionY(), 24, 0);

  gr->loadGraphics();

  level_c l;

  char name[20];

  snprintf(name, 20, "%05i", atoi(argv[1]));

  l.load(name);

  gr->setTheme(l.getTheme());

  l.updateBackground(gr);
  l.drawDominos(video, gr);

  SDL_Flip(video);
  SDL_Delay(10*1000);

  return 0;
}
