#include "graphicso.h"
#include "level.h"
#include "ant.h"

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

  graphics_c * gr = new graphicsO_c(".", 2);

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *video = SDL_SetVideoMode(gr->resolutionX(), gr->resolutionY(), 24, 0);

  gr->loadGraphics();

  level_c l;

  l.load(argv[1]);

  gr->setTheme(l.getTheme());

  ant_c a;

  a.init(&l, gr);

  bool exit = false;

  while (!exit) {

    SDL_Event event; /* Event structure */

    while(SDL_PollEvent(&event)) {  /* Loop until there are no events left on the queue */
      switch(event.type) { /* Process the appropiate event type */
        case SDL_KEYDOWN:  /* Handle a KEYDOWN event */
          exit = true;
          break;
      }
    }

    l.performDoors();
    a.performAnimation();

    l.updateBackground(gr);
    l.drawDominos(video, gr);
    a.draw(video);

    SDL_Flip(video);

    SDL_Delay(1000);
  }

  return 0;
}
