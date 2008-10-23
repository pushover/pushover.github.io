#ifndef __LEVEL_DISPLAY_H__
#define __LEVEL_DISPLAY_H__

#include "leveldata.h"

#include <SDL.h>

class graphics_c;
class surface_c;

class levelDisplay_c : public levelData_c {

  private:

    int Min, Sec;   // number of minutes and seconds shown in display


    /* this surface contains the background. It is only updated when necessary
     * the content it used to restore stuff behind the sprites
     */
    SDL_Surface * background;

  protected:

    /* 2 bitmasks containing a bit for each block saying if it changed
     * there is one array for the static background and one for the dynamic
     * foreground with the dominos and the ant, the clock, ...
     */
    uint32_t staticDirty[13];

    surface_c & target;
    graphics_c & gr;

  public:

    ~levelDisplay_c(void);

    // initializes a leve. From that moment on the level can only paint into
    // the given surface
    levelDisplay_c(surface_c & target, graphics_c & gr);

    void load(const textsections_c & sections);

    /* update the background where necessary */
    void updateBackground(void);

    /* draw the changed stuff into the target surface */
    void drawDominos(bool debug);
};


#endif

