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

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "ant.h"
#include "levelset.h"

#include <SDL.h>

#include <string>
#include <vector>

// one of those boxed windows
// the window may update its content, but stacking the windows.... well I don't know if
// that will be possible, try to keep only one window
// visible at a time
//
// the windows are aligned to the blocks of the screen, the window position
// is given as block position and block size
// this makes updating easier

class surface_c;
class graphicsN_c;

class window_c {

  protected:

    unsigned char x, y, w, h;
    bool done;
    surface_c & surf;
    graphicsN_c & gr;

    void clearInside(void);

  protected:
    bool escape;

  public:

    // minimum w and h is 2, but then you don't have space in the middle as everything is taken
    // away by the fame
    window_c(unsigned char x_, unsigned char y_, unsigned char w_, unsigned char h_, surface_c & s, graphicsN_c & gr);
    virtual ~window_c(void);

    virtual bool handleEvent(const SDL_Event & event) { return false; }

    bool isDone(void) { return done; }
    void resetWindow(void) { done = false; }
    bool hasEscaped(void) const { return escape; }

    unsigned char X(void) { return x; }
    unsigned char Y(void) { return y; }
    unsigned char W(void) { return w; }
    unsigned char H(void) { return h; }

    virtual const std::string getText(void) { return ""; }
    virtual unsigned int getSelection(void) { return 0; }

};

class solvedMap_c;

typedef struct configSettings {
  bool useFullscreen;
  bool playMusic;
  bool playSounds;
} configSettings;

window_c * getMainWindow(surface_c & surf, graphicsN_c & gr);
window_c * getConfigWindow(surface_c & surf, graphicsN_c & gr, const configSettings & c, int sel);
window_c * getMissionWindow(const levelsetList_c & ls, const solvedMap_c & solv, surface_c & surf, graphicsN_c & gr, const std::string & selection);
window_c * getLevelWindow(const levelset_c & ls, const solvedMap_c & solv, surface_c & surf, graphicsN_c & gr, const std::string & lname);
window_c * getQuitWindow(bool complete, surface_c & surf, graphicsN_c & gr);
window_c * getSolvedWindow(surface_c & surf, graphicsN_c & gr);
window_c * getFailedWindow(LevelState failReason, surface_c & surf, graphicsN_c & gr);
window_c * getTimeoutWindow(surface_c & surf, graphicsN_c & gr);
window_c * getAboutWindow(surface_c & surf, graphicsN_c & gr, const levelsetList_c &levels);
window_c * getProfileWindow(const solvedMap_c & solved, surface_c & surf, graphicsN_c & gr);
window_c * getProfileInputWindow(surface_c & surf, graphicsN_c & gr);
window_c * getProfileSelector(const solvedMap_c & solve, surface_c & surf, graphicsN_c & gr);
window_c * getHelpWindow(const std::string & mission, const levelData_c & level, DominoType carried, surface_c & surf, graphicsN_c & gr);
window_c * getEditorLevelChooserWindow(surface_c & surf, graphicsN_c & gr, const levelset_c & l);
window_c * getDeleteLevelWindow(surface_c & surf, graphicsN_c & gr, const levelset_c & l);
window_c * getNewLevelWindow(surface_c & surf, graphicsN_c & gr);
window_c * getMessageWindow(surface_c & surf, graphicsN_c & gr, const std::string & title);
window_c * getEditorMenu(surface_c & surf, graphicsN_c & gr);
window_c * getEditorHelp(surface_c & surf, graphicsN_c & gr);
window_c * getThemeSelectorWindow(surface_c & surf, graphicsN_c &gr);
window_c * getAuthorsWindow(surface_c & screen, graphicsN_c & gr, const std::vector<std::string> & auth);
window_c * getAuthorsAddWindow(surface_c & screen, graphicsN_c & gr);
window_c * getAuthorsDelWindow(surface_c & screen, graphicsN_c & gr, const std::vector<std::string> & auth);
window_c * getLevelnameWindow(surface_c & screen, graphicsN_c & gr, const std::string & init);

#endif

