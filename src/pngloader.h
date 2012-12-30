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

#ifndef __PNG_LOADER_H__
#define __PNG_LOADER_H__

/* this class allows loading certain kinds of png images
 * RGBA, non interlaced images
 * in small portions
 */

#include <SDL.h>
#include <png.h>
#include <string>

class pngLoader_c {

  private:

    FILE * f;
    png_structp png_ptr;
    png_infop info_ptr;

  public:

    pngLoader_c(std::string fname);
    ~pngLoader_c(void);

    bool loaderOk(void) { return png_ptr != 0; }

    unsigned int getWidth(void);
    unsigned int getHeight(void);

    /* fills this surface with a part of the PNG
     * the width of the surface MUST be the width of the
     * image. The number of lines loaded is the height
     * of this surface
     */
    void getPart(SDL_Surface * v);

    void skipLines(unsigned int lines);
};

#endif
