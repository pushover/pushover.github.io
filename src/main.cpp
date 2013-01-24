/* Pushover
 *
 * Pushover is the legal property of its developers, whose
 * names are listed in the COPYRIGHT file, which is included
 * within the source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA
 */

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
#include <stdexcept>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libintl.h>

static void check_record(const std::string & rec_path, levelsetList_c & levelsetList,
                         unsigned int & count, unsigned int & failed,
                         std::string checker(const ant_c & a, const levelData_c & l)) {
  recorder_c rec;
  rec.load(rec_path);

  graphicsN_c gr("");
  surface_c surf;
  levelPlayer_c l(surf, gr);
  levelsetList.getLevelset(rec.getLevelsetName()).loadLevel(l, rec.getLevelName(), "");
  ant_c a(l, gr, surf);

  while (!rec.endOfRecord()) {
    a.setKeyStates(rec.getEvent());
    l.performDoors();
    l.performDominos(a);
  }

  // add a few more iterations at the end to make sure the ant has left the level
  a.setKeyStates(0);
  for (unsigned int j = 0; j < 100; j++)
  {
    l.performDoors();
    l.performDominos(a);
  }

  const std::string error = checker(a, l);
  if (!error.empty()) {
    std::cout << rec_path << " " << error << std::endl;
    failed++;
  }
  count++;
}

void check(int argn, char * argv[], std::string checker(const ant_c & a, const levelData_c & l)) {
  levelsetList_c levelsetList;
  levelsetList.load("levels", "");

  unsigned int count = 0;
  unsigned int failed = 0;

  for (int i = 0; i < argn; i++)
  {
    const std::string path(argv[i]);
    struct stat st;
    if (stat(path.c_str(), &st) != 0)
      throw std::runtime_error("file or directory does not exist: " + path);
    if (S_ISDIR(st.st_mode))
    {
      const std::vector<std::string> entries = directoryEntries(path);
      for (std::vector<std::string>::const_iterator j = entries.begin(); j != entries.end(); j++) {
        const std::string & filename = *j;
        if (filename.size() > 0 && filename[0] != '.')
          check_record(path + '/' + filename, levelsetList, count, failed, checker);
      }
    }
    else
    {
      check_record(path, levelsetList, count, failed, checker);
    }
  }

  std::cout << failed << " out of " << count << " tests failed\n";
}

std::string checker1(const ant_c & a, const levelData_c & l) {

    int i;
    // we succeeded, when the ant has vanished, then it went out of the door
    if (a.isVisible() == false && l.levelCompleted(i))
      return "";
    else
      return "Level not Finished";
}

std::string checker2(const ant_c & a, const levelData_c & l) {

    int i;
    // we succeeded, when the ant has vanished, then it went out of the door
    if (a.isVisible() == true || !l.levelCompleted(i))
      return "";
    else
      return "Level not Failed";
}

std::string checker3(const ant_c & a, const levelData_c & l) {
  // we succeeded, when the ant has vanished, then it went out of the door
  for (int y = 0; y < 13; y++)
    for (int x = 0; x < 20; x++)
      if (l.getDominoType(x, y) >= levelData_c::DominoTypeCrash0 &&
          l.getDominoType(x, y) <= levelData_c::DominoTypeCrash5) {
        return "";
      }

  return "Crashes didn't happen";
}

static std::string getDataDir(void)
{
  struct stat st;
  return std::string((stat(PKGDATADIR, &st) == 0) ? PKGDATADIR : ".");
}

static std::string getLocaleDir(void)
{
  struct stat st;
  return std::string((stat(LOCALEDIR, &st) == 0) ? LOCALEDIR : "locale");
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
// 0 nothing happened
// 1 success
// 2 too slow
// 3 crashes
// 4 not all dominos fell
// 5 die
// 6 trigger not last to fall
// 7 trigger not flat on the ground
//
int playTick(levelPlayer_c & l, ant_c & a)
{
  l.performDoors();
  int res = l.performDominos(a);

  l.updateBackground();
  l.drawDominos();
  a.draw();

  if (l.triggerIsFalln() && !a.isVisible() && l.isExitDoorClosed() && (res == 0)) {

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
  ST_INIT,     // initial transition state
  ST_MAIN,     // main menu
  ST_PROFILE,  // state for profile selection
  ST_PROFILE_INIT, // profile selection at startup
  ST_PROFILE_IN, // profile name input
  ST_PROFILE_DEL, // select profile to delete
  ST_LEVELCONF,// configure while playing
  ST_CONFIG,   // configure menu
  ST_LEVELSET, // level set selection
  ST_LEVEL,    // level selection
  ST_PREREPLAY,// prepare replay for running
  ST_REPLAY,   // replay currently running
  ST_PREPLAY,  // prepare level for playing
  ST_PLAY,     // currently playing
  ST_FAILDELAY,// play a second more after failing
  ST_SOLVED,   // current level solved
  ST_TIMEOUT,  // took longer than level time
  ST_FAILED,   // current level failed (reason in failreason)
  ST_HELP,     // help dialog showing
  ST_QUIT,     // play exit dialog showing
  ST_EXIT,     // exiting program
  ST_ABOUT,
} states_e;

static levelsetList_c * loadAllLevels(const std::string & datadir, const std::string & userString)
{
  levelsetList_c * levelsetList = new levelsetList_c();

  levelsetList->load(datadir + "/levels", userString);
  {
    const std::string userleveldir(getHome() + "/.pushover/levels");
    struct stat st;
    if (stat(userleveldir.c_str(), &st) == 0)
      levelsetList->load(userleveldir, userString);
  }

  return levelsetList;
}


int main(int argc, char * argv[]) {

  // filter out the no graphic cases, they are special and will be treated
  // separately
  if (argc >= 3 && strcmp(argv[1], "-c") == 0)   // the must complete tests
  {
    check(argc-2, argv+2, checker1);
    return 0;
  }

  if (argc >= 3 && strcmp(argv[1], "-y") == 0)   // the must complete tests
  {
    check(argc-2, argv+2, checker2);
    return 0;
  }

  if (argc >= 3 && strcmp(argv[1], "-x") == 0)   // the must crash tests
  {
    check(argc-2, argv+2, checker3);
    return 0;
  }

  bool fullscreen = false;
  if (argc >= 2 && strcmp(argv[1], "-f") == 0) fullscreen = true;

  // setup internationalization
  setlocale(LC_MESSAGES, "");
  bindtextdomain("pushover", getLocaleDir().c_str());
  bind_textdomain_codeset("pushover", "UTF-8");
  textdomain("pushover");

  // now off to all modes that use graphics
  const std::string datadir = getDataDir();
  std::string selectedMission;  // the mission that was selected in menu

  // initialize random number generator
  srandFromTime();

  // initialize SDL, graphics, timer, video mode, and level data structure
  SDL_Init(SDL_INIT_TIMER);
  SDL_Init(SDL_INIT_VIDEO);
  SDL_EnableUNICODE(1);
  atexit(SDL_Quit);
  graphicsN_c gr(datadir);
  screen_c screen(gr);
  if (fullscreen) screen.toggleFullscreen();
  gr.loadGraphics();
  initText(datadir);
  soundSystem_c::instance()->openSound(datadir);
  levelPlayer_c l(screen, gr);
  recorder_c rec;
  ant_c a(l, gr, screen);
  solvedMap_c solved;
  std::string lwindowLevel = "";  // when it contains a valid levelname, the level is selected when the levelwindow is opened the next time

  // prepare the list of levelsets
  levelsetList_c * levelsetList = loadAllLevels(datadir, "");

  states_e currentState = ST_INIT, nextState = ST_INIT;

  if (argc == 3 && strcmp(argv[1], "-r") == 0)
  {
    // try to load the record and the level that belongs to it
    // if it fails, fall back to main menu
    try {
      rec.load(argv[2]);
      selectedMission = rec.getLevelsetName();
      levelsetList->getLevelset(selectedMission).loadLevel(l, rec.getLevelName(), "");
      a.initForLevel();
      soundSystem_c::instance()->playMusic(datadir+"/themes/"+l.getTheme()+".ogg");

      nextState = ST_REPLAY;
    }
    catch  (...) {
      nextState = ST_PROFILE_INIT;
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

      levelsetList->getLevelset(selectedMission).loadLevel(l, levelName, "");
      a.initForLevel();
      soundSystem_c::instance()->playMusic(datadir+"/themes/"+l.getTheme()+".ogg");

      nextState = ST_PREPLAY;
    }
    catch (...) {
      nextState = ST_PROFILE_INIT;
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
      l.load(sections, "");
      selectedMission = "Original";            // TODO we need to find out which levelset this file belongs to
      soundSystem_c::instance()->playMusic(datadir+"/themes/"+l.getTheme()+".ogg");

      nextState = ST_PREPLAY;
    }
    else
    {
      nextState = ST_PROFILE_INIT;
      screen.markAllDirty();
    }
  }
  else
  {
    nextState = ST_PROFILE_INIT;
    screen.markAllDirty();
  }


  // not we have initialized, lets get playing in the main state
  bool exitProgram = false;
  Uint32 ticks = SDL_GetTicks();

  window_c * window = 0; // the currently visible window
  unsigned int failReason = 0;
  unsigned int failDelay = 0; // a counter to delay the fail window a bit after failing

  if (nextState == ST_MAIN)
  {
    soundSystem_c::instance()->playMusic(datadir+"/themes/option.ogg");
  }

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
            case ST_LEVELCONF:
            case ST_SOLVED:
            case ST_FAILED:
            case ST_ABOUT:
            case ST_PROFILE:
            case ST_PROFILE_INIT:
            case ST_PROFILE_IN:
            case ST_PROFILE_DEL:
            case ST_TIMEOUT:
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
              break;

          }

          // now enter the new one
          switch (nextState) {

            case ST_MAIN:     window = getMainWindow(screen, gr); break;
            case ST_PROFILE_INIT:
            case ST_PROFILE:  window = getProfileWindow(solved, screen, gr); break;
            case ST_PROFILE_IN: window = getProfileInputWindow(screen, gr); break;
            case ST_PROFILE_DEL: window = getProfileSelector(solved, screen, gr); break;
            case ST_LEVELSET: window = getMissionWindow(*levelsetList, screen, gr, selectedMission); break;
            case ST_QUIT:     window = getQuitWindow(screen, gr); break;
            case ST_LEVELCONF:
            case ST_CONFIG:   window = getConfigWindow(screen, gr); break;
            case ST_SOLVED:   window = getSolvedWindow(screen, gr); break;
            case ST_ABOUT:    window = getAboutWindow(screen, gr); break;
            case ST_FAILED:   window = getFailedWindow(failReason, screen, gr); break;
            case ST_TIMEOUT:  window = getTimeoutWindow(screen, gr); break;


            case ST_LEVEL:
                window = getLevelWindow(levelsetList->getLevelset(selectedMission), solved, screen, gr, lwindowLevel);
                lwindowLevel = "";
                break;

            case ST_HELP:
                              {
                                std::string text;
                                if (l.someTimeLeft())
                                  text = _("Arrange dominos in a run so that trigger falls last. You have 1 push.");
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
                              ticks = SDL_GetTicks();    // the flip might have taken some time too long
                              rec.setLevel(selectedMission, l.getName());
                              rec.reset();
                              break;
            case ST_INIT:
            case ST_REPLAY:
            case ST_FAILDELAY:
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

        // check for quit-event
        if (event.type == SDL_QUIT)
        {
          // exit program, wherever we are
          nextState = ST_EXIT;
          break;
        }

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
                  case 2: nextState = ST_PROFILE; break; // open profile selector
                  case 3: nextState = ST_ABOUT; break;   // about window
                  case 4: nextState = ST_EXIT; break;    // exit program
                }
              }
            }
            break;

          case ST_PROFILE:
          case ST_PROFILE_INIT:
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
                if (sel < solved.getNumberOfUsers())
                {
                  solved.selectUser(sel);
                  delete levelsetList;
                  levelsetList = loadAllLevels(datadir, solved.getUserString());

                  nextState = ST_MAIN;
                }
                else if (sel == solved.getNumberOfUsers())
                {
                  nextState = ST_PROFILE_IN;
                }
                else if (sel == solved.getNumberOfUsers()+1)
                {
                  nextState = ST_PROFILE_DEL;
                }
                else
                {
                  nextState = ST_MAIN;
                }
              }
            }
            break;

          case ST_PROFILE_DEL:
            if (!window) {
              std::cout << "Oops no window\n";
              nextState = ST_MAIN;
            }
            else
            {
              window->handleEvent(event);
              if (window->isDone())
              {
                size_t s = dynamic_cast<listWindow_c*>(window)->getSelection();

                if (s+1 < solved.getNumberOfUsers())
                {
                  // valid selection available

                  // if the currently selected profile is deleted, go to default
                  if (s+1 == solved.getCurrentUser())
                  {
                    solved.selectUser(0);
                    delete levelsetList;
                    levelsetList = loadAllLevels(datadir, solved.getUserString());
                  }

                  solved.deleteUser(s+1);
                }

                nextState = ST_PROFILE;
              }
            }
            break;

          case ST_PROFILE_IN:
            if (!window) {
              std::cout << "Oops no window\n";
              nextState = ST_MAIN;
            }
            else
            {
              window->handleEvent(event);
              if (window->isDone())
              {
                if (!dynamic_cast<InputWindow_c*>(window)->hasEscaped())
                {
                  solved.addUser(dynamic_cast<InputWindow_c*>(window)->getText());
                  delete levelsetList;
                  levelsetList = loadAllLevels(datadir, solved.getUserString());
                }
                nextState = ST_MAIN;
              }
            }
            break;

          case ST_CONFIG:
          case ST_LEVELCONF:
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
                    window->resetWindow();
                    break;

                  case 1:  // toggle sound effects
                    soundSystem_c::instance()->toggleSound();
                    window->resetWindow();
                    break;

                  case 2:  // toggle music
                    soundSystem_c::instance()->toggleMusic();
                    window->resetWindow();
                    break;

                  default:
                    if (currentState == ST_CONFIG)
                      nextState = ST_MAIN; // back to main menu
                    else
                      nextState = ST_QUIT; // back to level quit dialog

                    break;
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
                if (sel >= levelsetList->getLevelsetNames().size())
                  nextState = ST_MAIN;
                else
                {
                  nextState = ST_LEVEL;
                  lwindowLevel = "";
                  selectedMission = levelsetList->getLevelsetNames()[sel];
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
                levelset_c ls = levelsetList->getLevelset(selectedMission);

                if (sel >= ls.getLevelNames().size())
                  nextState = ST_LEVELSET;
                else
                {
                  nextState = ST_PREPLAY;
                  ls.loadLevel(l, ls.getLevelNames()[sel], solved.getUserString());
                  soundSystem_c::instance()->playMusic(datadir+"/themes/"+l.getTheme()+".ogg");
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
                  case 3:
                    nextState = ST_LEVEL;
                    soundSystem_c::instance()->playMusic(datadir+"/themes/option.ogg");
                    lwindowLevel = l.getName();
                    break;    // return to level list
                  case 1:
                          {       // restart level
                            nextState = ST_PLAY;
                            levelset_c ls = levelsetList->getLevelset(selectedMission);
                            ls.loadLevel(l, l.getName(), solved.getUserString());
                            a.initForLevel();
                          }
                          break;
                  case 2:  // configuration
                    nextState = ST_LEVELCONF;
                    break;
                  default:
                  case 0:
                    nextState = ST_PLAY;
                    break;    // return to play
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
                  case 0:
                    nextState = ST_LEVEL;
                    lwindowLevel = "";
                    break; // select next level to play
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
                      // find the current level
                      levelset_c ls = levelsetList->getLevelset(selectedMission);

                      std::vector<std::string> levels = ls.getLevelNames();

                      bool foundLevel = false;

                      for (unsigned int i = 0; i < levels.size(); i++)
                      {
                        if (levels[i] == l.getName())
                        {
                          ls.loadLevel(l, levels[i], solved.getUserString());
                          a.initForLevel();
                          foundLevel = true;
                          break;
                        }
                      }

                      if (!foundLevel)
                      {
                        nextState = ST_MAIN;
                      }
                      else
                      {
                        nextState = ST_PLAY;
                      }
                    }
                    break;
                  case 1:
                    nextState = ST_LEVEL;
                    lwindowLevel = l.getName();
                    soundSystem_c::instance()->playMusic(datadir+"/themes/option.ogg");
                    break;  // back to level list
                }
              }
            }
            break;

          case ST_TIMEOUT:
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
                      // find the current level
                      levelset_c ls = levelsetList->getLevelset(selectedMission);

                      std::vector<std::string> levels = ls.getLevelNames();

                      bool foundLevel = false;

                      for (unsigned int i = 0; i < levels.size(); i++)
                      {
                        if (levels[i] == l.getName())
                        {
                          ls.loadLevel(l, levels[i], solved.getUserString());
                          a.initForLevel();
                          foundLevel = true;
                          break;
                        }
                      }

                      if (!foundLevel)
                      {
                        nextState = ST_MAIN;
                      }
                      else
                      {
                        nextState = ST_PLAY;
                      }
                    }
                    break;
                  case 1:                            // continue to next level
                    nextState = ST_LEVEL;
                    lwindowLevel = "";
                    break; // select next level to play
                }
              }
            }
            break;

          case ST_INIT:
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

          if (l.levelInactive())
          {
            switch (failReason) {
              case 1:
                rec.save("sol");
                solved.addLevel(l.getChecksum());
                nextState = ST_SOLVED;
                break;
              case 0:
                break;
              case 2:
                rec.save("time");
                solved.addLevel(l.getChecksumNoTime());
                nextState = ST_TIMEOUT;
                break;
              default:
                failDelay = 36;
                nextState = ST_FAILDELAY;
                break;
            }
          }
          break;

        case ST_INIT:
        case ST_MAIN:
        case ST_PROFILE:
        case ST_PROFILE_INIT:
        case ST_PROFILE_IN:
        case ST_PROFILE_DEL:
        case ST_CONFIG:
        case ST_LEVELCONF:
        case ST_LEVELSET:
        case ST_LEVEL:
        case ST_SOLVED:
        case ST_FAILED:
        case ST_TIMEOUT:
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
