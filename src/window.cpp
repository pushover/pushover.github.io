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
#include "graphicsn.h"
#include "levelset.h"
#include "textsections.h"
#include "solvedmap.h"
#include "leveldata.h"
#include "colors.h"

#include <SDL.h>

#include <stdexcept>
#include <algorithm>
#include <libintl.h>

#include <assert.h>

// a window that displays a list with selectable entries
class listWindow_c : public window_c {

  public:
    typedef struct entry {
        std::string text;
        std::vector<std::string> details;
        bool highlight;
        int sol;  // 0 no normal color, 1 bit, yellow, 2 complete green
        bool line;

        entry(std::string t) : text(t), highlight(false), sol(0), line(false) {}


        uint16_t height_details; // calculated value to store the hight of this entry.. used by light window
        uint16_t height; // calculated value to store the hight of this entry.. used by light window
    } entry;

  private:
    std::vector<entry> entries;
    std::string title;

    unsigned int current;
    bool escape;  // escape works

  public:

    listWindow_c(int x, int y, int w, int h, surface_c & s, graphicsN_c & gr,
        const std::string & title, const std::vector<entry> & entries, bool escape, int initial = 0);

    // the the user has selected something
    unsigned int getSelection(void) { return current; } // which list entry was selected

    virtual bool handleEvent(const SDL_Event & event);

    virtual void redraw(void);
};

class InputWindow_c : public window_c {

  private:
    std::string input;

    unsigned int cursorPosition;

    std::string title;

    void redraw(void);

  public:

    InputWindow_c(int x, int y, int w, int h, surface_c & s, graphicsN_c & gr,
        const std::string & title);

    // the the user has selected something
    const std::string getText(void) { return input; } // which list entry was selected

    virtual bool handleEvent(const SDL_Event & event);
};


window_c::window_c(unsigned char x_, unsigned char y_, unsigned char w_, unsigned char h_, surface_c & s, graphicsN_c & g) : x(x_), y(y_), w(w_), h(h_), surf(s), gr(g) {

  if (w < 2 || h < 2) return;

  clearInside();

  done = false;

  gr.markAllDirty();
  gr.drawLevel();
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

      surf.blitBlock(*gr.getBoxBlock(yp*3+xp), (x+i)*gr.blockX(), (y+j)*gr.blockY());
    }
}

window_c::~window_c(void) {
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

  unsigned int bestLine = 0xFFFF;
  unsigned int bestCenter = 10000; // distance to center for the current best starting line

  unsigned int center = (ypos + gr.blockY()*(y+h-2))/2;
  int end = gr.blockY()*(y+h-1);

  unsigned int line = current;

  // try to find the best starting line so that
  // 1: the current entry is completely on screen
  // 2: is is roughly in the middle of the screen
  //
  // for this check all view possibilitied that
  // do contain the current entry by starting with the current
  // entry and then starting one before that .. until the current
  // entry is out at the bottom
  while (true)
  {
    unsigned int line2 = line;
    int ypos2 = ypos;
    bool found = false;

    unsigned int c = 0;

    while (line2 < entries.size())
    {
      if (line2 == current)
      {
        ypos2 += entries[line2].height_details;

        if (ypos2 >= end)
        {
          line2--;
          break;
        }

        c = ypos2-entries[line2].height_details/2;
        found = true;
      }
      else
      {
        ypos2 += entries[line2].height;
        if (ypos2 >= end)
        {
          line2--;
          break;
        }
      }
      line2++;
    }

    unsigned int diff = abs(c-center);

    if (line2 < entries.size())
    {
      if (found && (diff < bestCenter))
      {
        bestCenter = diff;
        bestLine = line;
      }
    }
    else
    {
      bestCenter = abs(c-center);
      bestLine = line;
    }

    if (!found) break;
    if (line == 0) break;

    // next line above
    line--;
  }

  line = bestLine;

  while (true) {

    if (line >= entries.size()) break;

    if (line == current)
    {
      if (ypos + entries[line].height_details >= end) break;
    }
    else
    {
      if (ypos + entries[line].height >= end) break;
    }

    par.font = FNT_NORMAL;
    par.alignment = ALN_TEXT_CENTER;

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
    par.box.h = entries[line].height;

    surf.renderText(&par, entries[line].text);
    ypos += getTextHeight(&par, entries[line].text);
    par.box.y = ypos;

    if (line == current && entries[line].details.size() != 0)
    {
      par.font = FNT_SMALL;

      for (size_t i = 0; i < entries[line].details.size(); i++)
      {
        surf.renderText(&par, entries[line].details[i]);
        ypos += getTextHeight(&par, entries[line].details[i]);
        par.box.y = ypos;
      }
    }

    if (entries[line].line)
    {
      surf.fillRect(gr.blockX()*(x+1)+30, ypos+10/2, gr.blockX()*(w-2)-60, 2, SEP_COL_R, SEP_COL_G, SEP_COL_B);
      ypos += 10;
      par.box.y = ypos;
    }

    line++;
  }
}

listWindow_c::listWindow_c(int x, int y, int w, int h, surface_c & s, graphicsN_c & gr,
    const std::string & t, const std::vector<entry> & e, bool esc, int initial)
  : window_c(x, y, w, h, s, gr), entries(e), title(t), current(initial), escape(esc)
{
  // calculate the hight of all entries
  fontParams_s par;
  par.alignment = ALN_CENTER;

  for (size_t line = 0; line < entries.size(); line++)
  {
    par.font = FNT_NORMAL;
    par.alignment = ALN_CENTER;

    par.box.x = gr.blockX()*(x+1);
    par.box.w = gr.blockX()*(w-2);

    entries[line].height = getTextHeight(&par, entries[line].text);
    entries[line].height_details = entries[line].height;

    par.font = FNT_SMALL;
    par.alignment = ALN_TEXT_CENTER;

    for (size_t i = 0; i < entries[line].details.size(); i++)
    {
      entries[line].height_details += getTextHeight(&par, entries[line].details[i]);
    }

    par.alignment = ALN_CENTER;

    if (entries[line].line)
    {
      entries[line].height += 10;
      entries[line].height_details += 10;
    }
  }

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
            if (current > 10) current -= 10; else current = 0;
            redraw();
        }
    }
    else if (event.key.keysym.sym == SDLK_PAGEDOWN)
    {
        if (current+1 < entries.size())
        {
            if (current+10 < entries.size()) current += 10; else current = entries.size()-1;
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


window_c * getMainWindow(surface_c & surf, graphicsN_c & gr) {
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

window_c * getProfileInputWindow(surface_c & surf, graphicsN_c & gr)
{
  return new InputWindow_c(4,2,12,5, surf, gr, _("Enter new profile name"));
}


window_c * getProfileWindow(const solvedMap_c & solve, surface_c & surf, graphicsN_c & gr)
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

window_c * getProfileSelector(const solvedMap_c & solve, surface_c & surf, graphicsN_c & gr)
{
  std::vector<listWindow_c::entry> entries;

  for (size_t i = 1; i < solve.getNumberOfUsers(); i++)
    entries.push_back(listWindow_c::entry(solve.getUserName(i)));

  return new listWindow_c(4, 0, 12, 12, surf, gr, _("Select Profile to delete"), entries, true, 0);
}

window_c * getConfigWindow(surface_c & surf, graphicsN_c & gr, const configSettings & c, int sel)
{
  std::vector<listWindow_c::entry> entries;

  if (c.useFullscreen)
    entries.push_back(listWindow_c::entry(std::string(_("Use Fullscreen"))+" \xE2\x96\xA0"));
  else
    entries.push_back(listWindow_c::entry(std::string(_("Use Fullscreen"))+" \xE2\x96\xA1"));

  if (c.playSounds)
    entries.push_back(listWindow_c::entry(std::string(_("Play Sound Effects"))+" \xE2\x96\xA0"));
  else
    entries.push_back(listWindow_c::entry(std::string(_("Play Sound Effects"))+" \xE2\x96\xA1"));

  if (c.playMusic)
    entries.push_back(listWindow_c::entry(std::string(_("Play Background Music"))+" \xE2\x96\xA0"));
  else
    entries.push_back(listWindow_c::entry(std::string(_("Play Background Music"))+" \xE2\x96\xA1"));

  return new listWindow_c(3, 2, 14, 9, surf, gr, _("Configuration"), entries, true, sel);
}

static int MissionSolveState(const levelset_c & ls, const solvedMap_c & solv)
{
    bool allSolved = true;
    bool noneSolved = true;

    for (unsigned int i = 0; i < ls.getLevelNames().size(); i++)
    {
        std::string e = ls.getLevelNames()[i];

        if (solv.solved(ls.getChecksum(e)))
        {
            noneSolved = false;
        }
        else if (solv.solved(ls.getChecksumNoTime(e)))
        {
            noneSolved = false;
        }
        else
        {
            allSolved = false;
        }
    }

    if (noneSolved) return 0;
    if (allSolved) return 2;
    return 1;
}

class missionWindow_c : public listWindow_c
{

  public:

    missionWindow_c(int x, int y, int w, int h, surface_c & s, graphicsN_c & gr,
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
        _("not started"),
        //TRANSLATORS: keep very short as these 3 must fit in one line in the level selector window
        _("partially done"),
        //TRANSLATORS: keep very short as these 3 must fit in one line in the level selector window
        _("done")
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

class authorData {

  public:
    std::string name;
    uint16_t cnt;

    bool operator< (const authorData & b) const
    {
      return cnt > b.cnt;
    }

};

static std::string collectAuthors(const levelset_c & ls)
{
  std::vector<authorData> authors;

  for (size_t i = 0; i < ls.getLevelNames().size(); i++)
  {
    std::string e = ls.getLevelNames()[i];
    levelData_c level;
    ls.loadLevel(level, e, "");

    for (size_t j = 0; j < level.getAuthor().size(); j++)
    {
      bool found = false;
      for (size_t k = 0; k < authors.size(); k++)
        if (authors[k].name == level.getAuthor()[j])
        {
          authors[k].cnt++;
          found = true;
          break;
        }

      if (!found)
      {
        authors.resize(authors.size()+1);
        authors.back().name = level.getAuthor()[j];
        authors.back().cnt = 1;
      }
    }
  }

  // sort authors by number of levels they designed
  std::sort(authors.begin(), authors.end());

  std::string res;

  for (size_t i = 0; i < authors.size(); i++)
  {
    if (res != "") res += ", ";
    res += authors[i].name;
  }

  return res;
}

window_c * getMissionWindow(const levelsetList_c & ls, const solvedMap_c & solv, surface_c & surf, graphicsN_c & gr, const std::string & selection) {
    std::vector<listWindow_c::entry> entries;

    int index = -1;
    std::string f;

    for (unsigned int i = 0; i < ls.getLevelsetNames().size(); i++)
    {
      const std::string name = ls.getLevelsetNames()[i];

      if (selection == name)
        index = i;

      switch (MissionSolveState(ls.getLevelset(ls.getLevelsetNames()[i]), solv))
      {
        case 0:
          f = std::string(_(name)) + "  \xE2\x96\xA1";
          entries.push_back(listWindow_c::entry(f));
          entries.rbegin()->sol = 0;

          if (index == -1)
            index = i;

          break;
        case 1:
          f = std::string(_(name)) + "  \xE2\x96\xA3";
          entries.push_back(listWindow_c::entry(f));
          entries.rbegin()->sol = 1;

          if (index == -1)
            index = i;

          break;
        case 2:
          f = std::string(_(name)) + "  \xE2\x96\xA0";
          entries.push_back(listWindow_c::entry(f));
          entries.rbegin()->sol = 2;
          break;
        default:
          assert(0);
      }
      entries.rbegin()->details.push_back(_("Description: ") + _(ls.getLevelset(name).getDescription()));
      entries.rbegin()->details.push_back(_(" Authors: ") + collectAuthors(ls.getLevelset(name)));
    }

    // when all missions have been solved, return to the first
    if (index == -1) index = 0;

    return new missionWindow_c(3, 1, 14, 11, surf, gr, _("Select Levelset"), entries, true, index);
}

class levelWindow_c : public listWindow_c
{

  public:

    levelWindow_c(int x, int y, int w, int h, surface_c & s, graphicsN_c & gr,
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

window_c * getLevelWindow(const levelset_c & ls, const solvedMap_c & solv, surface_c & surf, graphicsN_c & gr, const std::string & lname) {
    std::vector<listWindow_c::entry> entries;

    int index = -1;

    for (unsigned int i = 0; i < ls.getLevelNames().size(); i++)
    {
        std::string e = ls.getLevelNames()[i];

        if (e == lname)
          index = i;

        if (solv.solved(ls.getChecksum(e)))
        {
            std::string f = std::string(_(e)) + "  \xE2\x96\xA0";
            entries.push_back(listWindow_c::entry(f));
            entries.rbegin()->sol = 2;
        }
        else if (solv.solved(ls.getChecksumNoTime(e)))
        {
            std::string f = std::string(_(e)) + "  \xE2\x96\xA3";
            entries.push_back(listWindow_c::entry(f));
            entries.rbegin()->sol = 1;
        }
        else
        {
            std::string f = std::string(_(e)) + "  \xE2\x96\xA1";
            entries.push_back(listWindow_c::entry(f));
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

window_c * getQuitWindow(bool complete, surface_c & surf, graphicsN_c & gr) {
    std::vector<listWindow_c::entry> entries;

    if (!entries.size())
    {
        entries.push_back(listWindow_c::entry(_("Return to level")));
        if (complete) entries.push_back(listWindow_c::entry(_("Restart level")));
        entries.push_back(listWindow_c::entry(_("Configuration")));
        entries.push_back(listWindow_c::entry(_("Return to menu")));
    }

    return new listWindow_c(4, 3, 12, 7, surf, gr, _("And now?"), entries, true);
}

window_c * getSolvedWindow(surface_c & surf, graphicsN_c & gr) {
    static std::vector<listWindow_c::entry> entries;

    if (!entries.size())
    {
        entries.push_back(listWindow_c::entry(_("Continue")));
    }

    return new listWindow_c(2, 3, 16, 6, surf, gr, _("Congratulations! You did it."), entries, false);
}

window_c * getFailedWindow(LevelState failReason, surface_c & surf, graphicsN_c & gr) {
    static std::vector<listWindow_c::entry> entries;

    if (!entries.size())
    {
        entries.push_back(listWindow_c::entry(_("Retry the level")));
        entries.push_back(listWindow_c::entry(_("Return to menu")));
    }

    std::string title;
    switch (failReason) {
      case LS_crashes        : title = _("You failed: Some dominoes crashed"); break;
      case LS_someLeft       : title = _("You failed: Not all dominoes fell"); break;
      case LS_died           : title = _("You failed: You died"); break;
      case LS_triggerNotLast : title = _("You failed: Trigger was not last to fall"); break;
      case LS_triggerNotFlat : title = _("You failed: Trigger not flat on the ground"); break;
      default                : title = _("You failed... but I don't know why ...?"); break;
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


window_c * getTimeoutWindow(surface_c & surf, graphicsN_c & gr) {
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


InputWindow_c::InputWindow_c(int x, int y, int w, int h, surface_c & s, graphicsN_c & gr,
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

        for (size_t i = 0; i < s; i++)
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
