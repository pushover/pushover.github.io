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

#ifndef __LEVEL_DISPLAY_H__
#define __LEVEL_DISPLAY_H__

#include "leveldata.h"
#include "screen.h"

#include <SDL.h>

class graphics_c;

class levelDisplay_c : public levelData_c {

  private:

    int Min, Sec;   // number of minutes and seconds shown in display

    /* this surface contains the background. It is only updated when necessary
     * the content it used to restore stuff behind the sprites
     */
    surface_c background;

    surface_c & target;
    graphics_c & gr;

  public:

    ~levelDisplay_c(void);

    // initializes a level. From that moment on the level can only paint into
    // the given surface
    levelDisplay_c(surface_c & target, graphics_c & gr);

    void load(const textsections_c & sections, const std::string & userString);

    /* update the background where necessary */
    void updateBackground(void);

    /* draw the changed stuff into the target surface */
    void drawDominos(void);

    void markDirty(int x, int y) { target.markDirty(x, y); }
    void markDirtyBg(int x, int y) { background.markDirty(x, y); target.markDirty(x, y); }
    bool isDirty(int x, int y) { return target.isDirty(x, y); }
};


#endif

