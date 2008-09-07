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

  Uint32 ticks = SDL_GetTicks();

  while (!exit) {

    ticks += 1000/18;

    {
      SDL_Event event; /* Event structure */

      while(SDL_PollEvent(&event)) {  /* Loop until there are no events left on the queue */
        switch(event.type) { /* Process the appropiate event type */
          case SDL_KEYDOWN:  /* Handle a KEYDOWN event */
            if (event.key.keysym.sym == SDLK_ESCAPE)
              exit = true;
            break;
        }

        Uint8 *keystate = SDL_GetKeyState(NULL);

        unsigned int keyMask = 0;

        if ( keystate[SDLK_UP] ) keyMask |= KEY_UP;
        if ( keystate[SDLK_DOWN] ) keyMask |= KEY_DOWN;
        if ( keystate[SDLK_LEFT] ) keyMask |= KEY_LEFT;
        if ( keystate[SDLK_RIGHT] ) keyMask |= KEY_RIGHT;
        if ( keystate[SDLK_SPACE] ) keyMask |= KEY_ACTION;

        a.setKeyStates(keyMask);

      }
    }

    l.performDoors();
    a.performAnimation();
    l.performDominos();

    l.updateBackground(gr);
    l.drawDominos(video, gr);
    a.draw(video);

    SDL_Flip(video);

    if (SDL_GetTicks() < ticks)
      SDL_Delay(ticks-SDL_GetTicks());
  }

  delete gr;

  return 0;
}
