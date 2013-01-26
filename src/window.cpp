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

#include "window.h"

#include "screen.h"
#include "graphics.h"
#include "levelset.h"
#include "textsections.h"
#include "solvedmap.h"
#include "leveldata.h"
#include "config.h"

#include <SDL.h>

#include <stdexcept>
#include <libintl.h>

// Colour for normal text
#define TXT_COL_R 112
#define TXT_COL_G 39
#define TXT_COL_B 0

// Colour for selected text in list windows
#define SEL_COL_R 255
#define SEL_COL_G 255
#define SEL_COL_B 255

// Colour for highlighted text in list windows
#define HIL_COL_R ((TXT_COL_R+2*255)/3)
#define HIL_COL_G ((TXT_COL_G+2*255)/3)
#define HIL_COL_B ((TXT_COL_B+2*255)/3)

// colour for partially solved levels
#define SO1_COL_R 221
#define SO1_COL_G 179
#define SO1_COL_B 0

// colour for solved levels
#define SOL_COL_R ((TXT_COL_R+ 70)/2)
#define SOL_COL_G ((TXT_COL_G+200)/2)
#define SOL_COL_B ((TXT_COL_B+ 70)/2)

// the colour for the help text and the arrows in the help window
#define HLP_COL_R 221
#define HLP_COL_G 179
#define HLP_COL_B 0

// colour for separator in list window
#define SEP_COL_R 120
#define SEP_COL_G 90
#define SEP_COL_B 60

window_c::window_c(unsigned char x_, unsigned char y_, unsigned char w_, unsigned char h_, surface_c & s, graphics_c & g) : x(x_), y(y_), w(w_), h(h_), surf(s), gr(g) {

  if (w < 2 || h < 2) return;

  clearInside();

  done = false;
}

void window_c::clearInside(void) {

  for (unsigned int i = 0; i < w; i++)
    for (unsigned int j = 0; j < h; j++) {

      int xp = 1;
      int yp = 1;

      if (i == 0) xp = 0;
      if (i+1 == w) xp = 2;

      if (j == 0) yp = 0;
      if (j+1 == h) yp = 2;

      surf.blitBlock(gr.getBoxBlock(yp*3+xp), (x+i)*gr.blockX(), (y+j)*gr.blockY());
      surf.markDirty(x+i, y+j);
    }
}

window_c::~window_c(void) {
  for (unsigned int i = 0; i < w; i++)
    for (unsigned int j = 0; j < h; j++)
      surf.markDirty(x+i, y+j);
}

#define NUM_DOMINOS 12
static struct {
  uint16_t numDominos;
  uint16_t dominos[3];
  uint16_t boxWidth;
  uint16_t spacing;
  std::string text;
} dominoHelp[NUM_DOMINOS] = {
  { 1, {levelData_c::DominoTypeStandard},   50,  0, _("Standard: nothing special about this stone, it simply falls") },
  { 1, {levelData_c::DominoTypeStopper},    50,  0, _("Blocker: can not fall, only stone allowed to stand at level end") },
  { 1, {levelData_c::DominoTypeSplitter},   50,  0, _("Splitter: when something falls on its top it will split in two") },
  { 1, {levelData_c::DominoTypeExploder},   50,  0, _("Exploder: will blast a hole into the platform below it") },
  { 1, {levelData_c::DominoTypeDelay},      50,  0, _("Delay: falls not immediately but a while after being pushed") },
  { 1, {levelData_c::DominoTypeTumbler},    50,  0, _("Tumbler: will continue rolling until it hits an obstacle") },
  { 1, {levelData_c::DominoTypeBridger},    50,  0, _("Bridger: will connect the platform if there is a gap of one unit") },
  { 1, {levelData_c::DominoTypeVanish},     50,  0, _("Vanish: pushes next block but then vanishes, only stone you may place in front of doors") },
  { 1, {levelData_c::DominoTypeTrigger},    50,  0, _("Trigger: the last domino that must fall and it must lie flat, can not be moved") },
  { 1, {levelData_c::DominoTypeAscender},   50,  0, _("Ascender: will raise to ceiling when pushed and then flip up there") },
  { 2, {levelData_c::DominoTypeConnectedA,
        levelData_c::DominoTypeConnectedB}, 50, 18, _("Entangled: all stones of this type will fall together as if quantum entangled") },
  { 3, {levelData_c::DominoTypeCounter1,
        levelData_c::DominoTypeCounter2,
        levelData_c::DominoTypeCounter3}, 60, 15, _("Semiblocker: these behave like blocker as long as there is a stone still standing that has more lines") },
};

#define SX 310
#define SY 85
#define TX 100
#define TY 100

void helpWindow_c::displayCurrentPage(void)
{
  clearInside();

  fontParams_s par;

  uint32_t page = *(pages.rbegin());
  uint32_t ypos = (Y()+1)*gr.blockY();

  if (page == 0)
  {
    par.font = FNT_NORMAL;
    par.alignment = ALN_TEXT_CENTER;
    par.color.r = HLP_COL_R; par.color.g = HLP_COL_G; par.color.b = HLP_COL_B;
    par.shadow = 0;
    par.box.x = (800-16*40)/2;
    par.box.w = 16*40;
    par.box.y = ypos;
    par.box.h = (H()-2)/3*gr.blockY();

    if (getTextHeight(&par, _(help.c_str())) > par.box.h) {
      par.font = FNT_SMALL;
    }

    s.renderText(&par, _(help.c_str()));

    ypos += getTextHeight(&par, _(help.c_str())) + 20;
  }

  nextPage = 0;

  uint32_t column = 0;
  uint32_t linehight = SY;

  while (page < NUM_DOMINOS)
  {

    int displaywidth = dominoHelp[page].boxWidth;

    par.font = FNT_SMALL;
    par.alignment = ALN_TEXT;
    par.color.r = TXT_COL_R; par.color.g = TXT_COL_G; par.color.b = TXT_COL_B;
    par.shadow = 0;
    par.box.x = SX*column+TX+displaywidth+5;
    par.box.w = SX-displaywidth-15;
    par.box.y = ypos;
    par.box.h = 80;

    if (  (ypos + getTextHeight(&par, _(dominoHelp[page].text.c_str())) > (Y()+H()-1)*gr.blockY())
        ||(ypos + 75                                                    > (Y()+H()-1)*gr.blockY())
       )
    {
      nextPage = page;
      break;
    }

    s.renderText(&par, _(dominoHelp[page].text.c_str()));

    {
      uint32_t h = getTextHeight(&par, _(dominoHelp[page].text.c_str()));
      h += 10;
      if (h > linehight)
        linehight = h;
    }

    s.fillRect(SX*column+TX,   ypos,   displaywidth,   75, 0, 0, 0);
    s.fillRect(SX*column+TX+2, ypos+2, displaywidth-4, 75-4, TXT_COL_R, TXT_COL_G, TXT_COL_B);

    for (int i = 0; i < dominoHelp[page].numDominos; i++)
      s.blitBlock(g.getDomino(dominoHelp[page].dominos[i]-1, 7),
          SX*column+TX-105+displaywidth/2+int(dominoHelp[page].spacing*(1.0*i-(dominoHelp[page].numDominos-1)*0.5)), ypos + 4);

    page++;
    column++;
    if (column == 2)
    {
      ypos += linehight;
      column = 0;
      linehight = SY;
    }
  }

  par.font = FNT_SMALL;
  par.box.x = (800-14*40)/2;
  par.box.w = 0;
  par.box.y = (Y()+H()-1)*gr.blockY();
  par.box.h = 0;
  par.color.r = HLP_COL_R; par.color.g = HLP_COL_G; par.color.b = HLP_COL_B;

  if (*(pages.rbegin()) > 0)
  {
    s.renderText(&par, "<<");
  }

  par.box.x = (800+14*40)/2;

  if (nextPage > 0)
  {
    s.renderText(&par, ">>");
  }
}

helpWindow_c::helpWindow_c(const std::string & t, surface_c & su, graphics_c & gr) : window_c(1, 1, 18, 11, su, gr), help(t),
  s(su), g(gr)
{
  pages.push_back(0);
  displayCurrentPage();
}

bool helpWindow_c::handleEvent(const SDL_Event & event) {
  if (event.type == SDL_KEYDOWN)
  {
    if (   event.key.keysym.sym == SDLK_ESCAPE
        || event.key.keysym.sym == SDLK_RETURN
       )
    {
      done = true;
      return true;
    }
    if (event.key.keysym.sym == SDLK_LEFT)
    {
      if (pages.size() > 1)
      {
        pages.pop_back();
        displayCurrentPage();
        return true;
      }
    }
    if (event.key.keysym.sym == SDLK_RIGHT)
    {

      if (nextPage > 0)
      {
        pages.push_back(nextPage);
        displayCurrentPage();
        return true;
      }
    }
  }

  return false;
}

class aboutWindow_c : public window_c {

  public:

    aboutWindow_c(surface_c & s, graphics_c & g);
    bool handleEvent(const SDL_Event & event);
};

aboutWindow_c::aboutWindow_c(surface_c & s, graphics_c & g) : window_c(2, 0, 16, 12, s, g) {

  clearInside();

  fontParams_s par;

  par.font = FNT_BIG;
  par.alignment = ALN_CENTER;
  par.color.r = TXT_COL_R; par.color.g = TXT_COL_G; par.color.b = TXT_COL_B;
  par.shadow = 2;
  par.box.x = gr.blockX()*(X()+1);
  par.box.y = gr.blockY()*(Y()+1);
  par.box.w = gr.blockX()*(W()-2);
  par.box.h = getFontHeight(FNT_BIG);

  {
    char title[200];
    //TRANSLATORS: the %s is a placeholder for the version
    snprintf(title, 200, _("Pushover - %s - About"), PACKAGE_VERSION);
    surf.renderText(&par, title);
  }

  int ypos = gr.blockY()*(Y()+1) + getFontHeight(FNT_BIG);

  surf.fillRect(gr.blockX()*(X()+1)+1, ypos+1, gr.blockX()*(W()-2), 2, 0, 0, 0);
  surf.fillRect(gr.blockX()*(X()+1), ypos, gr.blockX()*(W()-2), 2, TXT_COL_R, TXT_COL_G, TXT_COL_B);
  ypos += 20;

  unsigned int lineH = getFontHeight(FNT_SMALL);  // height of one entry line

  par.font = FNT_SMALL;
  par.alignment = ALN_TEXT;
  par.box.x = gr.blockX()*(X()+1);
  par.box.y = ypos;
  par.box.w = gr.blockX()*(W()-2)-30;
  par.box.h = lineH;

  par.shadow = 2; par.box.y += surf.renderText(&par, _("Original Concept:"))*lineH; par.box.x += 30;
  par.shadow = 0; par.box.y += surf.renderText(&par, "Chas Partington")*lineH; par.box.x -= 30;

  par.shadow = 2; par.box.y += surf.renderText(&par, _("Original Programming:"))*lineH; par.box.x += 30;
  par.shadow = 0; par.box.y += surf.renderText(&par, "Dave Elcock, Helen Elcock, Keith Watterson")*lineH; par.box.x -= 30;

  par.shadow = 2; par.box.y += surf.renderText(&par, _("Original Graphics:"))*lineH; par.box.x += 30;
  par.shadow = 0; par.box.y += surf.renderText(&par, "Bryan King, Barry Armstrong")*lineH; par.box.x -= 30;

  par.shadow = 2; par.box.y += surf.renderText(&par, _("Original Music & SFX:"))*lineH; par.box.x += 30;
  par.shadow = 0; par.box.y += surf.renderText(&par, "Keith Tinman, Dean Evans, Johnathan Dunn")*lineH; par.box.x -= 30;

  par.shadow = 2; par.box.y += surf.renderText(&par, _("Original Levels:"))*lineH; par.box.x += 30;
  par.shadow = 0; par.box.y += surf.renderText(&par, "Harry Nadler, Avril Rigby, Don Rigby, Chris Waterworth")*lineH; par.box.x -= 30;

  par.shadow = 2; par.box.y += surf.renderText(&par, _("New Programming:"))*lineH; par.box.x += 30;
  par.shadow = 0; par.box.y += surf.renderText(&par, "Andreas Röver, Volker Grabsch")*lineH; par.box.x -= 30;

  par.shadow = 2; par.box.y += surf.renderText(&par, _("New Music:"))*lineH; par.box.x += 30;
  par.shadow = 0; par.box.y += surf.renderText(&par, "Roberto Lorenz")*lineH; par.box.x -= 30;

  par.shadow = 2; par.box.y += surf.renderText(&par, _("New Graphics:"))*lineH; par.box.x += 30;
  par.shadow = 0; par.box.y += surf.renderText(&par, "Harald Radke")*lineH; par.box.x -= 30;

  par.shadow = 2; par.box.y += surf.renderText(&par, _("New Levels:"))*lineH; par.box.x += 30;
  par.shadow = 0; par.box.y += surf.renderText(&par, " A&V: Volker Grabsch, Andreas Röver, Dominik Pöhlker")*lineH; par.box.x -= 30;
}


bool aboutWindow_c::handleEvent(const SDL_Event & event) {
  if (event.type == SDL_KEYDOWN &&
      event.key.keysym.sym == SDLK_ESCAPE)
  {
    done = true;
    return true;
  }

  return false;
}

void listWindow_c::redraw(void) {

  clearInside();

  fontParams_s par;

  par.font = FNT_BIG;
  par.alignment = ALN_CENTER;
  par.color.r = TXT_COL_R; par.color.g = TXT_COL_G; par.color.b = TXT_COL_B;
  par.shadow = 2;
  par.box.x = gr.blockX()*(x+1);
  par.box.y = gr.blockY()*(y+1);
  par.box.w = gr.blockX()*(w-2);
  par.box.h = getFontHeight(FNT_BIG);

  surf.renderText(&par, title);

  int ypos = gr.blockY()*(y+1) + getTextHeight(&par, title);

  surf.fillRect(gr.blockX()*(x+1)+1, ypos+1, gr.blockX()*(w-2), 2, 0, 0, 0);
  surf.fillRect(gr.blockX()*(x+1), ypos, gr.blockX()*(w-2), 2, TXT_COL_R, TXT_COL_G, TXT_COL_G);
  ypos += 20;

  unsigned int lineH = getFontHeight(FNT_NORMAL);  // height of one entry line

  menuLines = (gr.blockY()*(y+h-1)-ypos) / lineH;

  unsigned int line = 0;

  while (true)
  {
    int ypos2 = ypos;
    unsigned int line2 = line;

    bool back = false;

    while (ypos2 + lineH < gr.blockY()*(y+h-1)) {

      if (line2 >= entries.size())
      {
        if (line2 > (unsigned int)current)
          back = true;
        else
        {
          //list is empty
        }
        break;
      }

      if (line2 == (unsigned int)current && ypos2 < gr.blockY()*(y+h/2))
      {
        back = true;
        break;
      }

      if (entries[line2].line)
      {
        ypos2 += lineH;
      }

      line2++;
      ypos2 += lineH;
    }

    if (back)
    {
      if (line > 0)
        line--;

      break;
    }

    line++;
  }

  while (ypos + lineH < gr.blockY()*(y+h-1)) {

    if (line >= entries.size()) break;

    par.font = FNT_NORMAL;
    par.alignment = ALN_CENTER;

    if (line == current)
    {
      par.color.r = SEL_COL_R; par.color.g = SEL_COL_G; par.color.b = SEL_COL_B;
    }
    else if (entries[line].highlight)
    {
      par.color.r = HIL_COL_R; par.color.g = HIL_COL_G; par.color.b = HIL_COL_B;
    }
    else if (entries[line].sol == 2)
    {
      par.color.r = SOL_COL_R; par.color.g = SOL_COL_G; par.color.b = SOL_COL_B;
    }
    else if (entries[line].sol == 1)
    {
      par.color.r = SO1_COL_R; par.color.g = SO1_COL_G; par.color.b = SO1_COL_B;
    }
    else
    {
      par.color.r = TXT_COL_R; par.color.g = TXT_COL_G; par.color.b = TXT_COL_B;
    }

    par.shadow = 0;
    par.box.x = gr.blockX()*(x+1);
    par.box.y = ypos;
    par.box.w = gr.blockX()*(w-2);
    par.box.h = lineH;

    if (!entries[line].line)
    {
      surf.renderText(&par, entries[line].text);
    }
    else
    {
      surf.renderText(&par, entries[line].text);

      if (entries[line].line)
      {
        surf.fillRect(gr.blockX()*(x+1)+30, ypos+lineH+lineH/2, gr.blockX()*(w-2)-60, 2, SEP_COL_R, SEP_COL_G, SEP_COL_B);
        ypos += lineH;
      }

    }

    line++;
    ypos += lineH;
  }
}

listWindow_c::listWindow_c(int x, int y, int w, int h, surface_c & s, graphics_c & gr,
    const std::string & t, const std::vector<entry> & e, bool esc, int initial)
  : window_c(x, y, w, h, s, gr), entries(e), title(t), current(initial), escape(esc)
{
  redraw();

}

bool listWindow_c::handleEvent(const SDL_Event & event) {

  if (window_c::handleEvent(event)) return true;

  if (event.type == SDL_KEYDOWN)
  {
    if (escape && event.key.keysym.sym == SDLK_ESCAPE)
    {
      current = entries.size();  // make sure nothing valid is selected
      done = true;
      return true;
    }
    if (event.key.keysym.sym == SDLK_RETURN)
    {
      done = true;
      return true;
    }
    else if (event.key.keysym.sym == SDLK_UP)
    {
      if (current > 0) {
        current--;
        redraw();
      }
      return true;
    }
    else if (event.key.keysym.sym == SDLK_DOWN)
    {
      if (current+1 < entries.size()) {
        current++;
        redraw();
      }
      return true;
    }
    else if (event.key.keysym.sym == SDLK_PAGEUP)
    {
        if (current > 0)
        {
            if (current > menuLines) current -= menuLines; else current = 0;
            redraw();
        }
    }
    else if (event.key.keysym.sym == SDLK_PAGEDOWN)
    {
        if (current+1 < entries.size())
        {
            if (current+menuLines < entries.size()) current += menuLines; else current = entries.size()-1;
            redraw();
        }
    }
    else if (event.key.keysym.sym == SDLK_HOME)
    {
        if (current > 0)
        {
            current = 0;
            redraw();
        }
    }
    else if (event.key.keysym.sym == SDLK_END)
    {
        if (current+1 < entries.size())
        {
            current = entries.size()-1;
            redraw();
        }
    }
  }

  return false;
}


listWindow_c * getMainWindow(surface_c & surf, graphics_c & gr) {
    static std::vector<listWindow_c::entry> entries;

    if (!entries.size())
    {
        entries.push_back(listWindow_c::entry(_("Play Levelset")));
        entries.push_back(listWindow_c::entry(_("Configuration")));
        entries.push_back(listWindow_c::entry(_("Profile Selection")));
        entries.push_back(listWindow_c::entry(_("About")));
        entries.push_back(listWindow_c::entry(_("Quit")));
    }

    return new listWindow_c(4, 2, 12, 8, surf, gr, _("Main menu"), entries, false);
}

InputWindow_c * getProfileInputWindow(surface_c & surf, graphics_c & gr)
{
  return new InputWindow_c(4,2,12,5, surf, gr, _("Enter new profile name"));
}


listWindow_c * getProfileWindow(const solvedMap_c & solve, surface_c & surf, graphics_c & gr)
{
  std::vector<listWindow_c::entry> entries;

  for (size_t i = 0; i < solve.getNumberOfUsers(); i++)
  {
    entries.push_back(listWindow_c::entry(solve.getUserName(i)));
    if (i == solve.getCurrentUser())
      entries.rbegin()->highlight = true;
  }
  entries.rbegin()->line = true;
  entries.push_back(listWindow_c::entry(_("Add new profile")));
  entries.push_back(listWindow_c::entry(_("Delete a profile")));

  return new listWindow_c(4, 0, 12, 12, surf, gr, _("Select Your Profile"), entries, true, solve.getCurrentUser());
}

listWindow_c * getProfileSelector(const solvedMap_c & solve, surface_c & surf, graphics_c & gr)
{
  std::vector<listWindow_c::entry> entries;

  for (size_t i = 1; i < solve.getNumberOfUsers(); i++)
    entries.push_back(listWindow_c::entry(solve.getUserName(i)));

  return new listWindow_c(4, 0, 12, 12, surf, gr, _("Select Profile to delete"), entries, true, 0);
}

listWindow_c * getConfigWindow(surface_c & surf, graphics_c & gr) {
    static std::vector<listWindow_c::entry> entries;

    if (!entries.size())
    {
        entries.push_back(listWindow_c::entry(_("Toggle Fullscreen")));
        entries.push_back(listWindow_c::entry(_("Toggle Sound Effects")));
        entries.push_back(listWindow_c::entry(_("Toggle Background Music")));
    }

    return new listWindow_c(3, 2, 14, 9, surf, gr, _("Configuration"), entries, true);
}

listWindow_c * getMissionWindow(const levelsetList_c & ls, surface_c & surf, graphics_c & gr, const std::string & selection) {
    std::vector<listWindow_c::entry> entries;

    int index = 0;

    for (unsigned int i = 0; i < ls.getLevelsetNames().size(); i++)
    {
      if (selection == ls.getLevelsetNames()[i])
        index = i;
      entries.push_back(listWindow_c::entry(_(ls.getLevelsetNames()[i].c_str())));
    }

    return new listWindow_c(4, 2, 12, 9, surf, gr, _("Select Levelset"), entries, true, index);
}

class levelWindow_c : public listWindow_c
{

  public:

    levelWindow_c(int x, int y, int w, int h, surface_c & s, graphics_c & gr,
        const std::string & t, const std::vector<entry> & e, bool esc, int initial) :
      listWindow_c(x, y, w, h, s, gr, t, e, esc, initial)
    {
      redraw();
    }

    void redraw(void)
    {
      listWindow_c::redraw();

      fontParams_s par;

      par.font = FNT_SMALL;
      par.alignment = ALN_TEXT;
      par.box.y = (Y()+H()-1)*gr.blockY();
      par.box.w = gr.blockX()*(W()-2)-30;
      par.box.h = 10;
      par.shadow = false;

      std::string text[3] = {
        //TRANSLATORS: keep very short as these 3 must fit in one line in the level selector window
        _("unsolved"),
        //TRANSLATORS: keep very short as these 3 must fit in one line in the level selector window
        _("solved but not in time"),
        //TRANSLATORS: keep very short as these 3 must fit in one line in the level selector window
        _("solved")
      };

      par.box.x = gr.blockX()*(X()+1 + X()+W()-1)/2 -
          (getTextWidth(FNT_SMALL, text[0]) + getTextWidth(FNT_SMALL, text[1]) + getTextWidth(FNT_SMALL, text[2]) + 30)/2;

      par.color.r = TXT_COL_R; par.color.g = TXT_COL_G; par.color.b = TXT_COL_B;
      surf.renderText(&par, text[0]);
      par.box.x += getTextWidth(FNT_SMALL, text[0])+15;

      par.color.r = SO1_COL_R; par.color.g = SO1_COL_G; par.color.b = SO1_COL_B;
      surf.renderText(&par, text[1]);
      par.box.x += getTextWidth(FNT_SMALL, text[1])+15;

      par.color.r = SOL_COL_R; par.color.g = SOL_COL_G; par.color.b = SOL_COL_B;
      surf.renderText(&par, text[2]);
    }
};

listWindow_c * getLevelWindow(const levelset_c & ls, const solvedMap_c & solv, surface_c & surf, graphics_c & gr, const std::string & lname) {
    std::vector<listWindow_c::entry> entries;

    int index = -1;

    for (unsigned int i = 0; i < ls.getLevelNames().size(); i++)
    {
        std::string e = ls.getLevelNames()[i];

        if (e == lname)
          index = i;

        if (solv.solved(ls.getChecksum(e)))
        {
            std::string f = std::string(_(e.c_str())) + "  \xE2\x96\xA0";
            entries.push_back(listWindow_c::entry(_(f.c_str())));
            entries.rbegin()->sol = 2;
        }
        else if (solv.solved(ls.getChecksumNoTime(e)))
        {
            std::string f = std::string(_(e.c_str())) + "  \xE2\x96\xA3";
            entries.push_back(listWindow_c::entry(_(f.c_str())));
            entries.rbegin()->sol = 1;
        }
        else
        {
            std::string f = std::string(_(e.c_str())) + "  \xE2\x96\xA1";
            entries.push_back(listWindow_c::entry(_(f.c_str())));
            entries.rbegin()->sol = 0;

            if (index == -1)
              index = i;
        }
    }

    if (entries.size() == 0) throw std::runtime_error(_("No Level in Levelset"));

    // when all levels have been solved, return to the first
    if (index == -1) index = 0;

    return new levelWindow_c(4, 0, 12, 12, surf, gr, _("Select Level"), entries, true, index);
}

listWindow_c * getQuitWindow(surface_c & surf, graphics_c & gr) {
    static std::vector<listWindow_c::entry> entries;

    if (!entries.size())
    {
        entries.push_back(listWindow_c::entry(_("Return to level")));
        entries.push_back(listWindow_c::entry(_("Restart level")));
        entries.push_back(listWindow_c::entry(_("Configuration")));
        entries.push_back(listWindow_c::entry(_("Return to menu")));
    }

    return new listWindow_c(4, 3, 12, 7, surf, gr, _("And now?"), entries, true);
}

window_c * getAboutWindow(surface_c & surf, graphics_c & gr) {
    return new aboutWindow_c(surf, gr);
}

listWindow_c * getSolvedWindow(surface_c & surf, graphics_c & gr) {
    static std::vector<listWindow_c::entry> entries;

    if (!entries.size())
    {
        entries.push_back(listWindow_c::entry(_("Continue")));
    }

    return new listWindow_c(2, 3, 16, 6, surf, gr, _("Congratulations! You did it."), entries, false);
}

listWindow_c * getFailedWindow(int failReason, surface_c & surf, graphics_c & gr) {
    static std::vector<listWindow_c::entry> entries;

    if (!entries.size())
    {
        entries.push_back(listWindow_c::entry(_("Retry the level")));
        entries.push_back(listWindow_c::entry(_("Return to menu")));
    }

    std::string title;
    switch (failReason) {
      case 3: title = _("You failed: Some dominoes crashed"); break;
      case 4: title = _("You failed: Not all dominoes fell"); break;
      case 5: title = _("You failed: You died"); break;
      case 6: title = _("You failed: Trigger was not last to fall"); break;
      case 7: title = _("You failed: Trigger not flat on the ground"); break;
      default: title = _("You failed... but I don't know why ...?"); break;
    }

    unsigned int w = (getTextWidth(FNT_BIG, title) + gr.blockX() - 1) / gr.blockX();

    // make sure minimum size is fulfilled
    if (w < 14) w = 14;
    // try to accommodate the header in the width
    if (w > 18) w = 18;
    // make sure widow is centerable by having an even width
    if (w & 1) w++;

    // add the border
    w += 2;

    return new listWindow_c((20-w)/2, 3, w, 6, surf, gr, title, entries, false);
}


listWindow_c * getTimeoutWindow(surface_c & surf, graphics_c & gr) {
    static std::vector<listWindow_c::entry> entries;

    if (!entries.size())
    {
        entries.push_back(listWindow_c::entry(_("Retry the level")));
        entries.push_back(listWindow_c::entry(_("Select next level")));
    }

    std::string title;
    title = _("Not quite, you were not fast enough, but you may continue if you want");

    unsigned int w = (getTextWidth(FNT_BIG, title) + gr.blockX() - 1) / gr.blockX();

    // make sure minimum size is fulfilled
    if (w < 14) w = 14;
    // try to accommodate the header in the width
    if (w > 18) w = 18;
    // make sure widow is centerable by having an even width
    if (w & 1) w++;

    // add the border
    w += 2;

    return new listWindow_c((20-w)/2, 3, w, 6, surf, gr, title, entries, false);
}


InputWindow_c::InputWindow_c(int x, int y, int w, int h, surface_c & s, graphics_c & gr,
        const std::string & ti) : window_c(x, y, w, h, s, gr)
{
  title = ti;
  input = "";
  cursorPosition = 0;
  escape = false;
  redraw();
}


static size_t utf8EncodeOne(uint16_t uni, char *utf8, size_t bufspace)
{
  size_t bytes, si;

  if (uni <= 0x7F) {
    /* one byte, simple */
    if (bufspace < 1) return 0;
    utf8[0] = (char) uni;
    return 1;

  } else if (uni <= 0x7FF) { /* 0b110ddddd 10dddddd */
    /* two bytes */
    if (bufspace < 2) return 0;
    bytes = 2;
    utf8[0] = 0xC0 | (uni >> 6);

  } else  { /* 0b1110dddd 10dddddd 10dddddd */
    /* three bytes */
    if (bufspace < 3) return 0;
    bytes = 3;
    utf8[0] = 0xE0 | ((uni >> 12) & 0x0F);

  }

  for (si = bytes - 1; si > 0; si--) {
    utf8[si] = 0x80 | (uni & 0x3F);
    uni >>= 6;
  }

  return bytes;
}

bool InputWindow_c::handleEvent(const SDL_Event & event)
{
  if (window_c::handleEvent(event)) return true;

  if (event.type == SDL_KEYDOWN)
  {
    if (event.key.keysym.sym == SDLK_ESCAPE)
    {
      escape = true;
      done = true;
      return true;
    }
    else if (event.key.keysym.sym == SDLK_RETURN)
    {
      done = true;
      return true;
    }
    else if (event.key.keysym.sym == SDLK_LEFT)
    {
      if (cursorPosition > 0) {
        cursorPosition--;
        while (   (cursorPosition > 0)
               && ((input[cursorPosition] & 0xC0) == 0x80)
              )
        {
          cursorPosition--;
        }
        redraw();
      }
      return true;
    }
    else if (event.key.keysym.sym == SDLK_RIGHT)
    {
      if (cursorPosition+1 <= input.length()) {
        cursorPosition++;
        while (   (cursorPosition < input.length())
               && ((input[cursorPosition] & 0xC0) == 0x80)
              )
        {
          cursorPosition++;
        }
        redraw();
      }
      return true;
    }
    else if (event.key.keysym.sym == SDLK_HOME)
    {
        if (cursorPosition > 0)
        {
          cursorPosition = 0;
          redraw();
        }
    }
    else if (event.key.keysym.sym == SDLK_END)
    {
        if (cursorPosition+1 < input.length())
        {
          cursorPosition = input.length();
          redraw();
        }
    }
    else if (event.key.keysym.sym == SDLK_BACKSPACE)
    {
      while (   (cursorPosition > 0)
             && ((input[cursorPosition-1] & 0xC0) == 0x80)
            )
      {
        input.erase(cursorPosition-1, 1);
        cursorPosition--;
      }
      if (cursorPosition > 0)
      {
        input.erase(cursorPosition-1, 1);
        cursorPosition--;
      }
      redraw();
    }
    else if (event.key.keysym.sym == SDLK_DELETE)
    {
      if (cursorPosition < input.length())
      {
        input.erase(cursorPosition, 1);

        while (   (cursorPosition < input.length())
               && ((input[cursorPosition] & 0xC0) == 0x80)
              )
        {
          input.erase(cursorPosition, 1);
        }
        redraw();
      }
    }
    else if (event.key.keysym.unicode >= 32)
    {
      if (getTextWidth(FNT_NORMAL, input+(char)event.key.keysym.unicode) < gr.blockX()*(w-2))
      {
        char utf8[10];
        size_t s = utf8EncodeOne(event.key.keysym.unicode, utf8, 10);

        for (int i = 0; i < s; i++)
        {
          input.insert(cursorPosition, 1, utf8[i]);
          cursorPosition++;
        }
        redraw();
      }
    }
    else
    {
    }
  }

  return false;
}

void InputWindow_c::redraw(void)
{
  clearInside();

  fontParams_s par;

  par.font = FNT_BIG;
  par.alignment = ALN_CENTER;
  par.color.r = TXT_COL_R; par.color.g = TXT_COL_G; par.color.b = TXT_COL_B;
  par.shadow = 2;
  par.box.x = gr.blockX()*(x+1);
  par.box.y = gr.blockY()*(y+1);
  par.box.w = gr.blockX()*(w-2);
  par.box.h = getFontHeight(FNT_BIG);

  surf.renderText(&par, title);

  int ypos = gr.blockY()*(y+1) + getFontHeight(FNT_BIG);

  surf.fillRect(gr.blockX()*(x+1)+1, ypos+1, gr.blockX()*(w-2), 2, 0, 0, 0);
  surf.fillRect(gr.blockX()*(x+1), ypos, gr.blockX()*(w-2), 2, TXT_COL_R, TXT_COL_G, TXT_COL_B);

  ypos += 20;

  par.alignment = ALN_TEXT;
  par.font = FNT_NORMAL;
  par.shadow = 0;

  unsigned int wi = getTextWidth(FNT_NORMAL, input.substr(0, cursorPosition));

  surf.fillRect(gr.blockX()*(x+1)+wi, ypos, 4, getFontHeight(FNT_NORMAL), 0, 0, 0);

  par.box.y = ypos;
  surf.renderText(&par, input);
}
