#include "graphicsn.h"
#include "textsections.h"
#include "levelplayer.h"
#include "levelset.h"
#include "ant.h"
#include "recorder.h"
#include "soundsys.h"
#include "screen.h"
#include "window.h"
#include "solvedmap.h"
#include "tools.h"

#include <SDL.h>

#include <vector>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int check1(std::string file) {
  recorder_c rec;
  rec.load(file);
  std::string levelsetName = rec.getLevelsetName();
  std::string levelName = rec.getLevelName();

  levelsetList_c levelsetList;
  levelsetList.load("levels");

  graphicsN_c gr("");
  surface_c surf;
  levelPlayer_c l(surf, gr);
  levelsetList.getLevelset(levelsetName).loadLevel(l, rec.getLevelName());
  ant_c a(l, gr, surf);

  while (!rec.endOfRecord()) {
    a.setKeyStates(rec.getEvent());
    l.performDoors();
    l.performDominos(a);
  }

  // add a few more iterations at the end to make sure the ant has left the level
  a.setKeyStates(0);
  for (unsigned int i = 0; i < 100; i++)
  {
    l.performDoors();
    l.performDominos(a);
  }

  // we succeeded, when the ant has vanished, then it went out of the door
  return a.isVisible() == false;
}

int check2(std::string file) {
  recorder_c rec;
  rec.load(file);
  std::string levelsetName = rec.getLevelsetName();
  std::string levelName = rec.getLevelName();

  levelsetList_c levelsetList;
  levelsetList.load("levels");

  graphicsN_c gr("");
  surface_c surf;
  levelPlayer_c l(surf, gr);
  levelsetList.getLevelset(levelsetName).loadLevel(l, rec.getLevelName());
  ant_c a(l, gr, surf);

  while (!rec.endOfRecord()) {
    a.setKeyStates(rec.getEvent());
    l.performDoors();
    l.performDominos(a);
  }

  // we succeeded, when the ant has vanished, then it went out of the door
  for (int y = 0; y < 13; y++)
    for (int x = 0; x < 20; x++)
      if (l.getDominoType(x, y) >= levelData_c::DominoTypeCrash0 &&
          l.getDominoType(x, y) <= levelData_c::DominoTypeCrash5)
        return true;
  return false;
}

static std::string getDataDir(void)
{
  struct stat st;
  return std::string((stat(PKGDATADIR, &st) == 0) ? PKGDATADIR : ".");
}

static unsigned int getKeyMask(void) {
  unsigned int keyMask = 0;

  Uint8 *keystate = SDL_GetKeyState(NULL);

  if ( keystate[SDLK_UP] ) keyMask |= KEY_UP;
  if ( keystate[SDLK_DOWN] ) keyMask |= KEY_DOWN;
  if ( keystate[SDLK_LEFT] ) keyMask |= KEY_LEFT;
  if ( keystate[SDLK_RIGHT] ) keyMask |= KEY_RIGHT;
  if ( keystate[SDLK_SPACE] ) keyMask |= KEY_ACTION;

  return keyMask;
}

// make a play tick
// return codes:
// 0 nothing happend
// 1 success
// 2 too slow
// 3 crashes
// 4 not all dominos fell
// 5 die
//
int playTick(levelPlayer_c & l, ant_c & a)
{
  l.performDoors();
  int res = l.performDominos(a);

  l.updateBackground();
  l.drawDominos();
  a.draw();

  if (l.triggerIsFalln() && !a.isVisible() && l.isExitDoorClosed()) {

    if (l.someTimeLeft())
    {
      return 1;
    }
    else
    {
      return 2;
    }
  }

  if (!a.isLiving())
    return 5;

  return res;
}

// the states of the program
typedef enum {
  ST_INIT,     // inistal transition state
  ST_MAIN,     // main menu
  ST_CONFIG,   // config menu
  ST_CONFIGTOG,// config menu toggled something, repaint
  ST_LEVELSET, // level set selection
  ST_LEVEL,    // level selection
  ST_PREREPLAY,// prepare replay for running
  ST_REPLAY,   // replay currently running
  ST_PREPLAY,  // prepare level for playing
  ST_PLAY,     // currently playing
  ST_FAILDELAY,// play a second more after failing
  ST_SOLVED,   // current level solved
  ST_FAILED,   // current level failed (reason in failreason)
  ST_HELP,     // help dialog showing
  ST_QUIT,     // play exit dialog showing
  ST_EXIT,     // exiting program
  ST_ABOUT,
} states_e;


int main(int argc, char * argv[]) {

  // filter out the no graphic cases, they are special and will be treated
  // separately
  if (argc == 3 && strcmp(argv[1], "-c") == 0)   // the must complete tests
  {
    if (check1(argv[2]))
    {
      std::cout << argv[2] << " Level Finished\n";
      return 0;
    }
    else
    {
      std::cout << argv[2] << " Level not Finished\n";
      return 1;
    }
  }
  if (argc == 3 && strcmp(argv[1], "-y") == 0)   // the must complete tests
  {
    if (check1(argv[2]))
    {
      std::cout << argv[2] << " Level not Failed\n";
      return 1;
    }
    else
    {
      std::cout << argv[2] << " Level Failed\n";
      return 1;
    }
  }

  if (argc == 3 && strcmp(argv[1], "-f") == 0)   // the must crash tests
  {
    if (check2(argv[2]))
    {
      std::cout << argv[2] << " Crashes happened\n";
      return 0;
    }
    else
    {
      std::cout << argv[2] << " Crashes did not happen\n";
      return 1;
    }
  }

  // now off to all modes that use graphics
  const std::string datadir = getDataDir();
  std::string selectedMission;  // the mission that was selected in menu

  // initialize SDL, graphics, timer, video mode, and level data structure
  SDL_Init(SDL_INIT_TIMER);
  SDL_Init(SDL_INIT_VIDEO);
  atexit(SDL_Quit);
  graphicsN_c gr(datadir);
  screen_c screen(gr);
  gr.loadGraphics();
  initText(datadir);
  soundSystem_c::instance()->openSound(datadir);
  levelPlayer_c l(screen, gr);
  recorder_c rec;
  ant_c a(l, gr, screen);
  solvedMap_c solved;

  // prepare the list of levelsets
  levelsetList_c levelsetList;
  levelsetList.load(datadir + "/levels");
  {
    const std::string userleveldir(getHome() + "/.pushover/levels");
    struct stat st;
    if (stat(userleveldir.c_str(), &st) == 0)
      levelsetList.load(userleveldir);
  }

  states_e currentState = ST_INIT, nextState = ST_INIT;

  if (argc == 3 && strcmp(argv[1], "-r") == 0)
  {
    // try to load the record and the level that belongs to it
    // if it fails, fall back to main menu
    try {
      rec.load(argv[2]);
      selectedMission = rec.getLevelsetName();
      levelsetList.getLevelset(selectedMission).loadLevel(l, rec.getLevelName());
      a.initForLevel();

      nextState = ST_REPLAY;
    }
    catch  (...) {
      nextState = ST_MAIN;
    }
  }
  else if (argc == 3)
  {
    // start with a given level set and level name
    selectedMission = argv[1];
    std::string levelName = argv[2];

    // try to load the given levelset and level, if
    // it fails fall back to main menu
    try {

      levelsetList.getLevelset(selectedMission).loadLevel(l, levelName);
      a.initForLevel();

      nextState = ST_PREPLAY;
    }
    catch (...) {
      nextState = ST_MAIN;
    }
  }
  else if (argc == 2)
  {
    // start with a given file if the file exists,
    // load it and play is, otherwise go to main menu
    std::ifstream file(argv[1]);
    if (file)
    {
      textsections_c sections(file, true);
      l.load(sections);
      selectedMission = "Original";            // TODO we need to find out which levelset this file belongs to

      nextState = ST_PREPLAY;
    }
    else
    {
      nextState = ST_MAIN;
      screen.markAllDirty();
    }
  }
  else
  {
    nextState = ST_MAIN;
    screen.markAllDirty();
  }


  // not we have initialized, lets get playing in the main state mashine
  bool exitProgram = false;
  Uint32 ticks = SDL_GetTicks();

  window_c * window = 0; // the currently visible window
  unsigned int failReason = 0;
  unsigned int failDelay = 0; // a counter to delay the fail window a bit after failing

  try {

    while (!exitProgram) {

      // wait for the right amount of time for the next frame
      ticks += 1000/18;
      if (SDL_GetTicks() < ticks)
        SDL_Delay(ticks-SDL_GetTicks());

      while (true) {

        if (nextState != currentState) {

          // switch states, first leave the old one
          switch (currentState) {

            case ST_MAIN:
            case ST_HELP:
            case ST_QUIT:
            case ST_LEVELSET:
            case ST_LEVEL:
            case ST_CONFIG:
            case ST_SOLVED:
            case ST_FAILED:
            case ST_ABOUT:
              delete window;
              window = 0;
              l.drawDominos();
              a.draw();
              break;

            case ST_PREREPLAY:
            case ST_PREPLAY:
            case ST_INIT:
            case ST_REPLAY:
            case ST_PLAY:
            case ST_FAILDELAY:
            case ST_EXIT:
            case ST_CONFIGTOG:
              break;

          }

          // now enter the new one
          switch (nextState) {

            case ST_MAIN:     window = getMainWindow(screen, gr); break;
            case ST_LEVELSET: window = getMissionWindow(levelsetList, screen, gr); break;
            case ST_LEVEL:    window = getLevelWindow(levelsetList.getLevelset(selectedMission), solved, screen, gr); break;
            case ST_QUIT:     window = getQuitWindow(screen, gr); break;
            case ST_CONFIG:   window = getConfigWindow(screen, gr); break;
            case ST_SOLVED:   window = getSolvedWindow(screen, gr); break;
            case ST_ABOUT:    window = getAboutWindow(screen, gr); break;
            case ST_FAILED:
                              {
                                std::string reason;
                                switch (failReason) {
                                  case 2: reason = "You've been too slow"; break;
                                  case 3: reason = "Some dominoes crashed"; break;
                                  case 4: reason = "Not all dominoes fell"; break;
                                  case 5: reason = "You died"; break;
                                  case 6: reason = "Trigger was not last to fall"; break;
                                }
                                window = getFailedWindow(reason, screen, gr);
                              }
                              break;

            case ST_HELP:
                              {
                                std::string text;
                                if (l.someTimeLeft())
                                  text = "Arrange dominos in a run so that trigger falls last. You have 1 push.";
                                else
                                  text = l.getHint();

                                window = new helpWindow_c(text, screen, gr);
                              }
                              break;

            case ST_PREREPLAY:
            case ST_PREPLAY:
                              l.updateBackground();
                              l.drawDominos();
                              a.draw();
                              ticks = SDL_GetTicks();    // this might have taken some time so reinit the ticks
                              break;

            case ST_PLAY:
                              rec.setLevel(selectedMission, l.getName());
                              rec.reset();
                              break;
            case ST_INIT:
            case ST_REPLAY:
            case ST_FAILDELAY:
            case ST_CONFIGTOG:
                              break;

            case ST_EXIT:
                              exitProgram = true;
                              break;

          }

          currentState = nextState;
        }

        // if there is no event to process, leave the loop, otherwise handle the event and loop again
        SDL_Event event;
        if (!SDL_PollEvent(&event)) break;

        switch (currentState) {

          case ST_MAIN:
            if (!window) {
              std::cout << "Oops no window\n";
              nextState = ST_MAIN;
            }
            else
            {
              window->handleEvent(event);
              if (window->isDone())
              {
                switch(dynamic_cast<listWindow_c*>(window)->getSelection())
                {
                  case 0: nextState = ST_LEVELSET; break;// select level set
                  case 1: nextState = ST_CONFIG; break;  // open config menu
                  case 2: nextState = ST_ABOUT; break;   // about window
                  case 3: nextState = ST_EXIT; break;    // exit program
                }
              }
            }
            break;

          case ST_CONFIG:
            if (!window) {
              std::cout << "Oops no window\n";
              nextState = ST_MAIN;
            }
            else
            {
              window->handleEvent(event);
              if (window->isDone())
              {
                switch(dynamic_cast<listWindow_c*>(window)->getSelection())
                {
                  case 0:   // toggle full screen
                    screen.toggleFullscreen();
                    screen.markAllDirty();
                    nextState = ST_CONFIGTOG;
                    break;
                  case 1: break;  // toggle sound effects
                  default: nextState = ST_MAIN; break;  // back to main menu
                }
              }
            }
            break;

          case ST_LEVELSET:
            if (!window) {
              std::cout << "Oops no window\n";
              nextState = ST_MAIN;
            }
            else
            {
              window->handleEvent(event);
              if (window->isDone())
              {
                unsigned int sel = dynamic_cast<listWindow_c*>(window)->getSelection();
                if (sel >= levelsetList.getLevelsetNames().size())
                  nextState = ST_MAIN;
                else
                {
                  nextState = ST_LEVEL;
                  selectedMission = levelsetList.getLevelsetNames()[sel];
                }
              }
            }
            break;

          case ST_LEVEL:
            if (!window) {
              std::cout << "Oops no window\n";
              nextState = ST_MAIN;
            }
            else
            {
              window->handleEvent(event);
              if (window->isDone())
              {
                unsigned int sel = dynamic_cast<listWindow_c*>(window)->getSelection();
                levelset_c ls = levelsetList.getLevelset(selectedMission);

                if (sel >= ls.getLevelNames().size())
                  nextState = ST_LEVELSET;
                else
                {
                  nextState = ST_PREPLAY;
                  ls.loadLevel(l, ls.getLevelNames()[sel]);
                  a.initForLevel();
                }
              }
            }
            break;

          case ST_PLAY:
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) nextState = ST_QUIT;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F1)     nextState = ST_HELP;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == 'r')         rec.save("man");
            break;

          case ST_PREPLAY:
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) nextState = ST_PLAY;
            break;

          case ST_PREREPLAY:
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) nextState = ST_REPLAY;
            break;

          case ST_REPLAY:
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
            {
              rec.truncate();
              nextState = ST_PLAY;
            }
            break;

          case ST_HELP:
            if (!window) {
              std::cout << "Oops no window\n";
              nextState = ST_MAIN;
            }
            else
            {
              window->handleEvent(event);
              if (window->isDone())
                nextState = ST_PLAY;
            }
            break;

          case ST_ABOUT:
            if (!window) {
              std::cout << "Oops no window\n";
              nextState = ST_MAIN;
            }
            else
            {
              window->handleEvent(event);
              if (window->isDone())
                nextState = ST_MAIN;
            }
            break;

          case ST_QUIT:
            if (!window) {
              std::cout << "Oops no window\n";
              nextState = ST_MAIN;
            }
            else
            {
              window->handleEvent(event);
              if (window->isDone())
              {
                switch(dynamic_cast<listWindow_c*>(window)->getSelection())
                {
                  case 2: nextState = ST_MAIN; break;    // return to main menu
                  case 1:
                          {       // restart level
                            nextState = ST_PREPLAY;
                            levelset_c ls = levelsetList.getLevelset(selectedMission);
                            ls.loadLevel(l, l.getName());
                            a.initForLevel();
                          }
                          break;
                  default:
                  case 0:
                          nextState = ST_PLAY; break;    // return to play
                }
              }
            }
            break;

          case ST_SOLVED:
            if (!window) {
              std::cout << "Oops no window\n";
              nextState = ST_MAIN;
            }
            else
            {
              window->handleEvent(event);
              if (window->isDone())
              {
                switch(dynamic_cast<listWindow_c*>(window)->getSelection())
                {
                  case 0:                            // play next level
                    {
                      // find the current level
                      levelset_c ls = levelsetList.getLevelset(selectedMission);

                      std::vector<std::string> levels = ls.getLevelNames();

                      // could not find the level ??? or no next level
                      // TODO find next _unsolvd_ level in mission

                      bool foundLevel = false;
                      bool foundNext = false;

                      for (unsigned int i = 0; i < levels.size(); i++)
                      {
                        if (levels[i] == l.getName())
                        {  // ok we found our level, continue forwar until we get the first
                          // unsolved level behint it
                          foundLevel = true;
                          i++;
                          while (i < levels.size())
                          {
                            if (!solved.solved(ls.getChecksum(levels[i])))
                            {  // fine we found one
                              foundNext = true;
                              ls.loadLevel(l, levels[i]);
                              a.initForLevel();
                              break;
                            }
                            i++;
                          }

                          break;
                        }
                      }

                      if (!foundLevel) nextState = ST_MAIN;
                      else if (!foundNext) nextState = ST_MAIN; // TODO present solved all window
                      else nextState = ST_PREPLAY;
                    }
                    break;
                  case 1: nextState = ST_MAIN; break;  // back to main
                }
              }
            }
            break;

          case ST_FAILED:
            if (!window) {
              std::cout << "Oops no window\n";
              nextState = ST_MAIN;
            }
            else
            {
              window->handleEvent(event);
              if (window->isDone())
              {
                switch(dynamic_cast<listWindow_c*>(window)->getSelection())
                {
                  case 0:                            // try again
                    {
                      nextState = ST_PREPLAY;
                      // find the current level
                      levelset_c ls = levelsetList.getLevelset(selectedMission);

                      std::vector<std::string> levels = ls.getLevelNames();

                      bool foundLevel = false;

                      for (unsigned int i = 0; i < levels.size(); i++)
                      {
                        if (levels[i] == l.getName())
                        {
                          ls.loadLevel(l, levels[i]);
                          a.initForLevel();
                          foundLevel = true;
                          break;
                        }
                      }

                      if (!foundLevel) nextState = ST_MAIN;
                      else nextState = ST_PREPLAY;
                    }
                    break;
                  case 1: nextState = ST_MAIN; break;  // back to main
                }
              }
            }
            break;

          case ST_INIT:
          case ST_CONFIGTOG:
          case ST_FAILDELAY:
          case ST_EXIT:
            break;
        }
      }

      // do the handling of the current state

      switch (currentState) {

        case ST_PREPLAY:
          if (screen.flipAnimate()) nextState = ST_PLAY;
          break;

        case ST_PREREPLAY:
          if (screen.flipAnimate()) nextState = ST_REPLAY;
          break;

        case ST_FAILDELAY:
          if (failDelay > 0)
          {
            failDelay--;
            a.setKeyStates(0);
            playTick(l, a);
          }
          else
          {
            nextState = ST_FAILED;
          }
          break;

        case ST_REPLAY:
          if (rec.endOfRecord())
            nextState = ST_PLAY;
          else
          {
            a.setKeyStates(rec.getEvent());
            if (playTick(l, a))
              nextState = ST_MAIN;
            break;
          }
          // intentionally fall through to ST_PLAY when
          // the record has ended to allow the player to
          // continue playing

        case ST_PLAY:
          {
            unsigned int keyMask = getKeyMask();
            rec.addEvent(keyMask);
            a.setKeyStates(keyMask);
          }
          failReason = playTick(l, a);
          switch (failReason) {
            case 1:
              rec.save("sol");
              solved.addLevel(l.getChecksum());
              nextState = ST_SOLVED;
              break;
            case 0:
              break;
            case 2:
              nextState = ST_FAILED;
              break;
            default:
              failDelay = 36;
              nextState = ST_FAILDELAY;
              break;
          }
          break;

        case ST_CONFIGTOG:
          nextState = ST_CONFIG;
          break;

        case ST_INIT:
        case ST_MAIN:
        case ST_CONFIG:
        case ST_LEVELSET:
        case ST_LEVEL:
        case ST_SOLVED:
        case ST_FAILED:
        case ST_HELP:
        case ST_QUIT:
        case ST_EXIT:
        case ST_ABOUT:
          break;
      }

      // flip the screen, but not when in the preplaymodes
      if (currentState != ST_PREPLAY && currentState != ST_PREREPLAY)
      {
        screen.flipDirty();
        screen.clearDirty();
      }
    }
  }

  catch (...) {

    if (currentState == ST_PLAY)
      rec.save("err");

  }

  return 0;
}
