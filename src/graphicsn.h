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

#ifndef __GRAPHICS_N_H__
#define __GRAPHICS_N_H__

#include "graphics.h"

#include <string>

class pngLoader_c;

/* implementation for graphics class using the original graphics */
class graphicsN_c : public graphics_c {

  public:

    graphicsN_c(const std::string & path);
    ~graphicsN_c(void) {}

    void loadGraphics(void);

    void loadTheme(const std::string & name);

    virtual unsigned int resolutionX(void) const { return 800; }
    virtual unsigned int resolutionY(void) const { return 600; }

    /* to get the block size of one block */
    virtual unsigned int blockX(void) const { return 40; }
    virtual unsigned int blockY(void) const { return 48; }
    virtual unsigned int halveBlockDisplace(void) const { return 8*3; }
    virtual unsigned int antDisplace(void) const { return 6*3; }

    virtual signed int getCarryOffsetX(unsigned int animation, unsigned int image) const;
    virtual signed int getCarryOffsetY(unsigned int animation, unsigned int image) const ;
    virtual signed int getMoveOffsetX(unsigned int animation, unsigned int image) const;
    virtual signed int getMoveOffsetY(unsigned int animation, unsigned int image) const;
    virtual signed int getMoveImage(unsigned int animation, unsigned int image) const;

    virtual int timeXPos(void) const { return 5*18/2; }
    virtual int timeYPos(void) const { return 3*186; }
    virtual int getDominoYStart(void) const { return 3*4; }
    virtual int convertDominoX(int x) const { return 5*x/2; }
    virtual int convertDominoY(int y) const { return 3*y; }
    virtual int splitterY(void) const { return 3*12; }

  private:

    std::string dataPath;

    void getAnimation(int anim, pngLoader_c * png);

};

#endif

