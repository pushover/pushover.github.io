#include "graphicsn.h"
#include "level.h"
#include "ant.h"

#include <SDL.h>

#include <vector>

#include <stdio.h>

std::vector<int> recorder;
bool play;
unsigned int playpos;

void record(const std::string & level) {

    int num = 0;
    FILE * f = 0;

    do {
        num++;

        if (f) fclose(f);

        char fname[200];

        snprintf(fname, 200, "recordings/%05i.rec", num);
        f = fopen(fname, "r");

    } while (f);

    char fname[200];

    snprintf(fname, 200, "recordings/%05i.rec", num);
    f = fopen(fname, "w");

    fprintf(f, "%s\n", level.c_str());

    int val = recorder[0];
    int cnt = 1;
    unsigned int pos = 1;

    while (pos < recorder.size()) {

        if (recorder[pos] != val) {
            fprintf(f, "%i %i\n", cnt, val);
            val = recorder[pos];
            cnt = 1;
        }
        else
        {
            cnt++;
        }
        pos++;
    }

    fprintf(f, "%i %i\n", cnt, val);
    fclose(f);
}

const std::string loadrecord(const std::string & filename) {
    FILE * f = fopen(filename.c_str(), "r");

    static char level[200];
    fgets(level, 200, f);

    // remove the newline at the end
    level[strlen(level)-1] = 0;

    while (!feof(f))
    {
        int cnt, val;
        fscanf(f, "%i %i\n", &cnt, &val);
        for (int i = 0; i < cnt; i++)
            recorder.push_back(val);
    }

    printf("record builds on level %s\n", level);
    return level;
}

int main(int argn, char * argv[]) {

  bool useGraphics = true;

  enum {
      NOTHING,
      CHECK_FINISH
  } operation = NOTHING;

  std::string levelName;

  if (strcmp(argv[1], "-r") == 0)
  {
      levelName = loadrecord(argv[2]);
      play = true;
      playpos = 0;
  }
  else if (strcmp(argv[1], "-c") == 0)
  {
      levelName = loadrecord(argv[2]);
      play = true;
      playpos = 0;
      operation = CHECK_FINISH;
      useGraphics = false;
  }
  else
  {
      levelName =  argv[1];
      play = false;
  }

  graphics_c * gr = new graphicsN_c(".");
  SDL_Surface * video = 0;


  if (useGraphics)
  {
    SDL_Init(SDL_INIT_VIDEO);
    video = SDL_SetVideoMode(gr->resolutionX(), gr->resolutionY(), 24, 0);
    gr->loadGraphics();
  }

  level_c l;
  l.load_binary(levelName);

  if (useGraphics)
    gr->setTheme(l.getTheme());

  ant_c a;

  a.init(&l, gr);

  if (useGraphics)
    l.updateBackground(gr);

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
            if (event.key.keysym.sym == SDLK_b)
              blocks = !blocks;
            if (event.key.keysym.sym == SDLK_r)
              record(argv[1]);
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
      if (playpos < recorder.size())
        keyMask = recorder[playpos++];
    }
    else
    {
      recorder.push_back(keyMask);
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

    if (play)
    {
      if (playpos >= recorder.size())
      {
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
      }
    }

    if (l.triggerIsFalln() && !a.isVisible()) {

      if (l.someTimeLeft())
      {
        if (!play)
          record(argv[1]);

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
    printf("check NOT successful\n");
    ::exit(1);
  }

  if (useGraphics)
    delete gr;

  return 0;
}
