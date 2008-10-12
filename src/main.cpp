#include "graphicsn.h"
#include "textsections.h"
#include "level.h"
#include "ant.h"
#include "recorder.h"
#include "soundsys.h"

#include <SDL.h>

#include <vector>

#include <stdio.h>

#include <fstream>

int main(int argn, char * argv[]) {

  bool useGraphics = true;

  enum {
      NOTHING,
      CHECK_FINISH
  } operation = NOTHING;

  std::string levelFile;
  recorder_c rec;
  bool play;

  if (strcmp(argv[1], "-r") == 0)
  {
      rec.load(argv[2]);
      levelFile = "./levels/original/" + rec.getLevelName() + ".level";
      play = true;
  }
  else if (strcmp(argv[1], "-c") == 0)
  {
      rec.load(argv[2]);
      levelFile = "./levels/original/" + rec.getLevelName() + ".level";
      play = true;
      operation = CHECK_FINISH;
      useGraphics = false;
  }
  else
  {
      levelFile = argv[1];
      play = false;
  }

  graphics_c * gr = new graphicsN_c(".");
  SDL_Surface * video = 0;

  if (useGraphics)
    SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO);
  else
    SDL_Init(SDL_INIT_TIMER);
  printf("%i: Init\n", SDL_GetTicks());
  atexit(SDL_Quit);

  if (useGraphics)
  {
    video = SDL_SetVideoMode(gr->resolutionX(), gr->resolutionY(), 24, 0);
    printf("%i: Video Mode set\n", SDL_GetTicks());
    gr->loadGraphics();
    printf("%i: Graphics loaded\n", SDL_GetTicks());

    soundSystem_c::instance()->openSound(".");
  }

  level_c l;
  {
    std::ifstream file(levelFile.c_str());
    textsections_c sections(file, true);
    l.load(sections);
  }
  rec.setLevelName(l.getName());
  printf("%i: Level loaded\n", SDL_GetTicks());

  if (useGraphics)
  {
    gr->setTheme(l.getTheme());
    printf("%i: Theme Loaded\n", SDL_GetTicks());
  }

  ant_c a;
  a.init(&l, gr);

  if (useGraphics)
  {
    l.updateBackground(gr);
    printf("%i: Background drawn\n", SDL_GetTicks());
  }

  Uint32 ticks = 0;

  if (useGraphics)
    ticks = SDL_GetTicks();

  bool exit = false;
  int tickDiv = 18;
  bool debug = false;
  bool finishCheckDone = false;
  bool pause = false;
  bool blocks = false;
  bool singleStep = false;
  SDL_Rect rects[10*13];

  while (!exit) {

    ticks += 1000/tickDiv;

    unsigned int keyMask = 0;

    if (singleStep) pause = true;

    if (useGraphics)
    {
      SDL_Event event; /* Event structure */

      while(SDL_PollEvent(&event)) {  /* Loop until there are no events left on the queue */
        switch(event.type) { /* Process the appropiate event type */
          case SDL_KEYDOWN:  /* Handle a KEYDOWN event */

            if (event.key.keysym.sym == SDLK_ESCAPE)
              if (play)
                play = false;
              else
                exit = true;
            if (event.key.keysym.sym == SDLK_s) {
              tickDiv = 1;
              ticks = SDL_GetTicks() + 1000/tickDiv;
            }
            if (event.key.keysym.sym == SDLK_f) {
              tickDiv = 1000;
              ticks = SDL_GetTicks() + 1000/tickDiv;
            }
            if (event.key.keysym.sym == SDLK_n) {
              tickDiv = 18;
              ticks = SDL_GetTicks() + 1000/tickDiv;
            }
            if (event.key.keysym.sym == SDLK_d)
              debug = !debug;
            if (event.key.keysym.sym == SDLK_h) {
              const std::vector<std::string> hint = l.getHint();
              for (std::vector<std::string>::const_iterator i = hint.begin(); i != hint.end(); i++)
                  std::cout << (*i) << std::endl;
            }
            if (event.key.keysym.sym == SDLK_b)
              blocks = !blocks;
            if (event.key.keysym.sym == SDLK_r)
              rec.save();
            if (event.key.keysym.sym == SDLK_p) {
              pause = !pause;
              ticks = SDL_GetTicks() + 1000/tickDiv;
              singleStep = false;
            }
            if (event.key.keysym.sym == SDLK_t) {
              pause = false;
              ticks = SDL_GetTicks();
              singleStep = true;
            }
            break;
        }
      }

      Uint8 *keystate = SDL_GetKeyState(NULL);

      if ( keystate[SDLK_UP] ) keyMask |= KEY_UP;
      if ( keystate[SDLK_DOWN] ) keyMask |= KEY_DOWN;
      if ( keystate[SDLK_LEFT] ) keyMask |= KEY_LEFT;
      if ( keystate[SDLK_RIGHT] ) keyMask |= KEY_RIGHT;
      if ( keystate[SDLK_SPACE] ) keyMask |= KEY_ACTION;
    }

    if (play)
    {
      keyMask = rec.getEvent();
    }
    else
    {
      rec.addEvent(keyMask);
    }

    if (l.triggerIsFalln() && !finishCheckDone)
    {
      finishCheckDone = true;
      if (l.levelCompleted(0) && !a.carrySomething() && a.isLiving())
      {
        a.success();
      }
      else
      {
        a.fail();
      }
    }

    if (!pause)
    {
      a.setKeyStates(keyMask);

      l.performDoors();
      a.performAnimation();
      l.performDominos();
    }

    if (useGraphics)
    {
      l.updateBackground(gr);
      l.drawDominos(video, gr, blocks);
      a.draw(video);
    }

    if (debug)
      l.print();

    if (useGraphics)
    {
      if (blocks)
      {
        SDL_Flip(video);
      }
      else
      {
        int numrects = 0;

        for (int y = 0; y < 13; y++)
        {
          int rowStart = -1;

          for (int x = 0; x < 21; x++)
          {
            if (l.isDirty(x, y) && (x < 20))
            {
              if (rowStart == -1)
                rowStart = x;
            }
            else if (rowStart != -1)
            {
              rects[numrects].y = gr->blockY()*y;
              rects[numrects].x = gr->blockX()*rowStart;

              if (y == 12)
                rects[numrects].h = gr->blockY()/2;
              else
                rects[numrects].h = gr->blockY();

              rects[numrects].w = gr->blockX()*(x-rowStart);
              numrects++;
              rowStart = -1;
            }
          }
        }
        SDL_UpdateRects(video, numrects, rects);
      }
    }

    if (!pause && !singleStep)
      l.clearDirty();

    if (play && rec.endOfRecord())
      exit = true;

    if (l.triggerIsFalln() && !a.isVisible()) {

      if (l.someTimeLeft())
      {
        if (!play)
          rec.save();

        printf("gratulation, you solved the level\n");
      }
      else
      {
        printf("Sorry You've been too slow\n");
      }

      exit = true;
    }

    if (useGraphics)
    {
      if (SDL_GetTicks() < ticks)
        SDL_Delay(ticks-SDL_GetTicks());
    }
  }

  if (operation == CHECK_FINISH)
  {
    if (l.triggerIsFalln() && !a.isVisible() && l.someTimeLeft())
    {
      printf("check successful\n");
      ::exit(0);
    } else {
      printf("check NOT successful\n");
      ::exit(1);
    }
  }

  if (useGraphics)
    delete gr;

  return 0;
}
