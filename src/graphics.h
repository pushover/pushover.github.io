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

#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <SDL.h>
#include <string>
#include <vector>
#include <map>

class ant_c;
class surface_c;
class levelDisplay_c;

/* this class contains all the information for all graphics */
class graphics_c {

  public:

    graphics_c(void);
    virtual ~graphics_c(void);

    // needs to be overloaded with a function loading
    // dominos, ant images and the box images
    virtual void loadGraphics(void) = 0;

    /* to get the resolution that should be used */
    virtual unsigned int resolutionX(void) const = 0;
    virtual unsigned int resolutionY(void) const = 0;

    /* to get the block size of one block */
    virtual unsigned int blockX(void) const = 0;
    virtual unsigned int blockY(void) const = 0;

    virtual unsigned int halveBlockDisplace(void) const = 0;  // return and noffset to actually place the objects

    // needs to be overloaded containing the loading function for a theme
    virtual void loadTheme(const std::string & name) = 0;

    // the position of the time in the level
    virtual int timeXPos(void) const = 0;
    virtual int timeYPos(void) const = 0;

    virtual int getDominoYStart(void) const = 0;
    virtual int convertDominoX(int x) const = 0;
    virtual int convertDominoY(int y) const = 0;
    virtual int splitterY(void) const = 0;


    virtual void drawLevel(void) = 0;

};

#endif

