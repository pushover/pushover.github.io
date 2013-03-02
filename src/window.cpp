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
    std::string subtitle;

    unsigned int current;
    bool escape;  // escape works

  public:

    listWindow_c(int x, int y, int w, int h, surface_c & s, graphicsN_c & gr,
        const std::string & title, const std::string & subtitle, const std::vector<entry> & entries, bool escape, int initial = 0);

    // the the user has selected something
    unsigned int getSelection(void) { return current; } // which list entry was selected

    virtual bool handleEvent(const SDL_Event & event);

    virtual void redraw(void);
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
  gr.markAllDirty();
}

window_c::~window_c(void) {
  gr.markAllDirty();
}


void listWindow_c::redraw(void) {

  clearInside();

  fontParams_s par;

  par.font = FNT_BIG;
  par.alignment = ALN_TEXT_CENTER;
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

  if (subtitle != "")
  {
    par.font = FNT_SMALL;
    par.shadow = false;
    par.box.y = ypos;
    surf.renderText(&par, subtitle);
    ypos += getTextHeight(&par, subtitle);
  }


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
    const std::string & t, const std::string & subt, const std::vector<entry> & e, bool esc, int initial)
  : window_c(x, y, w, h, s, gr), entries(e), title(t), subtitle(subt), current(initial), escape(esc)
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
        entries.push_back(listWindow_c::entry(_("Level Editor")));
        entries.push_back(listWindow_c::entry(_("About")));
        entries.push_back(listWindow_c::entry(_("Quit")));
    }

    return new listWindow_c(4, 2, 12, 8, surf, gr, _("Main menu"), "", entries, false);
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

  return new listWindow_c(4, 0, 12, 12, surf, gr, _("Select Your Profile"), "", entries, true, solve.getCurrentUser());
}

window_c * getProfileSelector(const solvedMap_c & solve, surface_c & surf, graphicsN_c & gr)
{
  std::vector<listWindow_c::entry> entries;

  for (size_t i = 1; i < solve.getNumberOfUsers(); i++)
    entries.push_back(listWindow_c::entry(solve.getUserName(i)));

  return new listWindow_c(4, 0, 12, 12, surf, gr, _("Select Profile to delete"), "",  entries, true, 0);
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

  return new listWindow_c(3, 2, 14, 9, surf, gr, _("Configuration"), "", entries, true, sel);
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
      listWindow_c(x, y, w, h, s, gr, t, "", e, esc, initial)
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
      listWindow_c(x, y, w, h, s, gr, t, "", e, esc, initial)
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

    return new listWindow_c(4, 3, 12, 7, surf, gr, _("And now?"), "", entries, true);
}

window_c * getSolvedWindow(surface_c & surf, graphicsN_c & gr) {
    static std::vector<listWindow_c::entry> entries;

    if (!entries.size())
    {
        entries.push_back(listWindow_c::entry(_("Continue")));
    }

    return new listWindow_c(2, 3, 16, 6, surf, gr, _("Congratulations! You did it."), "", entries, false);
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

    return new listWindow_c((20-w)/2, 3, w, 6, surf, gr, title, "", entries, false);
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

    return new listWindow_c((20-w)/2, 3, w, 6, surf, gr, title, "", entries, false);
}

window_c * getEditorLevelChooserWindow(surface_c & surf, graphicsN_c & gr, const levelset_c & l)
{
  std::vector<listWindow_c::entry> entries;

  const std::vector<std::string> & names = l.getLevelNames();

  for (size_t i = 0; i < names.size(); i++)
    entries.push_back(listWindow_c::entry(names[i]));

  if (entries.size() > 0)
    entries.back().line = true;

  entries.push_back(listWindow_c::entry(_("Add a new level")));

  if (entries.size() > 1)
    entries.push_back(listWindow_c::entry(_("Delete a level")));

  return new listWindow_c(2, 0, 16, 12, surf, gr, _("Choose level to edit"), "", entries, true);
}

window_c * getDeleteLevelWindow(surface_c & surf, graphicsN_c & gr, const levelset_c & l)
{
  std::vector<listWindow_c::entry> entries;

  const std::vector<std::string> names = l.getLevelNames();

  for (size_t i = 0; i < names.size(); i++)
    entries.push_back(listWindow_c::entry(names[i]));

  return new listWindow_c(2, 0, 16, 12, surf, gr, _("Choose level to delete"), "", entries, true);
}

window_c * getMessageWindow(surface_c & surf, graphicsN_c & gr, const std::string & title)
{
  std::vector<listWindow_c::entry> entries;

  if (!entries.size())
  {
    entries.push_back(listWindow_c::entry(_("Continue")));
  }

  return new listWindow_c(4, 3, 12, 7, surf, gr, title, "", entries, true);
}

window_c * getEditorMenu(surface_c & surf, graphicsN_c & gr)
{
  std::vector<listWindow_c::entry> entries;

  if (!entries.size())
  {
    entries.push_back(listWindow_c::entry(_("Select theme")));
    entries.push_back(listWindow_c::entry(_("Change name")));
    entries.push_back(listWindow_c::entry(_("Change time")));
    entries.push_back(listWindow_c::entry(_("Edit hint")));
    entries.push_back(listWindow_c::entry(_("Edit authors")));

    entries.back().line = true;

    entries.push_back(listWindow_c::entry(_("Test level")));

    entries.back().line = true;

    entries.push_back(listWindow_c::entry(_("Save")));
    entries.push_back(listWindow_c::entry(_("Edit another file")));
    entries.push_back(listWindow_c::entry(_("Leave editor")));

  }
  return new listWindow_c(4, 1, 12, 11, surf, gr, _("Editor main menu"), "", entries, true);
}

class editorHelpWindow_c : public window_c {

  private:
    surface_c & s;
    graphicsN_c & g;

  private:

    void displayCurrentPage(void)
    {
      clearInside();

      fontParams_s par;

      uint32_t ypos = (Y()+1)*gr.blockY();

      par.font = FNT_NORMAL;
      par.alignment = ALN_TEXT_CENTER;
      par.color.r = HLP_COL_R; par.color.g = HLP_COL_G; par.color.b = HLP_COL_B;
      par.shadow = 0;
      par.box.x = (800-16*40)/2;
      par.box.w = 16*40;
      par.box.y = ypos;
      par.box.h = (H()-2)/3*gr.blockY();

      s.renderText(&par, _("Editor Help window"));
    }

  public:

    editorHelpWindow_c(surface_c & su, graphicsN_c & gr) : window_c(1, 1, 18, 11, su, gr), s(su), g(gr)
    {
      displayCurrentPage();
    }

    bool handleEvent(const SDL_Event & event)
    {
      if (event.type == SDL_KEYDOWN)
      {
        if (   event.key.keysym.sym == SDLK_ESCAPE
            || event.key.keysym.sym == SDLK_RETURN
           )
        {
          done = true;
          return true;
        }
      }

      return false;
    }

};

window_c * getEditorHelp(surface_c & surf, graphicsN_c & gr)
{
  return new editorHelpWindow_c(surf, gr);
}

window_c * getThemeSelectorWindow(surface_c & surf, graphicsN_c &gr)
{
  static std::vector<listWindow_c::entry> entries;

  if (!entries.size())
  {
    entries.push_back(listWindow_c::entry(_("Toxic City")));
    entries.push_back(listWindow_c::entry(_("Atzec Temple")));
    entries.push_back(listWindow_c::entry(_("Space Station")));
    entries.push_back(listWindow_c::entry(_("Electric Circuits")));
    entries.push_back(listWindow_c::entry(_("Greek Temple")));
    entries.push_back(listWindow_c::entry(_("Medieval Castle")));
    entries.push_back(listWindow_c::entry(_("Mechanic Toys")));
    entries.push_back(listWindow_c::entry(_("Dungeon Cell")));
    entries.push_back(listWindow_c::entry(_("Japanese Room")));
    entries.push_back(listWindow_c::entry(_("Cave")));
  }
  return new listWindow_c(4, 1, 12, 11, surf, gr, _("Choose theme"), "Keep in mind that choosing a new theme will clear the background of your level, if you don't want that press ESC", entries, true);
}

window_c * getAuthorsWindow(surface_c & surf, graphicsN_c & gr, const std::vector<std::string> & auth)
{
  std::vector<listWindow_c::entry> entries;

  for (size_t i = 0; i < auth.size(); i++)
    entries.push_back(listWindow_c::entry(auth[i]));

  if (entries.size())
  {
    entries.back().line = true;

    entries.push_back(listWindow_c::entry(_("Delete an author")));
  }

  entries.push_back(listWindow_c::entry(_("Add an author")));

  return new listWindow_c(4, 1, 12, 11, surf, gr, _("Edit level authors"), "", entries, true);
}

window_c * getAuthorsDelWindow(surface_c & surf, graphicsN_c & gr, const std::vector<std::string> & auth)
{
  std::vector<listWindow_c::entry> entries;

  for (size_t i = 0; i < auth.size(); i++)
    entries.push_back(listWindow_c::entry(auth[i]));

  return new listWindow_c(4, 1, 12, 11, surf, gr, _("Select Author to delete"), "", entries, true);
}

