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

    unsigned char X(void) { return x; }
    unsigned char Y(void) { return y; }
    unsigned char W(void) { return w; }
    unsigned char H(void) { return h; }
};

class helpWindow_c : public window_c {

  private:
    std::string help;

  public:

    helpWindow_c(const std::string text, surface_c & s, graphics_c & g);
    bool handleEvent(const SDL_Event & event);

};

// a window that displays a list with selectable entries
class listWindow_c : public window_c {

  private:
    std::vector<std::string> entries;
    std::string title;

    unsigned int current;
    bool escape;  // escape works

    void redraw(void);

    unsigned int menuLines;

  public:

    listWindow_c(int x, int y, int w, int h, surface_c & s, graphics_c & gr,
        const std::string & title, const std::vector<std::string> & entries, bool escape, int initial = 0);

    // the the user has selected something
    unsigned int getSelection(void) { return current; } // which list entry was selected

    virtual bool handleEvent(const SDL_Event & event);
};

class levelsetList_c;
class levelset_c;
class solvedMap_c;

listWindow_c * getMainWindow(surface_c & surf, graphics_c & gr);
listWindow_c * getConfigWindow(surface_c & surf, graphics_c & gr);
listWindow_c * getMissionWindow(const levelsetList_c & ls, surface_c & surf, graphics_c & gr);
listWindow_c * getLevelWindow(const levelset_c & ls, const solvedMap_c & solv, surface_c & surf, graphics_c & gr);
listWindow_c * getQuitWindow(surface_c & surf, graphics_c & gr);
listWindow_c * getLevelWindow(levelset_c & ls, surface_c & surf, graphics_c & gr);
listWindow_c * getSolvedWindow(surface_c & surf, graphics_c & gr);
listWindow_c * getFailedWindow(int failReason, surface_c & surf, graphics_c & gr);
window_c * getAboutWindow(surface_c & surf, graphics_c & gr);

#endif

