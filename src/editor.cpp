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
#include "recorder.h"

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
  ST_EDIT_HOME,        // edit the foreground of the level (all level related elements
  ST_EDIT_DOMINOS_SELECTOR,
  ST_EDIT_BACKGROUND,  // edit the background imagery of the level
  ST_EDIT_BG_SELECTOR,
  ST_EDIT_MENU,
  ST_EDIT_HELP,
  ST_EDIT_PLAY,
} states_e;

static states_e currentState, nextState, messageContinue;
static window_c * window;

static graphicsN_c * gr;
static screen_c * screen;
static levelset_c * levels;
static levelPlayer_c * l;
static std::string userName;
static std::string levelFileName;
static std::string levelName;
static recorder_c rec;
static ant_c * a;
static DominoType dt;
static surface_c * dominoOverlay = 0;
static surface_c * backgroundOverlay = 0;
static uint8_t bgLayer = 0;
static std::vector<std::vector<uint16_t> > bgPattern;

static uint8_t bgSelX = 0;
static uint8_t bgSelY = 0;
static uint8_t bgSelW = 1;
static uint8_t bgSelH = 1;
static std::vector<std::vector<uint16_t> > bgTilesLayout; // contains the layout of the bg tiles as used in the bg tile selector
std::string bgTileLayoutTheme;
static uint16_t bgTilesStart = 0;

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

#define D_OVL_W 50
#define D_OVL_H 70
#define D_OVL_COL 5
#define D_OVL_ROW 3
#define D_OVL_FRAME 5

void updateDominoOverlay(void)
{
  if (!dominoOverlay)
  {
    dominoOverlay = new surface_c (2*D_OVL_FRAME+D_OVL_W*D_OVL_COL, 2*D_OVL_FRAME+D_OVL_H*D_OVL_ROW, false);
  }

  dominoOverlay->fillRect(0, 0, dominoOverlay->getX(), dominoOverlay->getY(), 0, 0, 0);

  for (int i = 0; i < DominoTypeLastNormal; i++)
  {
    if (dt == (DominoType(i+1)))
    {
      dominoOverlay->fillRect(D_OVL_FRAME+D_OVL_W*(i % D_OVL_COL), D_OVL_FRAME+D_OVL_H*(i / D_OVL_COL), D_OVL_W, D_OVL_H, 100, 100, 100);
    }
    else
    {
      dominoOverlay->fillRect(D_OVL_FRAME+D_OVL_W*(i % D_OVL_COL), D_OVL_FRAME+D_OVL_H*(i / D_OVL_COL), D_OVL_W, D_OVL_H, 0, 0, 0);
    }

    dominoOverlay->blit(*gr->getHelpDominoImage((DominoType)(i+1)), D_OVL_W*(i%D_OVL_COL)-75, 62+D_OVL_H*(i/D_OVL_COL));
  }

  gr->setOverlay(dominoOverlay);
}

#define B_OVL_W gr->blockX()
#define B_OVL_H (gr->blockY()/2)
#define B_OVL_COL 17
#define B_OVL_ROW 22
#define B_OVL_FRAME 5

void updateBackgroundOverlay(void)
{
  const std::vector<surface_c *> bgTiles = gr->getBgTiles();

  if (bgTileLayoutTheme != l->getTheme())
  {
    bgTileLayoutTheme = l->getTheme();

    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t b = 0;

    for (size_t t = 0; t < bgTiles.size(); t++)
    {
      if (bgTilesLayout.size() <= (size_t)(y*2+b))
      {
        bgTilesLayout.resize(y*2+b+1);
        bgTilesLayout.back().resize(B_OVL_COL);
      }

      bgTilesLayout[y*2+b][x] = t;

      b++;
      if (b == 2)
      {
        b = 0;
        x++;
        if (x == B_OVL_COL)
        {
          x = 0;
          y++;

          if (y == B_OVL_ROW)
            break;
        }
      }
    }
  }

  if (!backgroundOverlay)
  {
    backgroundOverlay = new surface_c (2*B_OVL_FRAME+B_OVL_W*B_OVL_COL, 2*B_OVL_FRAME+B_OVL_H*B_OVL_ROW, false);
  }

  backgroundOverlay->fillRect(0, 0, backgroundOverlay->getX(), backgroundOverlay->getY(), 0, 0, 0);

  for (int y = 0; y < B_OVL_ROW; y++)
    for (int x = 0; x < B_OVL_COL; x++)
    {
      for (int yc = 0; yc < 3; yc++)
        for (int xc = 0; xc < 5; xc++)
        {
          if ((xc+5*x+yc+3*y)%2)
            backgroundOverlay->fillRect(B_OVL_FRAME+x*B_OVL_W+xc*8, B_OVL_FRAME+y*B_OVL_H+yc*8, 8, 8, 85, 85, 85);
          else
            backgroundOverlay->fillRect(B_OVL_FRAME+x*B_OVL_W+xc*8, B_OVL_FRAME+y*B_OVL_H+yc*8, 8, 8, 2*85, 2*85, 2*85);
        }

      if ((size_t)(bgTilesStart+y) < bgTilesLayout.size())
        backgroundOverlay->blitBlock(*(bgTiles[bgTilesLayout[bgTilesStart+y][x]]), B_OVL_FRAME+x*B_OVL_W, B_OVL_FRAME+y*B_OVL_H);

      backgroundOverlay->fillRect(B_OVL_FRAME+B_OVL_W*(bgSelX)-1, B_OVL_FRAME+B_OVL_H*(bgSelY), 2, B_OVL_H*bgSelH, 255, 255, 255);
      backgroundOverlay->fillRect(B_OVL_FRAME+B_OVL_W*(bgSelX+bgSelW)-1, B_OVL_FRAME+B_OVL_H*(bgSelY), 2, B_OVL_H*bgSelH, 255, 255, 255);

      backgroundOverlay->fillRect(B_OVL_FRAME+B_OVL_W*(bgSelX), B_OVL_FRAME+B_OVL_H*(bgSelY)-1, B_OVL_W*bgSelW, 2, 255, 255, 255);
      backgroundOverlay->fillRect(B_OVL_FRAME+B_OVL_W*(bgSelX), B_OVL_FRAME+B_OVL_H*(bgSelY+bgSelH)-1, B_OVL_W*bgSelW, 2, 255, 255, 255);
    }

  gr->setOverlay(backgroundOverlay);
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
      case ST_EDIT_BACKGROUND:
        break;

      case ST_EDIT_PLAY:
        gr->setPaintData(l, 0, screen);
        break;

      case ST_CHOOSE_LEVEL:
      case ST_NEW_LEVEL:
      case ST_DELETE_LEVEL:
      case ST_DOUBLE_NAME:
      case ST_EDIT_MENU:
      case ST_EDIT_HELP:
        delete window;
        window = 0;
        break;

      case ST_EDIT_DOMINOS_SELECTOR:
      case ST_EDIT_BG_SELECTOR:
        gr->setOverlay(0);
        break;
    }

    currentState = nextState;

    // enter new state
    switch (currentState)
    {
      case ST_INIT:
      case ST_EXIT:
      case ST_EDIT_HOME:
      case ST_EDIT_BACKGROUND:
        break;

      case ST_CHOOSE_LEVEL:
        loadLevels();
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

      case ST_EDIT_MENU:
        window = getEditorMenu(*screen, *gr);
        break;

      case ST_EDIT_HELP:
        window = getEditorHelp(*screen, *gr);
        break;

      case ST_EDIT_PLAY:
        gr->setPaintData(l, a, screen);
        break;

      case ST_EDIT_DOMINOS_SELECTOR:
        updateDominoOverlay();
        break;

      case ST_EDIT_BG_SELECTOR:
        updateBackgroundOverlay();
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

  gr->setEditorMode(false);
  gr->setShowGrid(false);
}

void startEditor(graphicsN_c & g, screen_c & s, levelPlayer_c & lp, ant_c & ant, const std::string & user)
{
  nextState = ST_CHOOSE_LEVEL;
  currentState = ST_INIT;
  window = 0;

  screen = &s;
  gr = &g;
  gr->setEditorMode(true);

  loadLevels();

  l = &lp;
  a = &ant;
  userName = user;

  gr->setCursor(l->levelX()/2, l->levelY()/2, 1, 1);

  dt = DominoTypeStandard;
  bgLayer = 0;
}

static bool handleCommonKeys(const SDL_Event & event)
{
  Uint8 *keystate = SDL_GetKeyState(NULL);

  if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_UP)
  {
    if (keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT])
    {
      gr->setCursor(gr->getCursorX(), gr->getCursorY(), gr->getCursorW(), gr->getCursorH()-1);
      return true;
    }
    else
    {
      gr->setCursor(gr->getCursorX(), gr->getCursorY()-1, gr->getCursorW(), gr->getCursorH());
      return true;
    }
  }

  if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_DOWN)
  {
    if (keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT])
    {
      gr->setCursor(gr->getCursorX(), gr->getCursorY(), gr->getCursorW(), gr->getCursorH()+1);
      return true;
    }
    else
    {
      gr->setCursor(gr->getCursorX(), gr->getCursorY()+1, gr->getCursorW(), gr->getCursorH());
      return true;
    }
  }

  if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LEFT)
  {
    if (keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT])
    {
      gr->setCursor(gr->getCursorX(), gr->getCursorY(), gr->getCursorW()-1, gr->getCursorH());
      return true;
    }
    else
    {
      gr->setCursor(gr->getCursorX()-1, gr->getCursorY(), gr->getCursorW(), gr->getCursorH());
      return true;
    }
  }

  if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RIGHT)
  {
    if (keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT])
    {
      gr->setCursor(gr->getCursorX(), gr->getCursorY(), gr->getCursorW()+1, gr->getCursorH());
      return true;
    }
    else
    {
      gr->setCursor(gr->getCursorX()+1, gr->getCursorY(), gr->getCursorW(), gr->getCursorH());
      return true;
    }
  }

  if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_HOME)
  {
    if (keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT])
    {
      gr->setCursor(gr->getCursorX(), gr->getCursorY(), 1, gr->getCursorH());
      return true;
    }
    else
    {
      gr->setCursor(0, gr->getCursorY(), gr->getCursorW(), gr->getCursorH());
      return true;
    }
  }

  if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_END)
  {
    if (keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT])
    {
      gr->setCursor(gr->getCursorX(), gr->getCursorY(), l->levelX(), gr->getCursorH());
      return true;
    }
    else
    {
      gr->setCursor(l->levelX()-1, gr->getCursorY(), gr->getCursorW(), gr->getCursorH());
      return true;
    }
  }

  if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_PAGEUP)
  {
    if (keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT])
    {
      gr->setCursor(gr->getCursorX(), gr->getCursorY(), gr->getCursorW(), 1);
      return true;
    }
    else
    {
      gr->setCursor(gr->getCursorX(), 0, gr->getCursorW(), gr->getCursorH());
      return true;
    }
  }

  if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_PAGEDOWN)
  {
    if (keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT])
    {
      gr->setCursor(gr->getCursorX(), gr->getCursorY(), gr->getCursorW(), l->levelY());
      return true;
    }
    else
    {
      gr->setCursor(gr->getCursorX(), l->levelY()-1, gr->getCursorW(), gr->getCursorH());
      return true;
    }
  }

  if (event.type == SDL_KEYDOWN && event.key.keysym.sym == 'g')
  {
    // toggle grid
    gr->setShowGrid(!gr->getShowGrid());
  }

  return false;
}

static void setBgCursor(int8_t x, int8_t y, int8_t w, int8_t h)
{
  if (x < 0)
  {
    w += x;
    x = 0;
  }

  if (y < 0)
  {
    h += y;
    y = 0;
  }

  if (w < 1) w = 1;
  if (h < 1) h = 1;

  if (x >= B_OVL_COL)
  {
    x = B_OVL_COL-1;
    w = 1;
  }

  if (y >= B_OVL_ROW)
  {
    y = B_OVL_ROW-1;
    h = 1;
  }

  if (x+w > B_OVL_COL)
  {
    w = B_OVL_COL-x;
  }

  if (y+h > B_OVL_ROW)
  {
    h = B_OVL_ROW-y;
  }

  bgSelX = x;
  bgSelY = y;
  bgSelW = w;
  bgSelH = h;

  updateBackgroundOverlay();
}

static void setDominoStatus(DominoType dt)
{
  switch (dt)
  {
    case DominoTypeStandard:   gr->setStatus(_("Selected Standard domino")); break;
    case DominoTypeStopper:    gr->setStatus(_("Selected Stopper domino")); break;
    case DominoTypeSplitter:   gr->setStatus(_("Selected Splitter domino")); break;
    case DominoTypeExploder:   gr->setStatus(_("Selected Exploder domino")); break;
    case DominoTypeDelay:      gr->setStatus(_("Selected Delay domino")); break;
    case DominoTypeTumbler:    gr->setStatus(_("Selected Tumbler domino")); break;
    case DominoTypeBridger:    gr->setStatus(_("Selected Bridger domino")); break;
    case DominoTypeVanish:     gr->setStatus(_("Selected Vanisher domino")); break;
    case DominoTypeTrigger:    gr->setStatus(_("Selected Trigger domino")); break;
    case DominoTypeAscender:   gr->setStatus(_("Selected Ascender domino")); break;
    case DominoTypeConnectedA: gr->setStatus(_("Selected Connected A domino")); break;
    case DominoTypeConnectedB: gr->setStatus(_("Selected Connected B domino")); break;
    case DominoTypeCounter1:   gr->setStatus(_("Selected Counter Stopper 1 domino")); break;
    case DominoTypeCounter2:   gr->setStatus(_("Selected Counter Stopper 2 domino")); break;
    case DominoTypeCounter3:   gr->setStatus(_("Selected Counter Stopper 3 domino")); break;
    default:                   gr->setStatus(_("Selected unknown domino")); break;
  }
}

static void updateEditPlane()
{

  gr->clearEditPlane();

  if (bgPattern.size() > 0)
  {
    gr->setEditPlaneLayer(bgLayer);

    for (size_t i = 0; i < gr->getCursorW(); i++)
      for (size_t j = 0; j < gr->getCursorH(); j++)
        gr->setEditPlaneTile(gr->getCursorX()+i, gr->getCursorY()+j, bgPattern[j % bgPattern.size()][i % bgPattern[0].size()]);
  }
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
            levelName = levels->getLevelNames()[sel];
            levelFileName = levels->getFilename(levelName);

            levels->loadLevel(*l, levelName, "");
            nextState = ST_EDIT_HOME;
            gr->setPaintData(l, 0, screen);
            bgLayer = 0;
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
            levelName = window->getText();
            levels->loadLevel(*l, levelName, "");

            gr->setPaintData(l, 0, screen);

            levelFileName = levels->getFilename(levelName);
            nextState = ST_EDIT_HOME;
            bgLayer = 0;
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

    case ST_EDIT_MENU:
      if (!window)
      {
        nextState = ST_EXIT;
      }
      else
      {
        window->handleEvent(event);

        if (window->isDone())
        {
          switch (window->getSelection())
          {
            case 0:
              // theme
              nextState = ST_EDIT_HOME;
              break;

            case 1:
              // name
              nextState = ST_EDIT_HOME;
              break;

            case 2:
              // time
              nextState = ST_EDIT_HOME;
              break;

            case 3:
              // hint
              nextState = ST_EDIT_HOME;
              break;

            case 4:
              // authors
              nextState = ST_EDIT_HOME;
              break;

            case 5:
              // play
              {
                std::ofstream out(levelFileName.c_str());
                l->save(out);
              }
              loadLevels();
              a->initForLevel();
              gr->setShowGrid(false);
              nextState = ST_EDIT_PLAY;
              break;

            case 6:
              // save
              {
                std::ofstream out(levelFileName.c_str());
                l->save(out);
              }
              loadLevels();
              nextState = messageContinue;
              break;

            case 7:
              // another level
              nextState = ST_CHOOSE_LEVEL;
              break;

            case 8:
              // leave
              nextState = ST_EXIT;
              break;

            default:
              nextState = messageContinue;
              break;
          }
        }
      }
      break;

    case ST_EDIT_HELP:
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

    case ST_EDIT_DOMINOS_SELECTOR:

      if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_LCTRL)
      {
        setDominoStatus(dt);
        nextState = ST_EDIT_HOME;
      }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_UP)
        if (dt > D_OVL_COL)
        {
          dt = (DominoType)(dt-D_OVL_COL);
          updateDominoOverlay();
        }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LEFT)
        if (dt > 1)
        {
          dt = (DominoType)(dt-1);
          updateDominoOverlay();
        }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_DOWN)
        if (dt+D_OVL_COL <= DominoTypeLastNormal )
        {
          dt = (DominoType)(dt+D_OVL_COL);
          updateDominoOverlay();
        }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RIGHT)
        if (dt+1 <= DominoTypeLastNormal )
        {
          dt = (DominoType)(dt+1);
          updateDominoOverlay();
        }

      break;

    case ST_EDIT_BG_SELECTOR:

      if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_LCTRL)
      {
        nextState = ST_EDIT_BACKGROUND;

        bgPattern.resize(bgSelH);
        for (size_t i = 0; i < bgSelH; i++)
        {
          bgPattern[i].resize(bgSelW);

          for (size_t j = 0; j < bgSelW; j++)
            bgPattern[i][j] = bgTilesLayout[bgSelY+i][bgSelX+j];
        }

        updateEditPlane();
      }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
      {
        bgPattern.clear();
        nextState = ST_EDIT_BACKGROUND;
        updateEditPlane();
      }

      {
        Uint8 *keystate = SDL_GetKeyState(NULL);

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_UP)
        {
          if (keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT])
            setBgCursor(bgSelX, bgSelY, bgSelW, bgSelH-1);
          else
            setBgCursor(bgSelX, bgSelY-1, bgSelW, bgSelH);
        }

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_DOWN)
        {
          if (keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT])
            setBgCursor(bgSelX, bgSelY, bgSelW, bgSelH+1);
          else
            setBgCursor(bgSelX, bgSelY+1, bgSelW, bgSelH);
        }

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LEFT)
        {
          if (keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT])
            setBgCursor(bgSelX, bgSelY, bgSelW-1, bgSelH);
          else
            setBgCursor(bgSelX-1, bgSelY, bgSelW, bgSelH);
        }

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RIGHT)
        {
          if (keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT])
            setBgCursor(bgSelX, bgSelY, bgSelW+1, bgSelH);
          else
            setBgCursor(bgSelX+1, bgSelY, bgSelW, bgSelH);
        }

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_HOME)
        {
          if (keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT])
            setBgCursor(bgSelX, bgSelY, 1, bgSelH);
          else
            setBgCursor(0, bgSelY, bgSelW, bgSelH);
        }

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_END)
        {
          if (keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT])
            setBgCursor(bgSelX, bgSelY, B_OVL_COL, bgSelH);
          else
            setBgCursor(B_OVL_COL-1, bgSelY, bgSelW, bgSelH);
        }

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_PAGEUP)
        {
          if (keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT])
            setBgCursor(bgSelX, bgSelY, bgSelW, 1);
          else
            setBgCursor(bgSelX, 0, bgSelW, bgSelH);
        }

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_PAGEDOWN)
        {
          if (keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT])
            setBgCursor(bgSelX, bgSelY, bgSelW, B_OVL_ROW);
          else
            setBgCursor(bgSelX, B_OVL_ROW-1, bgSelW, bgSelH);
        }
      }

      break;

    case ST_EDIT_HOME:

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
      {
        nextState = ST_EDIT_MENU;
        messageContinue = ST_EDIT_HOME;
      }
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F1)
      {
        nextState = ST_EDIT_HELP;
        messageContinue = ST_EDIT_HOME;
      }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_TAB)
      {
        nextState = ST_EDIT_BACKGROUND;
        gr->setStatus(_("Background editing mode"));
      }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == '-')
      {
        if (SDL_GetKeyState(NULL)[SDLK_LSHIFT] || SDL_GetKeyState(NULL)[SDLK_RSHIFT])
        {
          bool val = !l->getPlatform(gr->getCursorX(), gr->getCursorY());

          for (uint8_t x = gr->getCursorX(); x < gr->getCursorX()+gr->getCursorW(); x++)
            for (uint8_t y = gr->getCursorY(); y < gr->getCursorY()+gr->getCursorH(); y++)
              l->setPlatform(x, y, val);
        }
        else
        {
          // toggle platforms
          for (uint8_t x = gr->getCursorX(); x < gr->getCursorX()+gr->getCursorW(); x++)
            for (uint8_t y = gr->getCursorY(); y < gr->getCursorY()+gr->getCursorH(); y++)
              l->setPlatform(x, y, !l->getPlatform(x, y));
        }
      }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == 'h')
      {
        if (SDL_GetKeyState(NULL)[SDLK_LSHIFT] || SDL_GetKeyState(NULL)[SDLK_RSHIFT])
        {
          bool val = !l->getLadder(gr->getCursorX(), gr->getCursorY());

          // toggle ladders
          for (uint8_t x = gr->getCursorX(); x < gr->getCursorX()+gr->getCursorW(); x++)
            for (uint8_t y = gr->getCursorY(); y < gr->getCursorY()+gr->getCursorH(); y++)
              l->setLadder(x, y, val);
        }
        else
        {
          // toggle ladders
          for (uint8_t x = gr->getCursorX(); x < gr->getCursorX()+gr->getCursorW(); x++)
            for (uint8_t y = gr->getCursorY(); y < gr->getCursorY()+gr->getCursorH(); y++)
              l->setLadder(x, y, !l->getLadder(x, y));
        }
      }

      // door placement
      if (  (event.type == SDL_KEYDOWN && event.key.keysym.sym == 'o')
          ||(event.type == SDL_KEYDOWN && event.key.keysym.sym == 'i'))
      {
        if (gr->getCursorH() != 2 || gr->getCursorW() != 1)
          gr->setStatus(_("Doors can only beplaced in 1x2 sized boxes"));
        else if ((size_t)(gr->getCursorY()+2) >= l->levelY())
          gr->setStatus(_("Doors can not be placed that low down in the level"));
        else
        {
          int x, y;

          if (event.type == SDL_KEYDOWN && event.key.keysym.sym == 'i')
          {
            x = l->getExitX();
            y = l->getExitY();
          }
          else
          {
            x = l->getEntryX();
            y = l->getEntryY();
          }

          if (x == gr->getCursorX() && abs(y-(gr->getCursorY()+2)) < 2)
            gr->setStatus(_("Doors must not overlap"));
          else
          {
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == 'i')
              l->setEntry(gr->getCursorX(), gr->getCursorY()+2);
            else
              l->setExit(gr->getCursorX(), gr->getCursorY()+2);
          }
        }
      }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LCTRL)
      {
        nextState = ST_EDIT_DOMINOS_SELECTOR;
      }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == ' ')
      {
        DominoType val = dt;
        if (l->getDominoType(gr->getCursorX(), gr->getCursorY()) != DominoTypeEmpty)
          val = DominoTypeEmpty;

        for (uint8_t x = gr->getCursorX(); x < gr->getCursorX()+gr->getCursorW(); x++)
          for (uint8_t y = gr->getCursorY(); y < gr->getCursorY()+gr->getCursorH(); y++)
            l->setDominoType(x, y, val);
      }

      handleCommonKeys(event);

      break;

    case ST_EDIT_BACKGROUND:

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
      {
        nextState = ST_EDIT_MENU;
        messageContinue = ST_EDIT_BACKGROUND;
        gr->setForegroundVisibility(true);
      }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F1)
      {
        nextState = ST_EDIT_HELP;
        messageContinue = ST_EDIT_BACKGROUND;
      }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_TAB)
      {
        nextState = ST_EDIT_HOME;
        gr->setStatus(_("Foreground editing mode"));
        gr->setForegroundVisibility(true);
      }

      // toggle foreground visibilty
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == 'f')
      {
        gr->setForegroundVisibility(!gr->getForegroundVisibility());
      }

      // choose layer to edit
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == '+')
      {
        if (bgLayer < 8)
        {
          bgLayer++;
          gr->setBgDrawLayer(bgLayer);
          updateEditPlane();
        }
      }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == '-')
      {
        if (bgLayer > 0)
        {
          bgLayer--;
          gr->setBgDrawLayer(bgLayer);
          updateEditPlane();
        }
      }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_INSERT)
      {
        for (size_t i = 0; i < gr->getCursorW(); i++)
          for (size_t j = 0; j < gr->getCursorH(); j++)
          {
            for (size_t a = 7; a > bgLayer; a--)
              l->setBg(gr->getCursorX()+i, gr->getCursorY()+j, a, l->getBg(gr->getCursorX()+i, gr->getCursorY()+j, a-1));

            l->setBg(gr->getCursorX()+i, gr->getCursorY()+j, bgLayer, 0);
          }

        gr->markAllDirtyBg();
      }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_DELETE)
      {
        for (size_t i = 0; i < gr->getCursorW(); i++)
          for (size_t j = 0; j < gr->getCursorH(); j++)
          {
            for (size_t a = bgLayer; a < 7; a++)
              l->setBg(gr->getCursorX()+i, gr->getCursorY()+j, a, l->getBg(gr->getCursorX()+i, gr->getCursorY()+j, a+1));

            l->setBg(gr->getCursorX()+i, gr->getCursorY()+j, 7, 0);
          }

        gr->markAllDirtyBg();
      }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F2) gr->setBgDrawMode(0);
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F3) gr->setBgDrawMode(1);
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F4) gr->setBgDrawMode(2);

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LCTRL)
      {
        nextState = ST_EDIT_BG_SELECTOR;
      }

      if (handleCommonKeys(event))
      {
        // cursor has changed, redo the pattern
        updateEditPlane();
      }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == ' ')
      {
        // place tiles

        if (bgPattern.size())
        {
          for (size_t i = 0; i < gr->getCursorW(); i++)
            for (size_t j = 0; j < gr->getCursorH(); j++)
              l->setBg(gr->getCursorX()+i, gr->getCursorY()+j, bgLayer, bgPattern[j % bgPattern.size()][i % bgPattern[0].size()]);

          gr->markAllDirtyBg();
        }
      }

      // copy existing background
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == 'c')
      {
        bgPattern.resize(gr->getCursorH());
        for (size_t j = 0; j < gr->getCursorH(); j++)
        {
          bgPattern[j].resize(gr->getCursorW());
          for (size_t i = 0; i < gr->getCursorW(); i++)
            bgPattern[j][i] = l->getBg(gr->getCursorX()+i, gr->getCursorY()+j, bgLayer);
        }

        updateEditPlane();
      }

      // cut the cursor shape
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == 'x')
      {
        bgPattern.resize(gr->getCursorH());
        for (size_t j = 0; j < gr->getCursorH(); j++)
        {
          bgPattern[j].resize(gr->getCursorW());
          for (size_t i = 0; i < gr->getCursorW(); i++)
          {
            bgPattern[j][i] = l->getBg(gr->getCursorX()+i, gr->getCursorY()+j, bgLayer);
            l->setBg(gr->getCursorX()+i, gr->getCursorY()+j, bgLayer, 0);
          }
        }

        updateEditPlane();
      }

      break;


    case ST_EDIT_PLAY:
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
        nextState = ST_EDIT_HOME;
        levels->loadLevel(*l, levelName, "");
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
    case ST_NEW_LEVEL:
    case ST_DELETE_LEVEL:
    case ST_DOUBLE_NAME:
    case ST_EDIT_MENU:
    case ST_EDIT_HELP:
      break;


    case ST_EDIT_PLAY:
      {
        unsigned int keyMask = getKeyMask();
        rec.addEvent(keyMask);
        uint16_t failReason = a->performAnimation(keyMask);

        if (l->levelInactive())
        {
          switch (failReason) {
            case LS_undecided:
              break;
            case LS_solved:
              rec.save("levels/"+levelName+"_");
              // intentionally fall throgh
            default:
              nextState = ST_EDIT_HOME;
              levels->loadLevel(*l, levelName, "");
              break;
          }
        }
      }
      // intentionally fall though to repaint the level

    case ST_EDIT_HOME:
    case ST_EDIT_BACKGROUND:
    case ST_EDIT_DOMINOS_SELECTOR:
    case ST_EDIT_BG_SELECTOR:
      gr->drawLevel();
      break;

  }
}

