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

#include "graphicsn.h"

#include "pngloader.h"
#include "luaclass.h"

#include <SDL.h>
#include <string.h>
#include <iostream>

static int antOffsets[] = {
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, -9, -30, -24, -24,
  0, 0, -9, -27, -24, -24,
  3, 18, 24, 24,
  3, 18, 24, 24,
  0, -12, -12, -24, -24, -36, -36, -48,
  0, -12, -12, -24, -24, -36, -36, -48,
  0, 12, 12, 24, 24, 36, 36, 48,
  0, 12, 12, 24, 24, 36, 36, 48,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, -12, -24, -24,
  0, 0, 0, -12, -24, -24,
  6, 15, 24, 24, 24, 24,
  6, 15, 24, 24, 24, 24,
  0, -12, -12, -24, -24, -36, -36, -48,
  0, -12, -12, -24, -24, -36, -36, -48,
  0, 12, 12, 24, 24, 36, 36, 48,
  0, 12, 12, 24, 24, 36, 36, 48,
  0,
  3,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0,
  0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 24, 33,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 24, 33,
  0,
  0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, -6,
  0, 0, 0, 0, 0, 0, -6,
  -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, 0, 0,
  -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, 0, 0,
  -6, -6, -6, -6, -6, -6, -6, -6,
  -6, -6, -6, -6, -6, -6, -6, -6,
  -6, -6, -6, -6,
  -6, -6, -6, -6,
  0, 0, 0, 9, 18, 27,
  0, 0, 0, 9, 18, 27,
  0, 12, 24, 36,
  0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  -6, -6,
  0, -12, -24, -36,
  -18, -15, -15, -6, -6, 0, 0,
  0, 0, 0,
  0, 0, -6, -6, -12, -12, -18,
  0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0,
  0,
  -24, -12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};



graphicsN_c::graphicsN_c(const std::string & path) : dataPath(path) {
}

void graphicsN_c::getAnimation(int anim, pngLoader_c * png) {

  static int antOffsetPos = 0;

  for (unsigned int j = 0; j < getAntImages(anim); j++) {

    SDL_Surface * v = SDL_CreateRGBSurface(0, png->getWidth(), 75, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
    SDL_SetAlpha(v, SDL_SRCALPHA | SDL_RLEACCEL, 0);

    png->getPart(v);

    int ofs = antOffsets[antOffsetPos++];

    SDL_Rect dst;

    dst.x = 0;
    dst.y = 0;
    dst.w = 1;
    dst.h = 75;

    SDL_FillRect(v, &dst, SDL_MapRGBA(v->format, 0, 0, 0, 0));

    addAnt(anim, j, ofs, v);
  }
}

void graphicsN_c::loadGraphics(void) {

  // if no data path has been set, we don't even try to load something...
  if (dataPath == "") return;

  /* load domino sprites */

  /* the number of sprites for each domino type is fixed */

  // all domino sprites are in a PNG image load the image and then copy
  // the information to the destination sprites

  {
    pngLoader_c png(dataPath+"/data/dominos.png");

    for (unsigned int i = 0; i < 22; i++)
      for (unsigned int j = 0; j < numDominos[i]; j++) {

        SDL_Surface * v = SDL_CreateRGBSurface(0, png.getWidth(), 58, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
        SDL_SetAlpha(v, SDL_SRCALPHA | SDL_RLEACCEL, 0);

        png.getPart(v);
        setDomino(i, j, v);
        png.skipLines(2);
      }
  }

  // load the ant images

  {
    pngLoader_c png(dataPath+"/data/ant.png");

    // load images from first file
    for (unsigned int i = 0; i <= 27; i++)
      getAnimation(i, &png);

    // load animation 28 and 29 these animations are the same as the 2 animations before but
    // the inverse order
    for (unsigned int i = 28; i <= 29; i++)
      for (unsigned int j = 0; j < getAntImages(i-2); j++)
        addAnt(i, j, getAntOffset(i-2, getAntImages(i-2)-j-1),
            getAnt      (i-2, getAntImages(i-2)-j-1), false);

    for (unsigned int i = 30; i <= 43; i++)
      getAnimation(i, &png);

    // 44 and 45 are again copied from for animations before
    for (unsigned int i = 44; i <= 45; i++)
      for (unsigned int j = 0; j < getAntImages(i-4); j++)
        addAnt(i, j, getAntOffset(i-4, j), getAnt(i-4, j), false);

    for (unsigned int i = 46; i <= 49; i++)
      getAnimation(i, &png);

    // 50 is copied it is the last images of the animation before
    addAnt(50, 0, getAntOffset(49, getAntImages(49)-1),
        getAnt      (49, getAntImages(49)-1), false);

    for (unsigned int i = 51; i <= 65; i++)
      getAnimation(i, &png);
  }

  {
    pngLoader_c png(dataPath+"/data/carried.png");

    for (unsigned int i = 0; i < 7; i++) {
      for (unsigned int j = 0; j < 15; j++) {
        SDL_Surface * v = SDL_CreateRGBSurface(0, png.getWidth(), 55, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
        SDL_SetAlpha(v, SDL_SRCALPHA | SDL_RLEACCEL, 0);

        png.getPart(v);

        setCarriedDomino(i, j, v);
      }
    }

    // copy some surfaces surfaces
    for (unsigned int i = 7; i < 10; i++) {
      for (unsigned int j = 0; j < 15; j++) {
        setCarriedDomino(i, j, SDL_DisplayFormatAlpha(getCarriedDomino(i-1, j)));
      }
    }

    for (unsigned int i = 10; i < 12; i++) {
      for (unsigned int j = 0; j < 15; j++) {
        setCarriedDomino(i, j, SDL_DisplayFormatAlpha(getCarriedDomino(i & 1, j)));
      }
    }

    // load the final surfaces
    for (unsigned int i = 12; i < 16; i++) {
      for (unsigned int j = 0; j < 15; j++) {
        SDL_Surface * v = SDL_CreateRGBSurface(0, png.getWidth(), 55, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
        SDL_SetAlpha(v, SDL_SRCALPHA | SDL_RLEACCEL, 0);

        png.getPart(v);

        setCarriedDomino(i, j, v);
      }
    }
  }

  {
    pngLoader_c png(dataPath+"/data/box.png");

    if (png.getWidth() != 40*3 || png.getHeight() != 48*3)
    {
      std::cout << " oops box image hasn't the right dimensions\n";
      return;
    }

    SDL_Surface * v = SDL_CreateRGBSurface(0, png.getWidth(), 48, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
    SDL_SetAlpha(v, SDL_SRCALPHA | SDL_RLEACCEL, 0);

    for (int i = 0; i < 3; i++) {

      png.getPart(v);

      for (int x = 0; x < 3; x++) {

        SDL_Surface * w = SDL_CreateRGBSurface(0, 40, 48, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
        SDL_SetAlpha(w, SDL_SRCALPHA | SDL_RLEACCEL, 0);

        for (unsigned int y = 0; y < 48; y++)
          memcpy((char*)w->pixels+y*w->pitch,
              (char*)v->pixels+y*v->pitch+x*40*v->format->BytesPerPixel,
              w->pitch);

        addBoxBlock(w);
      }
    }
  }
}

void graphicsN_c::loadTheme(const std::string & name) {

  // if no data path has been set, we don't even try to load something...
  if (dataPath == "") return;

  luaClass_c l;
  l.doFile(dataPath+"/themes/tools.lua");
  l.doFile(dataPath+"/themes/"+name+".lua");

  pngLoader_c png(dataPath+"/themes/"+name+".png");

  SDL_Surface * v = SDL_CreateRGBSurface(0, png.getWidth(), 48, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
  SDL_SetAlpha(v, SDL_SRCALPHA | SDL_RLEACCEL, 0);

  unsigned int xBlocks = png.getWidth()/40;
  unsigned int yBlocks = png.getHeight()/48;

  unsigned int foreSize = l.getArraySize("foreground")/2;
  unsigned int backSize = l.getArraySize("background")/2;

  for (unsigned int i = 0; i < foreSize; i++)
    if (  (unsigned int)l.getNumberArray("foreground", 2*i+2) >= yBlocks
        ||(unsigned int)l.getNumberArray("foreground", 2*i+1) >= xBlocks)
      std::cout << "Warning: Foreground Tile " << i << " is outside of image\n";

  for (unsigned int i = 0; i < backSize; i++)
    if (  (unsigned int)l.getNumberArray("background", 2*i+2) >= yBlocks
        ||(unsigned int)l.getNumberArray("background", 2*i+1) >= xBlocks)
      std::cout << "Warning: Background Tile " << i << " is outside of image\n";

  unsigned int yPos = 0;

  while (yPos < yBlocks)
  {
    png.getPart(v);

    // get possible foreground tiles in the current line
    for (unsigned int i = 0; i < foreSize; i++) {
      if ((unsigned int)l.getNumberArray("foreground", 2*i+2) == yPos) {

        unsigned int x = (unsigned int)l.getNumberArray("foreground", 2*i+1);

        if (x < xBlocks)
        {
          SDL_Surface * w = SDL_CreateRGBSurface(0, 40, 48, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
          SDL_SetAlpha(w, SDL_SRCALPHA | SDL_RLEACCEL, 0);

          for (unsigned int y = 0; y < 48; y++)
            memcpy((char*)w->pixels+y*w->pitch,
                   (char*)v->pixels+y*v->pitch+x*40*v->format->BytesPerPixel,
                   w->pitch);
          addFgTile(i+1, w);
        }
      }
    }

    // get possible background tiles in the current line
    for (unsigned int i = 0; i < backSize; i++) {
      if (l.getNumberArray("background", 2*i+2) == yPos) {

        unsigned int x = (unsigned int)l.getNumberArray("background", 2*i+1);

        if (x < xBlocks)
        {
          SDL_Surface * w = SDL_CreateRGBSurface(0, 40, 48, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
          SDL_SetAlpha(w, SDL_SRCALPHA | SDL_RLEACCEL, 0);

          for (unsigned int y = 0; y < 48; y++)
            memcpy((char*)w->pixels+y*w->pitch,
                   (char*)v->pixels+y*v->pitch+x*40*v->format->BytesPerPixel,
                   w->pitch);
          addBgTile(i, w);
        }
      }
    }

    yPos++;
  }

  SDL_FreeSurface(v);
}


static signed int offsets[12][16] = {
  {     -7, -3,  -8, -3, -11, -3, -14, -3, -16, -3, -20, -3,
  }, {   5, -3,   6, -3,   9, -3,  12, -3,  14, -3,  18, -3,
  }, {  -4, -3,  -6, -5,  -7, -7, -11, -5, -16, -4, -20, -3,
  }, {   2, -3,   4, -5,   5, -7,   9, -5,  14, -4,  18, -3,
  }, { -12, -3, -16, -4, -19, -1, -19,  0, -19,  0, -20, -1,
  }, {   9, -3,  13, -4,  16, -1,  16,  0,  16,  0,  17, -1,
  }, {  -8, -3,  -8, -2,  -7, -3,  -7, -2,  -8, -3,  -8, -2, -7, -3, -7, -2,
  }, {  -8, -3,  -8, -2,  -7, -3,  -7, -2,  -8, -3,  -8, -2, -7, -3, -7, -2,
  }, {  -7, -2,  -7, -3,  -8, -2,  -8, -3,  -7, -2,  -7, -3, -8, -2, -8, -3,
  }, {  -7, -2,  -7, -3,  -8, -2,  -8, -3,  -7, -2,  -7, -3, -8, -2, -8, -3,
  }, {  -5, -3,
  }, {   5, -3,
  }
};

signed int graphicsN_c::getCarryOffsetX(unsigned int animation, unsigned int image) const { return 5*offsets[animation][2*image+0]/2; }
signed int graphicsN_c::getCarryOffsetY(unsigned int animation, unsigned int image) const { return 3*offsets[animation][2*image+1]; }

static signed int moveOffsets[10][64] = {

  // this is a bit complicated:
  // first 2 values are x and y coordinates, they are added to the normal block positions of the domino
  // the 3rd value is the domino to paint, a value less than 32 is a normal domino, it is the domino state to use (7 is straight up...)
  //     a value starting from 32 on is a carried domino image, use that image number...
  // the 4th value is unused

  { // AntAnimPullOutLeft
    0, -2, 7, 0,
    0, -2, 7, 0,
    0, -2, 7, 0,
   -1, -1, 7, 0,
   -2,  0, 7, 0,
   -2,  0, 7, 0,
   -2,  0, 7, 0,
   -2,  0, 7, 0,
   -2,  0, 7, 0,
   -2,  0, 7, 0,
   -2,  0, 7, 0,
   -2, -1, 9, 0,
    0, -3, 32, 0,    // image 0 (carry left)
   -1, -3, 32, 0,
   -4, -3, 32, 0,
  }, { // AntAnimPullOutRight
    0, -2, 7, 0,
    0, -2, 7, 0,
    0, -2, 7, 0,
   -1, -1, 7, 0,
   -2,  0, 7, 0,
   -2,  0, 7, 0,
   -2,  0, 7, 0,
   -2,  0, 7, 0,
   -2,  0, 7, 0,
   -2,  0, 7, 0,
   -2,  0, 7, 0,
   -2, -1, 5, 0,
   -3, -3, 33, 0,    // image 1 (carry right)
   -2, -3, 33, 0,
   -1, -3, 33, 0,
  }, {  // AntAnimPushInLeft,
   -7, -3, 32, 0,
   -8, -3, 32, 0,
  -11, -3, 32, 0,
  -14, -3, 32, 0,
  -17, -3, 32, 0,
  -17, -1, 32, 0,
  -18,  0,  9, 0,
  -18,  0,  7, 0,
  -18,  0,  7, 0,
  -18,  0,  7, 0,
  -18,  0,  7, 0,
  -18,  0,  7, 0,
  -17, -1,  7, 0,
  -16, -2,  7, 0,
  -16, -2,  7, 0,
  -16, -2,  7, 0,
  }, {  // AntAnimPushInRight,
    5, -3, 33, 0,
    6, -3, 33, 0,
   10, -2, 33, 0,
   11, -3, 33, 0,
   14, -3, 33, 0,
   14, -1, 33, 0,
   14,  0,  5, 0,
   14,  0,  7, 0,
   14,  0,  7, 0,
   14,  0,  7, 0,
   14,  0,  7, 0,
   14,  0,  7, 0,
   15, -1,  7, 0,
   16, -2,  7, 0,
   16, -2,  7, 0,
   16, -2,  7, 0,
  }, {  0, -3, 44, 0,  0, -3, 45, 0,     //AntAnimXXX1
  }, { -3, -3, 46, 0, -3, -3, 47, 0,     //AntAnimXXX2
  }, {  0, -3, 45, 0,  0, -3, 44, 0,     //AntAnimXXX3
  }, { -3, -3, 47, 0, -3, -3, 46, 0,     //AntAnimXXX4
  }, { //AntAnimLoosingDominoRight
    16, -3, 7, 0,
    16, -3, 7, 0,
    16, -3, 7, 0,
    16, -3, 7, 0,
  }, { //AntAnimLoosingDominoLeft
    -7, -3, 32, 0,
    -8, -3, 32, 0,
   -11, -3, 32, 0,
   -14, -3, 32, 0,
     0, -3,  7, 0,
     0, -3,  7, 0,
     0, -3,  7, 0,
     0, -3,  7, 0,
  }
};

signed int graphicsN_c::getMoveOffsetX(unsigned int animation, unsigned int image) const { return 5*moveOffsets[animation][4*image+0]/2; }
signed int graphicsN_c::getMoveOffsetY(unsigned int animation, unsigned int image) const { return 3*moveOffsets[animation][4*image+1]; }
signed int graphicsN_c::getMoveImage(unsigned int animation, unsigned int image) const { return moveOffsets[animation][4*image+2]; }

