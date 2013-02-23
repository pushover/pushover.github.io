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

// states of the level Editor
typedef enum {
  ST_INIT,
  ST_CHOOSE_LEVEL,     // initial transition state
  ST_EXIT,
} states_e;

static states_e currentState, nextState;
static window_c * window;

graphicsN_c * gr;
screen_c * screen;
levelset_c * levels;

static void changeState(void)
{
  if (currentState != nextState)
  {
    // leave old state
    switch (currentState)
    {
      case ST_INIT:
      case ST_EXIT:
        break;

      case ST_CHOOSE_LEVEL:
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
        break;

      case ST_CHOOSE_LEVEL:
        window = getEditorLevelChooserWindow(*screen, *gr, *levels);
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

void startEditor(graphicsN_c & g, screen_c & s)
{
  nextState = ST_CHOOSE_LEVEL;
  currentState = ST_INIT;
  window = 0;

  screen = &s;
  gr = &g;

  levels = new levelset_c(getHome() + "levels/", "", true);
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

          nextState = ST_EXIT;
        }
      }
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
      break;
  }
}

