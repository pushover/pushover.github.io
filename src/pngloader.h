#ifndef __PNG_LOADER_H__
#define __PNG_LOADER_H__

/* this class allows loading certain kinds of png images
 * RGBA, non interlaced images
 * in small portions
 */

#include <SDL.h>

#include <png.h>

#include <string>

#include <stdio.h>

class pngLoader_c {

  private:

    FILE * f;
    png_structp png_ptr;
    png_infop info_ptr;

  public:

    pngLoader_c(std::string fname);
    ~pngLoader_c(void);

    unsigned int getWidth(void);

    /* fills this surface with a part of the png
     * the width of the surface MUST be the width of the
     * image. The number of lines loaded is the hight
     * of this surface
     */
    void getPart(SDL_Surface * v);

    void skipLines(unsigned int lines);
};

#endif
