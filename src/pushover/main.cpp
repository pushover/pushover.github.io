/* Pushover
 *
 * Pushover is the legal property of its developers, whose
 * names are listed in the AUTHORS file, which is included
 * within the source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA
 */

#include "check.h"
#include "graphicsn.h"
#include "levelset.h"
#include "ant.h"
#include "recorder.h"
#include "soundsys.h"
#include "window.h"
#include "solvedmap.h"
#include "tools.h"
#include "editor.h"

#include <SDL.h>

#include <fstream>
#include <string>

#include <sys/stat.h>
#include <libintl.h>

static std::string getDataDir(void)
{
  const std::string portable_datadir = "./data";
  struct stat st;
  if (stat(portable_datadir.c_str(), &st) == 0) {
      std::cout << "Using portable datadir: " << portable_datadir << std::endl;
      return portable_datadir;
  } else {
      std::cout << "Using system datadir: " << DATADIR << std::endl;
      return DATADIR;
  }
}

static LevelState playTick(ant_c & a, graphics_c & gr, unsigned int keys)
{
  LevelState res = a.performAnimation(keys);

  gr.drawLevel();

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
  ST_EDITOR,   // leveleditor stuff
} states_e;



int main(int argc, char * argv[]) {

  // directories
  const std::string datadir = getDataDir();
  const std::string pkgdatadir = datadir+"/pushover";
  const std::string localedir = datadir+"/locale";
  const std::string levelsdir = pkgdatadir+"/levels";

  check(levelsdir, argc, argv);

  configSettings conf;
  conf.useFullscreen = false;
  conf.playMusic = true;
  conf.playSounds = true;

  if (argc >= 2 && strcmp(argv[1], "-f") == 0) conf.useFullscreen = true;

  // setup internationalization
  setlocale(LC_MESSAGES, "");
  bindtextdomain("pushover", localedir.c_str());
  bind_textdomain_codeset("pushover", "UTF-8");
  textdomain("pushover");

  // now off to all modes that use graphics
  std::string selectedMission;  // the mission that was selected in menu

  // initialize random number generator
  srandFromTime();

  // initialize SDL, graphics, timer, video mode, and level data structure
  SDL_Init(SDL_INIT_TIMER);
  SDL_Init(SDL_INIT_VIDEO);
  SDL_EnableUNICODE(1);
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  atexit(SDL_Quit);
  graphicsN_c gr(pkgdatadir);
  screen_c screen(gr);
  if (conf.useFullscreen) screen.toggleFullscreen();
  initText();
  soundSystem_c::instance()->openSound(pkgdatadir);
  levelPlayer_c l;
  recorder_c rec;
  ant_c a(l);
  solvedMap_c solved;
  std::string lwindowLevel = "";  // when it contains a valid levelname, the level is selected when the levelwindow is opened the next time
#ifdef DEBUG
  bool debug_fastforward = false;
  bool debug_singlestep = false;
  bool debug_play = true;
#endif

  // prepare the list of levelsets
  levelsetList_c * levelsetList = loadAllLevels(levelsdir, "");

  states_e currentState = ST_INIT, nextState = ST_INIT;

  bool exitAfterReplay = false;

  if (argc == 3 && strcmp(argv[1], "-r") == 0)
  {
    // try to load the record and the level that belongs to it
    // if it fails, fall back to main menu
    try {
      rec.load(argv[2]);
      selectedMission = rec.getLevelsetName();
      levelsetList->getLevelset(selectedMission).loadLevel(l, rec.getLevelName(), "");
      a.initForLevel();
      soundSystem_c::instance()->playMusic(pkgdatadir+"/themes/"+l.getTheme()+".ogg");

      gr.setPaintData(&l, &a, &screen);
      nextState = ST_REPLAY;
    }
    catch  (...) {
      nextState = ST_PROFILE_INIT;
    }
  } else if (argc == 3 && strcmp(argv[1], "-R") == 0)
  {
    // try to load the record and the level that belongs to it
    // if it fails, fall back to main menu
    try {
      rec.load(argv[2]);
      selectedMission = rec.getLevelsetName();
      levelsetList->getLevelset(selectedMission).loadLevel(l, rec.getLevelName(), "");
      a.initForLevel();
      soundSystem_c::instance()->playMusic(pkgdatadir+"/themes/"+l.getTheme()+".ogg");

      gr.setPaintData(&l, &a, &screen);
      nextState = ST_REPLAY;
      exitAfterReplay = true;
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
      soundSystem_c::instance()->playMusic(pkgdatadir+"/themes/"+l.getTheme()+".ogg");

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
      selectedMission = "";            // TODO we need to find out which levelset this file belongs to
      gr.setPaintData(&l, &a, &screen);
      soundSystem_c::instance()->playMusic(pkgdatadir+"/themes/"+l.getTheme()+".ogg");

      nextState = ST_PREPLAY;
    }
    else
    {
      nextState = ST_PROFILE_INIT;
    }
  }
  else
  {
    nextState = ST_PROFILE_INIT;
    gr.setPaintData(0, 0, &screen);
  }


  // not we have initialized, lets get playing in the main state
  bool exitProgram = false;
  Uint32 ticks = SDL_GetTicks();

#ifdef DEBUG
  uint32_t tick = 0;
#endif

  window_c * window = 0; // the currently visible window
  LevelState failReason = LS_undecided;
  unsigned int failDelay = 0; // a counter to delay the fail window a bit after failing

  if (nextState == ST_MAIN)
  {
    soundSystem_c::instance()->playMusic(pkgdatadir+"/themes/option.ogg");
  }

  try {

    while (!exitProgram) {

      // wait for the right amount of time for the next frame
      if (!exitAfterReplay)
      {
#ifdef DEBUG
        if (!debug_fastforward)
#endif
        ticks += 1000/18;
        while (SDL_GetTicks() < ticks)
          SDL_Delay(ticks-SDL_GetTicks());
      }

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
              gr.drawLevel();
              break;

            case ST_EDITOR:
              leaveEditor();
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

            case ST_MAIN:
              window = getMainWindow(screen, gr);
              break;

            case ST_PROFILE_INIT:
              soundSystem_c::instance()->playMusic(pkgdatadir+"/themes/option.ogg");
              // intentionally fall through

            case ST_PROFILE:
              window = getProfileWindow(solved, screen, gr);
              break;

            case ST_PROFILE_IN:
              window = getProfileInputWindow(screen, gr);
              break;

            case ST_PROFILE_DEL:
              window = getProfileSelector(solved, screen, gr);
              break;

            case ST_LEVELSET:
              window = getMissionWindow(*levelsetList, solved, screen, gr, selectedMission);
              break;

            case ST_QUIT:
              window = getQuitWindow(selectedMission != "", screen, gr);
              break;

            case ST_LEVELCONF:
            case ST_CONFIG:
              window = getConfigWindow(screen, gr, conf, 0);
              break;

            case ST_SOLVED:
              window = getSolvedWindow(screen, gr);
              break;

            case ST_ABOUT:
              window = getAboutWindow(screen, gr, *levelsetList);
              break;

            case ST_FAILED:
              window = getFailedWindow(failReason, screen, gr);
              break;

            case ST_TIMEOUT:
              window = getTimeoutWindow(screen, gr);
              break;

            case ST_LEVEL:
              window = getLevelWindow(levelsetList->getLevelset(selectedMission), solved, screen, gr, lwindowLevel);
              lwindowLevel = "";
              break;

            case ST_HELP:
              window = getHelpWindow(selectedMission, l, a.getCarriedDomino(), screen, gr);
              break;

            case ST_PREREPLAY:
            case ST_PREPLAY:
              gr.drawLevel();
              ticks = SDL_GetTicks();    // this might have taken some time so reinit the ticks
              break;

            case ST_PLAY:
#ifdef DEBUG
              tick = 0;
#endif
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

            case ST_EDITOR:
              startEditor(gr, screen, l, a, solved.getUserName(solved.getCurrentUser()));
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

#ifdef DEBUG
        if (currentState == ST_PLAY || currentState == ST_REPLAY)
        {
          if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F2)         gr.setShowGrid(!gr.getShowGrid());
          if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F3)         debug_fastforward = true;
          if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F4)         debug_singlestep = true;
          if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F5)         debug_play = true;
          if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F6)         { printf("Tick: %i\n", tick); l.print(); }
        }
#endif
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
                switch(window->getSelection())
                {
                  case 0: nextState = ST_LEVELSET; break;// select level set
                  case 1: nextState = ST_CONFIG; break;  // open config menu
                  case 2: nextState = ST_PROFILE; break; // open profile selector
                  case 3: nextState = ST_EDITOR; break;  // go to level editor
                  case 4: nextState = ST_ABOUT; break;   // about window
                  case 5: nextState = ST_EXIT; break;    // exit program
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
                unsigned int sel = window->getSelection();
                if (sel < solved.getNumberOfUsers())
                {
                  solved.selectUser(sel);
                  delete levelsetList;
                  levelsetList = loadAllLevels(levelsdir, solved.getUserString());

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
                size_t s = window->getSelection();

                if (s+1 < solved.getNumberOfUsers())
                {
                  // valid selection available

                  // if the currently selected profile is deleted, go to default
                  if (s+1 == solved.getCurrentUser())
                  {
                    solved.selectUser(0);
                    delete levelsetList;
                    levelsetList = loadAllLevels(levelsdir, solved.getUserString());
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
                if (!window->hasEscaped())
                {
                  solved.addUser(window->getText());
                  delete levelsetList;
                  levelsetList = loadAllLevels(levelsdir, solved.getUserString());
                  nextState = ST_MAIN;
                }
                else
                {
                  nextState = ST_PROFILE;
                }
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
                switch(window->getSelection())
                {
                  case 0:   // toggle full screen
                    conf.useFullscreen = !conf.useFullscreen;
                    screen.toggleFullscreen();
                    gr.markAllDirty();
                    delete window;
                    window = getConfigWindow(screen, gr, conf, 0);
                    break;

                  case 1:  // toggle sound effects
                    conf.playSounds = !conf.playSounds;
                    soundSystem_c::instance()->toggleSound();
                    delete window;
                    window = getConfigWindow(screen, gr, conf, 1);
                    break;

                  case 2:  // toggle music
                    conf.playMusic = !conf.playMusic;
                    soundSystem_c::instance()->toggleMusic();
                    delete window;
                    window = getConfigWindow(screen, gr, conf, 2);
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
                unsigned int sel = window->getSelection();
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
                unsigned int sel = window->getSelection();
                levelset_c ls = levelsetList->getLevelset(selectedMission);

                if (sel >= ls.getLevelNames().size())
                  nextState = ST_LEVELSET;
                else
                {
                  nextState = ST_PREPLAY;
                  ls.loadLevel(l, ls.getLevelNames()[sel], solved.getUserString());
                  gr.setPaintData(&l, &a, &screen);
                  soundSystem_c::instance()->playMusic(pkgdatadir+"/themes/"+l.getTheme()+".ogg");
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
                // there are 2 different versions of the quit window, one
                // for normal level selection, one for selection with a command
                // line argument

                if (selectedMission != "")
                {
                  switch(window->getSelection())
                  {
                    case 3:
                      nextState = ST_LEVEL;
                      soundSystem_c::instance()->playMusic(pkgdatadir+"/themes/option.ogg");
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
                else
                {
                  switch(window->getSelection())
                  {
                    case 2:
                      nextState = ST_MAIN;
                      soundSystem_c::instance()->playMusic(pkgdatadir+"/themes/option.ogg");
                      break;    // return to main window
                    case 1:  // configuration
                      nextState = ST_LEVELCONF;
                      break;
                    default:
                    case 0:
                      nextState = ST_PLAY;
                      break;    // return to play
                  }
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
                switch(window->getSelection())
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
                switch(window->getSelection())
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
                    soundSystem_c::instance()->playMusic(pkgdatadir+"/themes/option.ogg");
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
                switch(window->getSelection())
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

          case ST_EDITOR:
            if (eventEditor(event))
            {
              nextState = ST_MAIN;
            }
            break;
        }
      }

#ifdef DEBUG
      if (currentState == ST_PLAY || currentState == ST_REPLAY)
      {
        if (debug_singlestep)
        {
          debug_singlestep = false;
          debug_play = false;
          debug_fastforward = false;
        }
        else if (debug_play)
        {
          debug_fastforward = false;
          debug_singlestep = false;
        }
        else if (debug_fastforward)
        {
          debug_play = false;
        }
        else
        {
          continue;
        }
      }
#endif

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
            playTick(a, gr, 0);
          }
          else
          {
            nextState = ST_FAILED;
          }
          break;

        case ST_REPLAY:
          if (rec.endOfRecord())
          {
            if (exitAfterReplay)
              nextState = ST_EXIT;
            else
              nextState = ST_PLAY;
          }
          else
          {
#ifdef DEBUG
            tick++;
#endif
            if (playTick(a, gr, rec.getEvent()) != LS_undecided)
            {
              if (exitAfterReplay)
              {
                nextState = ST_EXIT;
              }
              else
              {
                nextState = ST_MAIN;
              }
            }
            break;
          }
          // intentionally fall through to ST_PLAY when
          // the record has ended to allow the player to
          // continue playing
          [[fallthrough]];

        case ST_PLAY:
          {
            unsigned int keyMask = getKeyMask();
            rec.addEvent(keyMask);
#ifdef DEBUG
            tick++;
#endif
            failReason = playTick(a, gr, keyMask);
          }

          if (l.levelInactive())
          {
            switch (failReason) {
              case LS_solved:
                rec.save("sol");
                solved.addLevel(l.getChecksum());
                nextState = ST_SOLVED;
                break;
              case LS_undecided:
                break;
              case LS_solvedTime:
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

        case ST_EDITOR:
          stepEditor();
          break;
      }

      // flip the screen, but not when in the preplaymodes
      if (currentState != ST_PREPLAY && currentState != ST_PREREPLAY)
      {
        screen.flipDirty(gr.getDirty());
        gr.clearDirty();
      }
    }
  }

  catch (...) {

    if (currentState == ST_PLAY)
      rec.save("err");

  }

  soundSystem_c::instance()->playMusic(pkgdatadir+"/themes/NonExistingFile.ogg");

  return 0;
}
