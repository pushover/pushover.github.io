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

#include "editor.h"

#include "window.h"
#include "screen.h"
#include "graphicsn.h"
#include "tools.h"

#include <fstream>
#include <sys/stat.h>

// states of the level Editor
typedef enum {
  ST_INIT,
  ST_CHOOSE_LEVEL,     // initial transition state
  ST_EXIT,
  ST_NEW_LEVEL,
  ST_DOUBLE_NAME,
  ST_DELETE_LEVEL,
  ST_EDIT_HOME,        // the base level for the level editor
} states_e;

static states_e currentState, nextState, messageContinue;
static window_c * window;

static graphicsN_c * gr;
static screen_c * screen;
static levelset_c * levels;
static levelPlayer_c * l;
static std::string userName;

static void loadLevels(void)
{
  if (levels) delete levels;

  try {
    levels = new levelset_c(getHome() + "levels/", "", true);
  }
  catch (format_error e)
  {
    printf("format error %s\n", e.msg.c_str());
    levels = 0;
  }
}

static void changeState(void)
{
  if (currentState != nextState)
  {
    // leave old state
    switch (currentState)
    {
      case ST_INIT:
      case ST_EXIT:
      case ST_EDIT_HOME:
        break;

      case ST_CHOOSE_LEVEL:
      case ST_NEW_LEVEL:
      case ST_DELETE_LEVEL:
      case ST_DOUBLE_NAME:
        delete window;
        window = 0;
        break;
    }

    currentState = nextState;

    // enter new state
    switch (currentState)
    {
      case ST_INIT:
      case ST_EXIT:
      case ST_EDIT_HOME:
        break;

      case ST_CHOOSE_LEVEL:
        window = getEditorLevelChooserWindow(*screen, *gr, *levels);
        break;

      case ST_NEW_LEVEL:
        window = getNewLevelWindow(*screen, *gr);
        break;

      case ST_DELETE_LEVEL:
        window = getDeleteLevelWindow(*screen, *gr, *levels);
        break;

      case ST_DOUBLE_NAME:
        window = getMessageWindow(*screen, *gr, _("The given level name already exists"));
        break;
    }
  }
}

void leaveEditor(void)
{
  if (window) delete window;
  window = 0;

  delete levels;
  levels = 0;

  if (currentState != ST_EXIT)
    printf("oops editor left without reason\n");
}

void startEditor(graphicsN_c & g, screen_c & s, levelPlayer_c & lp, const std::string & user)
{
  nextState = ST_CHOOSE_LEVEL;
  currentState = ST_INIT;
  window = 0;

  screen = &s;
  gr = &g;

  loadLevels();

  l = &lp;
  userName = user;
}

bool eventEditor(const SDL_Event & event)
{
  changeState();

  switch (currentState)
  {
    case ST_INIT:
    case ST_EXIT:
      break;

    case ST_CHOOSE_LEVEL:
      if (!window)
      {
        nextState = ST_EXIT;
      }
      else
      {
        window->handleEvent(event);

        if (window->isDone())
        {
          unsigned int sel = window->getSelection();

          if (sel < levels->getLevelNames().size())
          {
            // a real level choosen... load that one and go into editor
            levels->loadLevel(*l, levels->getLevelNames()[sel], "");
            nextState = ST_EDIT_HOME;
          }
          else
          {
            // no level loaded
            sel -= levels->getLevelNames().size();

            if (sel == 0)
              nextState = ST_NEW_LEVEL;
            else if (levels->getLevelNames().size() && sel == 1)
              nextState = ST_DELETE_LEVEL;
            else
              nextState = ST_EXIT;
          }
        }
      }
      break;

    case ST_NEW_LEVEL:
      if (!window)
      {
        nextState = ST_EXIT;
      }
      else
      {
        window->handleEvent(event);

        if (window->isDone())
        {
          std::string fname = getHome() + "levels/" + window->getText() + ".level";
          struct stat st;

          // first check if the resulting filename would be
          if (stat(fname.c_str(), &st) == 0)
          {
            nextState = ST_DOUBLE_NAME;
            messageContinue = ST_CHOOSE_LEVEL;
          }
          else
          {
            // create the new level
            {
              levelData_c l;
              l.setName(window->getText());
              l.setAuthor(userName);
              std::ofstream f(fname.c_str());
              l.save(f);
            }
            loadLevels();

            levels->loadLevel(*l, window->getText(), "");

            nextState = ST_EDIT_HOME;
          }
        }
      }
      break;

    case ST_DELETE_LEVEL:
      if (!window)
      {
        nextState = ST_EXIT;
      }
      else
      {
        window->handleEvent(event);

        if (window->isDone())
        {
          unsigned int sel = window->getSelection();

          if (sel < levels->getLevelNames().size())
          {
            std::string name = levels->getLevelNames()[sel];
            std::string fname = levels->getFilename(name);

            remove(fname.c_str());

            loadLevels();
          }
          nextState = ST_CHOOSE_LEVEL;
        }
      }
      break;

    case ST_DOUBLE_NAME:
      if (!window)
      {
        nextState = ST_EXIT;
      }
      else
      {
        window->handleEvent(event);

        if (window->isDone())
        {
          nextState = messageContinue;
        }
      }
      break;

    case ST_EDIT_HOME:

      nextState = ST_CHOOSE_LEVEL;

      break;

  }

  return currentState == ST_EXIT;
}

void stepEditor(void)
{
  changeState();

  switch (currentState)
  {
    case ST_INIT:
    case ST_EXIT:
    case ST_CHOOSE_LEVEL:
    case ST_NEW_LEVEL:
    case ST_EDIT_HOME:
    case ST_DELETE_LEVEL:
    case ST_DOUBLE_NAME:
      break;
  }
}

