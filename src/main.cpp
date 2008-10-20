#include "graphicsn.h"
#include "textsections.h"
#include "level.h"
#include "levelset.h"
#include "ant.h"
#include "recorder.h"
#include "soundsys.h"
#include "text.h"
#include "screen.h"
#include "window.h"

#include <SDL.h>

#include <vector>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char * argv[]) {

  bool useGraphics = true;

  enum {
      NOTHING,
      CHECK_FINISH
  } operation = NOTHING;

  std::string levelName;
  std::string levelsetName;
  std::string levelFile;
  recorder_c rec;
  bool play;

  if (strcmp(argv[1], "-r") == 0)
  {
      rec.load(argv[2]);
      levelsetName = rec.getLevelsetName();
      levelName = rec.getLevelName();
      play = true;
  }
  else if (strcmp(argv[1], "-c") == 0)
  {
      rec.load(argv[2]);
      levelsetName = rec.getLevelsetName();
      levelName = rec.getLevelName();
      play = true;
      operation = CHECK_FINISH;
      useGraphics = false;
  }
  else if (argc == 3)
  {
      levelsetName = argv[1];
      levelName = argv[2];
      play = false;
  }
  else
  {
      levelFile = argv[1];
      play = false;
  }

  struct stat st;
  const std::string datadir((stat(PKGDATADIR, &st) == 0) ? PKGDATADIR : ".");

  graphicsN_c gr(datadir);
  screen_c screen(gr);

  SDL_Init(SDL_INIT_TIMER);

  if (useGraphics)
    screen.init();

  printf("%i: Init\n", SDL_GetTicks());
  atexit(SDL_Quit);

  if (useGraphics)
  {
    printf("%i: Video Mode set\n", SDL_GetTicks());
    gr.loadGraphics();
    printf("%i: Graphics loaded\n", SDL_GetTicks());

    initText();
    soundSystem_c::instance()->openSound(datadir);
  }

  level_c l(screen);
  if (levelFile == "")
  {
    levelsetList_c levelsetList;
    levelsetList.load(datadir + "/levels");
    char *home = getenv("HOME");
    if (home != NULL) {
      const std::string userleveldir(std::string(home) + "/.pushover/levels");
      struct stat st;
      if (stat(userleveldir.c_str(), &st) == 0)
        levelsetList.load(userleveldir);
    }
    levelsetList.getLevelset(levelsetName).loadLevel(l, levelName);
  }
  else
  {
    std::ifstream file(levelFile.c_str());
    textsections_c sections(file, true);
    l.load(sections);
  }
  rec.setLevel(levelsetName, l.getName());
  printf("%i: Level loaded\n", SDL_GetTicks());

  if (useGraphics)
  {
    gr.setTheme(l.getTheme());
    printf("%i: Theme loaded\n", SDL_GetTicks());
  }

  ant_c a(l, gr);

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

  if (useGraphics)
  {
    l.drawDominos(gr, blocks);

    while(!screen.flipAnimate()) {

      ticks += 1000/tickDiv;

      if (SDL_GetTicks() < ticks)
        SDL_Delay(ticks-SDL_GetTicks());
    }
  }

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
            if (event.key.keysym.sym == SDLK_F1) {
              std::string txt = "Arrange dominos in a run so that trigger falls last. You have 1 push.";
              if (!l.someTimeLeft())
              {
                std::vector<std::string> hints = l.getHint();

                txt = hints[0];
                for (int i = 1; i < hints.size(); i++)
                  txt = txt + " " + hints[i];
              }
              helpwindow_c w(txt, screen, gr);
              screen.flipComplete();

              bool exit = false;

              while (!exit) {

                SDL_Delay(100);

                while(SDL_PollEvent(&event)) {
                  if (event.type == SDL_KEYDOWN &&
                      event.key.keysym.sym == SDLK_ESCAPE)
                    exit = true;
                }
              }
              ticks = SDL_GetTicks() + 1000/tickDiv;

              screen.markAllDirty();

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
      a.performAnimation(screen);
      l.performDominos();
    }

    if (useGraphics)
    {
      l.updateBackground(gr);
      l.drawDominos(gr, blocks);
      a.draw(screen);
    }

    if (debug)
      l.print();

    if (useGraphics)
    {
      if (blocks)
      {
        screen.flipComplete();
      }
      else
      {
        screen.flipDirty();
      }
    }

    if (!pause && !singleStep)
      screen.clearDirty();

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

  if (useGraphics) {
    deinitText();
  }

  return 0;
}
