#ifndef __LEVEL_DISPLAY_H__
#define __LEVEL_DISPLAY_H__

#include "leveldata.h"
#include "screen.h"

#include <SDL.h>

class graphics_c;

class levelDisplay_c : public levelData_c {

  private:

    int Min, Sec;   // number of minutes and seconds shown in display

  protected:

    /* this surface contains the background. It is only updated when necessary
     * the content it used to restore stuff behind the sprites
     */
    pixelSurface_c background;

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

