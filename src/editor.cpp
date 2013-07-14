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
#include <sstream>

#include <sys/stat.h>

// states of the level Editor
typedef enum {
  ST_INIT,
  ST_CHOOSE_LEVEL,     // initial transition state
  ST_EXIT,
  ST_NEW_LEVEL,
  ST_DOUBLE_NAME,
  ST_CANT_CREATE_FILE,
  ST_INVALID_FILENAME,
  ST_INVALID_TIMEFORMAT,
  ST_DELETE_LEVEL,
  ST_SELECT_THEME,
  ST_EDIT_AUTHORS,
  ST_EDIT_AUTHORS_DEL,
  ST_EDIT_AUTHORS_ADD,
  ST_EDIT_HOME,        // edit the foreground of the level (all level related elements
  ST_EDIT_DOMINOS_SELECTOR,
  ST_EDIT_BACKGROUND,  // edit the background imagery of the level
  ST_EDIT_BG_SELECTOR,
  ST_EDIT_MENU,
  ST_EDIT_HELP,
  ST_EDIT_PLAY,
  ST_EDIT_LEVELNAME,
  ST_EDIT_TIME,
  ST_EDIT_HINT,
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

    bgTilesLayout.clear();
    bgTilesStart = 0;

    const std::vector<std::vector<uint16_t> > & pt = gr->getBgTilePatterns();

    std::vector<std::vector<bool> > used; // which tiles of the selector are already used

    uint16_t line = 0; // first empty line

    // first place the patterns
    for (size_t p = 0; p < pt.size(); p++)
    {
      uint16_t w = pt[p][0];
      uint16_t h = (pt[p].size()-1)/pt[p][0];

      if (w > B_OVL_COL) continue;

      // make sure that we fit
      if (line+h > used.size())
      {
        used.resize(line+h);

        for (size_t i = 0; i < used.size(); i++)
          if (used[i].size() != B_OVL_COL) used[i].resize(B_OVL_COL);
      }

      // find an empty place
      for (size_t y = 0; y <= line+1; y++)
        for (size_t x = 0; x < (size_t)B_OVL_COL-w+1; x++)
        {
          bool found = true;

          for (size_t xw = 0; xw < w; xw++)
            for (size_t yh = 0; yh < h; yh++)
              if (used[y+yh][x+xw])
                found = false;

          if (found)
          {
            // place big tile
            if (bgTilesLayout.size() < y+h)
            {
              bgTilesLayout.resize(y+h);

              for (size_t i = 0; i < bgTilesLayout.size(); i++)
                if (bgTilesLayout[i].size() != B_OVL_COL) bgTilesLayout[i].resize(B_OVL_COL);
            }

            for (size_t xw = 0; xw < w; xw++)
              for (size_t yh = 0; yh < h; yh++)
              {
                used[y+yh][x+xw] = true;
                bgTilesLayout[y+yh][x+xw] = pt[p][1+yh*w+xw];
              }

            if (y+h > line) line = y+h;
            y = line+1;
            break;
          }
        }
    }

    // now all the remaining tiles
    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t b = 0;
    for (size_t t = 0; t < bgTiles.size(); t++)
    {
      bool done = false;

      for (size_t yy = 0; yy < bgTilesLayout.size(); yy++)
        for (size_t xx = 0; xx < B_OVL_COL; xx++)
          if (used[yy][xx] && bgTilesLayout[yy][xx] == t)
            done = true;

      if (!done)
      {
        // find next empty cell

        while ((2*y+b < used.size()) && used[2*y+b][x])
        {
          b++;
          if (b == 2)
          {
            b = 0;
            x++;
            if (x == B_OVL_COL)
            {
              x = 0;
              y++;
            }
          }
        }

        // make sure the tile can be placed
        if (bgTilesLayout.size() <= (size_t)(y*2+b))
        {
          bgTilesLayout.resize(y*2+b+1);

          for (size_t i = 0; i < bgTilesLayout.size(); i++)
            if (bgTilesLayout[i].size() != B_OVL_COL) bgTilesLayout[i].resize(B_OVL_COL);
        }

        if (used.size() <= (size_t)(2*y+b))
        {
          used.resize(y*2+b+1);

          for (size_t i = 0; i < used.size(); i++)
            if (used[i].size() != B_OVL_COL) used[i].resize(B_OVL_COL);
        }

        bgTilesLayout[y*2+b][x] = t;
        used[y*2+b][x] = true;
      }
    }

    // mark all unused tiles with a big number
    for (size_t xw = 0; xw < B_OVL_COL; xw++)
      for (size_t yh = 0; yh < bgTilesLayout.size(); yh++)
        if (!used[yh][xw])
          bgTilesLayout[yh][xw] = (uint16_t)(-1);
  }

  if (!backgroundOverlay)
  {
    backgroundOverlay = new surface_c (2*B_OVL_FRAME+B_OVL_W*B_OVL_COL, 2*B_OVL_FRAME+B_OVL_H*B_OVL_ROW, false);
  }

  backgroundOverlay->fillRect(0, 0, backgroundOverlay->getX(), backgroundOverlay->getY(), 0, 0, 0);

  for (int y = 0; y < B_OVL_ROW; y++)
    for (int x = 0; x < B_OVL_COL; x++)
    {
      if ((y+bgTilesStart) < bgTilesLayout.size() && bgTilesLayout[y+bgTilesStart][x] < bgTiles.size())
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

        if (gr->getShowBgNumbers())
        {
          char number[5];
          snprintf(number, 4, "%i", bgTilesLayout[bgTilesStart+y][x]);
          fontParams_s pars;

          pars.font = FNT_SMALL;
          pars.alignment = ALN_CENTER;
          pars.box.w = B_OVL_W;
          pars.box.h = B_OVL_H;
          pars.box.x = B_OVL_FRAME+x*B_OVL_W;
          pars.box.y = B_OVL_FRAME+y*B_OVL_H;
          pars.shadow = 1;
          pars.color.r = 255;
          pars.color.g = 255;
          pars.color.b = 255;

          backgroundOverlay->renderText(&pars, number);
        }

        backgroundOverlay->fillRect(B_OVL_FRAME+B_OVL_W*(bgSelX)-1, B_OVL_FRAME+B_OVL_H*(bgSelY), 2, B_OVL_H*bgSelH, 255, 255, 255);
        backgroundOverlay->fillRect(B_OVL_FRAME+B_OVL_W*(bgSelX+bgSelW)-1, B_OVL_FRAME+B_OVL_H*(bgSelY), 2, B_OVL_H*bgSelH, 255, 255, 255);

        backgroundOverlay->fillRect(B_OVL_FRAME+B_OVL_W*(bgSelX), B_OVL_FRAME+B_OVL_H*(bgSelY)-1, B_OVL_W*bgSelW, 2, 255, 255, 255);
        backgroundOverlay->fillRect(B_OVL_FRAME+B_OVL_W*(bgSelX), B_OVL_FRAME+B_OVL_H*(bgSelY+bgSelH)-1, B_OVL_W*bgSelW, 2, 255, 255, 255);
      }
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
      case ST_CANT_CREATE_FILE:
      case ST_INVALID_FILENAME:
      case ST_INVALID_TIMEFORMAT:
      case ST_DOUBLE_NAME:
      case ST_EDIT_MENU:
      case ST_EDIT_HELP:
      case ST_SELECT_THEME:
      case ST_EDIT_AUTHORS:
      case ST_EDIT_AUTHORS_DEL:
      case ST_EDIT_AUTHORS_ADD:
      case ST_EDIT_LEVELNAME:
      case ST_EDIT_TIME:
      case ST_EDIT_HINT:
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

      case ST_CANT_CREATE_FILE:
        window = getMessageWindow(*screen, *gr, _("Can not create a file with the given filename"));
        break;

      case ST_INVALID_FILENAME:
        window = getMessageWindow(*screen, *gr, _("The filename you provided does contain invalid characters"));
        break;

      case ST_INVALID_TIMEFORMAT:
        window = getMessageWindow(*screen, *gr, _("The time you have given is invalid"));
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

      case ST_SELECT_THEME:
        window = getThemeSelectorWindow(*screen, *gr);
        break;

      case ST_EDIT_AUTHORS:
        window = getAuthorsWindow(*screen, *gr, l->getAuthor());
        break;

      case ST_EDIT_AUTHORS_ADD:
        window = getAuthorsAddWindow(*screen, *gr);
        break;

      case ST_EDIT_AUTHORS_DEL:
        window = getAuthorsDelWindow(*screen, *gr, l->getAuthor());
        break;

      case ST_EDIT_LEVELNAME:
        window = getLevelnameWindow(*screen, *gr, l->getName());
        break;

      case ST_EDIT_TIME:
        window = getTimeWindow(*screen, *gr, l->getTimeLeft() / 18);
        break;

      case ST_EDIT_HINT:
        window = getHintWindow(*screen, *gr, l->getHint());
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
    if (bgTilesStart >= -y)
      bgTilesStart += y;
    else
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
    if (bgTilesStart+B_OVL_ROW < bgTilesLayout.size())
      bgTilesStart++;

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

    if (bgTilesStart+B_OVL_ROW < bgTilesLayout.size())
      bgTilesStart++;
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

static bool stringValidAsFilename(const std::string s)
{
  for (size_t i = 0; i < s.length(); i++)
  {
    if (  (s[i] < '0' || s[i] > '9')
        &&(s[i] < 'a' || s[i] > 'z')
        &&(s[i] < 'A' || s[i] > 'Z')
        &&(s[i] != '_')
        &&(s[i] != '-')
       )
      return false;
  }

  return true;
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

    case ST_SELECT_THEME:
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

          switch (sel)
          {
            case 0: l->setTheme("toxcity"); break;
            case 1: l->setTheme("aztec"); break;
            case 2: l->setTheme("space"); break;
            case 3: l->setTheme("electro"); break;
            case 4: l->setTheme("greek"); break;
            case 5: l->setTheme("castle"); break;
            case 6: l->setTheme("mechanic"); break;
            case 7: l->setTheme("dungeon"); break;
            case 8: l->setTheme("japanese"); break;
            case 9: l->setTheme("cavern"); break;
            default: break;
          }

          if (sel < 10)
            for (size_t y = 0; y < l->levelY(); y++)
              for (size_t x = 0; x < l->levelX(); x++)
                for (size_t lay = 0; lay < l->getNumBgLayer(); lay++)
                  l->setBg(x, y, lay, 0);

          gr->setPaintData(l, 0, screen);

          nextState = ST_EDIT_MENU;
        }
      }

      break;

    case ST_EDIT_AUTHORS:
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
          unsigned int aut = l->getAuthor().size();

          if (sel < aut)
          {
            // don't do anything when an author was selected
            window->resetWindow();
          }
          else if (sel == aut && aut != 0)
          {
            // delete author
            nextState = ST_EDIT_AUTHORS_DEL;
          }
          else if ((sel == aut && aut == 0) || (sel == aut+1 && aut != 0))
          {
            // add a new author
            nextState = ST_EDIT_AUTHORS_ADD;
          }
          else
          {
            // escape pressed
            nextState = ST_EDIT_MENU;
          }
        }
      }
      break;

    case ST_EDIT_LEVELNAME:
      if (!window)
      {
        nextState = ST_EXIT;
      }
      else
      {
        window->handleEvent(event);

        if (window->isDone())
        {
          if (!window->hasEscaped())
          {
            l->setName(window->getText());
          }
          nextState = ST_EDIT_MENU;
        }
      }
      break;

    case ST_EDIT_HINT:
      if (!window)
      {
        nextState = ST_EXIT;
      }
      else
      {
        window->handleEvent(event);

        if (window->isDone())
        {
          if (!window->hasEscaped())
          {
            l->setHint(window->getText());
          }
          nextState = ST_EDIT_MENU;
        }
      }
      break;

    case ST_EDIT_TIME:
      if (!window)
      {
        nextState = ST_EXIT;
      }
      else
      {
        window->handleEvent(event);

        if (window->isDone())
        {
          if (!window->hasEscaped())
          {
            std::istringstream tm(window->getText());

            bool validFormat = true;

            unsigned int timeMinutes;
            tm >> timeMinutes;
            if (tm.get() != ':')
              validFormat = false;
            unsigned int timeSeconds;
            tm >> timeSeconds;
            if (!tm.eof() || !tm)
              validFormat = false;

            if (validFormat)
            {
              l->setTimeLeft((timeMinutes*60+timeSeconds)*18);
              nextState = ST_EDIT_MENU;
            }
            else
            {
              nextState = ST_INVALID_TIMEFORMAT;
              messageContinue = ST_EDIT_MENU;
            }
          }
          else
          {
            nextState = ST_EDIT_MENU;
          }
        }
      }
      break;

    case ST_EDIT_AUTHORS_ADD:
      if (!window)
      {
        nextState = ST_EXIT;
      }
      else
      {
        window->handleEvent(event);

        if (window->isDone())
        {
          if (!window->hasEscaped())
          {
            l->getAuthor().push_back(window->getText());
          }
          nextState = ST_EDIT_AUTHORS;
        }
      }
      break;

    case ST_EDIT_AUTHORS_DEL:
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

          if (sel < l->getAuthor().size())
            l->getAuthor().erase(l->getAuthor().begin()+sel);

          nextState = ST_EDIT_AUTHORS;
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
          if (!window->hasEscaped())
          {
            if (!stringValidAsFilename(window->getText()))
            {
              nextState = ST_INVALID_FILENAME;;
              messageContinue = ST_CHOOSE_LEVEL;
            }
            else
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
                  std::ofstream f(fname.c_str());

                  if (!f)
                  {
                    nextState = ST_CANT_CREATE_FILE;
                    messageContinue = ST_CHOOSE_LEVEL;
                  }
                  else
                  {
                    levelData_c l;
                    l.setName(window->getText());
                    l.setAuthor(userName);
                    l.save(f);
                  }
                }

                if (nextState != ST_CANT_CREATE_FILE)
                {
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
    case ST_CANT_CREATE_FILE:
    case ST_INVALID_FILENAME:
    case ST_INVALID_TIMEFORMAT:

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
              nextState = ST_SELECT_THEME;
              break;

            case 1:
              nextState = ST_EDIT_LEVELNAME;
              break;

            case 2:
              nextState = ST_EDIT_TIME;
              break;

            case 3:
              nextState = ST_EDIT_HINT;
              break;

            case 4:
              nextState = ST_EDIT_AUTHORS;
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
            if (bgTilesStart+bgSelY+i < bgTilesLayout.size())
              bgPattern[i][j] = bgTilesLayout[bgTilesStart+bgSelY+i][bgSelX+j];
            else
              bgPattern[i][j] = 0;
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
          // toggle platforms
          for (uint8_t x = gr->getCursorX(); x < gr->getCursorX()+gr->getCursorW(); x++)
            for (uint8_t y = gr->getCursorY(); y < gr->getCursorY()+gr->getCursorH(); y++)
              l->setPlatform(x, y, !l->getPlatform(x, y));
        }
        else
        {
          // set all platforms, or remove all platforms if all platforms are already set
          bool allSet = true;
          for (uint8_t x = gr->getCursorX(); x < gr->getCursorX()+gr->getCursorW(); x++)
            for (uint8_t y = gr->getCursorY(); y < gr->getCursorY()+gr->getCursorH(); y++)
              allSet = allSet && l->getPlatform(x, y);
          for (uint8_t x = gr->getCursorX(); x < gr->getCursorX()+gr->getCursorW(); x++)
            for (uint8_t y = gr->getCursorY(); y < gr->getCursorY()+gr->getCursorH(); y++)
              l->setPlatform(x, y, !allSet);
        }
      }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == 'h')
      {
        if (SDL_GetKeyState(NULL)[SDLK_LSHIFT] || SDL_GetKeyState(NULL)[SDLK_RSHIFT])
        {
          // toggle ladders
          for (uint8_t x = gr->getCursorX(); x < gr->getCursorX()+gr->getCursorW(); x++)
            for (uint8_t y = gr->getCursorY(); y < gr->getCursorY()+gr->getCursorH(); y++)
              l->setLadder(x, y, !l->getLadder(x, y));
        }
        else
        {
          // set all ladders, or remove all ladders if all ladders are already set
          bool allSet = true;
          for (uint8_t x = gr->getCursorX(); x < gr->getCursorX()+gr->getCursorW(); x++)
            for (uint8_t y = gr->getCursorY(); y < gr->getCursorY()+gr->getCursorH(); y++)
              allSet = allSet && l->getLadder(x, y);
          for (uint8_t x = gr->getCursorX(); x < gr->getCursorX()+gr->getCursorW(); x++)
            for (uint8_t y = gr->getCursorY(); y < gr->getCursorY()+gr->getCursorH(); y++)
              l->setLadder(x, y, !allSet);
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

            gr->markAllDirtyBg();
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
        if (bgLayer < 255)
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
            for (size_t a = l->getNumBgLayer(); a > bgLayer; a--)
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
            for (size_t a = bgLayer; a < l->getNumBgLayer()-1; a++)
              l->setBg(gr->getCursorX()+i, gr->getCursorY()+j, a, l->getBg(gr->getCursorX()+i, gr->getCursorY()+j, a+1));

            l->setBg(gr->getCursorX()+i, gr->getCursorY()+j, l->getNumBgLayer()-1, 0);
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

      // toggle number visibility
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == 'n')
      {
        gr->setShowBgNumbers(!gr->getShowBgNumbers());
      }

      // print out the current selection for a "big tile" for the theme
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == 'l')
      {
        std::cout << "  { " << (int)gr->getCursorW() << "   ";

        for (size_t j = 0; j < gr->getCursorH(); j++)
          for (size_t i = 0; i < gr->getCursorW(); i++)
            std::cout << ", " << l->getBg(gr->getCursorX()+i, gr->getCursorY()+j, bgLayer);

        std::cout << " }, \n";
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
    case ST_CANT_CREATE_FILE:
    case ST_INVALID_FILENAME:
    case ST_INVALID_TIMEFORMAT:
    case ST_EDIT_MENU:
    case ST_EDIT_HELP:
    case ST_EDIT_AUTHORS:
    case ST_EDIT_AUTHORS_ADD:
    case ST_EDIT_AUTHORS_DEL:
    case ST_EDIT_LEVELNAME:
    case ST_EDIT_TIME:
    case ST_EDIT_HINT:
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

    case ST_SELECT_THEME:
      break;

  }
}

