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

#include <string>
#include <vector>

#include <SDL.h>

// one of those boxed windows
// the window may update its content, but stacking the windows.... well I don't know if
// that will be possible, try to keep only one window
// visible at a time
//
// the windows are aligned to the blocks of the screen, the window position
// is given as block position and block size
// this makes updating easier

class surface_c;
class graphics_c;

class window_c {

  protected:

    unsigned char x, y, w, h;
    bool done;
    surface_c & surf;
    graphics_c & gr;

    void clearInside(void);

  public:

    // minimum w and h is 2, but then you don't have space in the middle as everything is taken
    // away by the fame
    window_c(unsigned char x_, unsigned char y_, unsigned char w_, unsigned char h_, surface_c & s, graphics_c & gr);
    virtual ~window_c(void);

    virtual bool handleEvent(const SDL_Event & event) { return false; }

    bool isDone(void) { return done; }
    void resetWindow(void) { done = false; }

    unsigned char X(void) { return x; }
    unsigned char Y(void) { return y; }
    unsigned char W(void) { return w; }
    unsigned char H(void) { return h; }
};

class helpWindow_c : public window_c {

  private:
    std::string help;
    std::vector<uint32_t> pages;
    uint32_t nextPage;

    surface_c & s;
    graphics_c & g;

  private:

    void displayCurrentPage(void);

  public:

    helpWindow_c(const std::string & text, surface_c & s, graphics_c & g);
    bool handleEvent(const SDL_Event & event);

};

// a window that displays a list with selectable entries
class listWindow_c : public window_c {

  public:
    typedef struct entry {
        std::string text;
        bool highlight;
        int sol;  // 0 no normal color, 1 bit, yellow, 2 complete green
        bool line;

        entry(std::string t) : text(t), highlight(false), sol(0), line(false) {}
    } entry;

  private:
    std::vector<entry> entries;
    std::string title;

    unsigned int current;
    bool escape;  // escape works


    unsigned int menuLines;

  public:

    listWindow_c(int x, int y, int w, int h, surface_c & s, graphics_c & gr,
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

    bool escape;

  public:

    InputWindow_c(int x, int y, int w, int h, surface_c & s, graphics_c & gr,
        const std::string & title);

    // the the user has selected something
    const std::string & getText(void) { return input; } // which list entry was selected

    bool hasEscaped(void) const { return escape; }

    virtual bool handleEvent(const SDL_Event & event);
};

class levelsetList_c;
class levelset_c;
class solvedMap_c;

listWindow_c * getMainWindow(surface_c & surf, graphics_c & gr);
listWindow_c * getConfigWindow(surface_c & surf, graphics_c & gr);
listWindow_c * getMissionWindow(const levelsetList_c & ls, surface_c & surf, graphics_c & gr, const std::string & selection);
listWindow_c * getLevelWindow(const levelset_c & ls, const solvedMap_c & solv, surface_c & surf, graphics_c & gr, const std::string & lname);
listWindow_c * getQuitWindow(surface_c & surf, graphics_c & gr);
listWindow_c * getSolvedWindow(surface_c & surf, graphics_c & gr);
listWindow_c * getFailedWindow(int failReason, surface_c & surf, graphics_c & gr);
listWindow_c * getTimeoutWindow(surface_c & surf, graphics_c & gr);
window_c * getAboutWindow(surface_c & surf, graphics_c & gr);
listWindow_c * getProfileWindow(const solvedMap_c & solved, surface_c & surf, graphics_c & gr);
InputWindow_c * getProfileInputWindow(surface_c & surf, graphics_c & gr);
listWindow_c * getProfileSelector(const solvedMap_c & solve, surface_c & surf, graphics_c & gr);

#endif

