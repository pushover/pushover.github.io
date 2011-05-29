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

#include "pngloader.h"

#include "../config.h"

#include <iostream>


pngLoader_c::pngLoader_c(std::string fname) {

  f = fopen(fname.c_str(), "rb");

  if (!f) {
    std::cout << "File not found: " << fname << std::endl;
    return;
  }

  png_uint_32 width, height;
  int bit_depth, color_type, interlace_type;

  // TODO enter error handler in here
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    std::cout << "Can't create PNG reading structure, out of memory" << std::endl;
    return;
  }

  info_ptr = png_create_info_struct(png_ptr);

  if (info_ptr == 0)
  {
    std::cout << "Can't create PNG info structure, out of memory" << std::endl;
    png_destroy_read_struct(&png_ptr, (png_infopp)0, (png_infopp)0);
    png_ptr = 0;
    return;
  }

  if (setjmp(png_jmpbuf(png_ptr)))
  {
    std::cout << "Error in PNG init\n";
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)0);
    png_ptr = 0;
    return;
  }

  png_init_io(png_ptr, f);

  /* Read PNG header info */
  png_read_info(png_ptr, info_ptr);

  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
      &color_type, &interlace_type, NULL, NULL);

  if ((color_type != PNG_COLOR_TYPE_RGB_ALPHA && color_type != PNG_COLOR_TYPE_PALETTE) ||
      interlace_type != PNG_INTERLACE_NONE ||
      (color_type == PNG_COLOR_TYPE_RGB_ALPHA && bit_depth != 8)) {

    if (color_type != PNG_COLOR_TYPE_RGB_ALPHA && color_type != PNG_COLOR_TYPE_PALETTE) printf("color type not ok\n");
    if (interlace_type != PNG_INTERLACE_NONE) printf("interlace mode not right\n");
    if (bit_depth != 8) printf("bitdepth is %i but must be 8\n", bit_depth);


    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)0);
    png_ptr = 0;
    return;
  }

  if (color_type == PNG_COLOR_TYPE_PALETTE)
  {
    png_set_palette_to_rgb(png_ptr);

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    {
      png_set_tRNS_to_alpha(png_ptr);
    }
  }

#ifdef WORDS_BIGENDIAN

  // for bigendian system we swap everything around
  png_set_bgr(png_ptr);
  png_set_swap_alpha(png_ptr);

#endif

  png_read_update_info(png_ptr, info_ptr);
}

pngLoader_c::~pngLoader_c(void) {

  if ( png_ptr )
    png_destroy_read_struct(&png_ptr, info_ptr ? &info_ptr : (png_infopp)0, (png_infopp)0);

  if (f)
    fclose(f);
}

unsigned int pngLoader_c::getWidth(void) {
  return png_get_image_width(png_ptr, info_ptr);
}

unsigned int pngLoader_c::getHeight(void) {
  return png_get_image_height(png_ptr, info_ptr);
}

void pngLoader_c::getPart(SDL_Surface * v) {

  if (!png_ptr) return;

  if (setjmp(png_jmpbuf(png_ptr))) {
    return;
  }

  for (int i = 0; i < v->h; i++)
    png_read_row(png_ptr, ((png_byte*)v->pixels) + i*v->pitch, 0);
}

void pngLoader_c::skipLines(unsigned int lines) {
  if (!png_ptr) return;

  if (setjmp(png_jmpbuf(png_ptr)))
    return;

  png_byte * row = new png_byte[png_get_rowbytes(png_ptr, info_ptr)];

  for (unsigned int i = 0; i < lines; i++)
    png_read_row(png_ptr, row, 0);

  delete [] row;
}

