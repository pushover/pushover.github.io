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

#include "ant.h"
#include "leveldata.h"
#include "screen.h"

#include <SDL.h>
#include <string.h>
#include <iostream>

static const int antOffsets[] = {
  0, 0, 0, 0, 0, 0,                                                 // 6,        // AntAnimWalkLeft,
  0, 0, 0, 0, 0, 0,                                                 // 6,        // AntAnimWalkRight,
  0, 0, -9, -30, -24, -24,                                          // 6,        // AntAnimJunpUpLeft,
  0, 0, -9, -27, -24, -24,                                          // 6,        // AntAnimJunpUpRight,
  3, 18, 24, 24,                                                    // 4,        // AntAnimJunpDownLeft,
  3, 18, 24, 24,                                                    // 4,        // AntAnimJunpDownRight,
  0, -12, -12, -24, -24, -36, -36, -48,                             // 8,        // AntAnimLadder1,
  0, -12, -12, -24, -24, -36, -36, -48,                             // 8,        // AntAnimLadder2,
  0, 12, 12, 24, 24, 36, 36, 48,                                    // 8,        // AntAnimLadder3,
  0, 12, 12, 24, 24, 36, 36, 48,                                    // 8,        // AntAnimLadder4,
  0, 0, 0, 0, 0, 0,                                                 // 6,        // AntAnimCarryLeft,
  0, 0, 0, 0, 0, 0,                                                 // 6,        // AntAnimCarryRight,
  0, 0, 0, -12, -24, -24,                                           // 6,        // AntAnimCarryUpLeft,
  0, 0, 0, -12, -24, -24,                                           // 6,        // AntAnimCarryUpRight,
  6, 15, 24, 24, 24, 24,                                            // 6,        // AntAnimCarryDownLeft,
  6, 15, 24, 24, 24, 24,                                            // 6,        // AntAnimCarryDownRight,
  0, -12, -12, -24, -24, -36, -36, -48,                             // 8,        // AntAnimCarryLadder1,
  0, -12, -12, -24, -24, -36, -36, -48,                             // 8,        // AntAnimCarryLadder2,
  0, 12, 12, 24, 24, 36, 36, 48,                                    // 8,        // AntAnimCarryLadder3,
  0, 12, 12, 24, 24, 36, 36, 48,                                    // 8,        // AntAnimCarryLadder4,
  0,                                                                // 1,        // AntAnimCarryStopLeft,
  3,                                                                // 1,        // AntAnimCarryStopRight,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                      // 15,       // AntAnimPullOutLeft,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                      // 15,       // AntAnimPullOutRight,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                   // 16,       // AntAnimPushInLeft,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                   // 16,       // AntAnimPushInRight,
  0, 0,                                                             // 2,        // AntAnimXXX1,          // 2,        // AntAnimXXX4,
  0, 0,                                                             // 2,        // AntAnimXXX2,          // 2,        // AntAnimXXX3,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 24, 33,                         // 13,       // AntAnimLoosingDominoRight,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 24, 33,             // 17,       // AntAnimLoosingDominoLeft,
  0,                                                                // 1,        // AntAnimStop,
  0, 0,                                                             // 2,        // AntAnimTapping,
  0, 0, 0, 0, 0, 0,                                                 // 6,        // AntAnimYawning,
  0, 0, 0, 0, 0, 0, -6,                                             // 7,        // AntAnimEnterLeft,
  0, 0, 0, 0, 0, 0, -6,                                             // 7,        // AntAnimEnterRight,
  -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, 0, 0,                     // 12,       // AntAnimPushLeft,
  -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, 0, 0,                     // 12,       // AntAnimPushRight,
  -6, -6, -6, -6, -6, -6, -6, -6,                                   // 8,        // AntAnimPushStopperLeft,     // 8,        // AntAnimPushDelayLeft,
  -6, -6, -6, -6, -6, -6, -6, -6,                                   // 8,        // AntAnimPushStopperRight,    // 8,        // AntAnimPushDelayRight,
  -6, -6, -6, -6,                                                   // 4,        // AntAnimPushRiserLeft,
  -6, -6, -6, -6,                                                   // 4,        // AntAnimPushRiserRight,
  0, 0, 0, 9, 18, 27,                                               // 6,        // AntAnimSuddenFallRight,
  0, 0, 0, 9, 18, 27,                                               // 6,        // AntAnimSuddenFallLeft,
  0, 12, 24, 36,                                                    // 4,        // AntAnimFalling,
  0, 0, 0,                                                          // 3,        // AntAnimInFrontOfExploder,    // 1,        // AntAnimInFrontOfExploderWait,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                      // 15,       // AntAnimLanding,
  -6, -6,                                                           // 2,        // AntAnimGhost1,
  0, -12, -24, -36,                                                 // 4,        // AntAnimGhost2,
  -18, -15, -15, -6, -6, 0, 0,                                      // 7,        // AntAnimLeaveDoorEnterLevel,
  0, 0, 0,                                                          // 3,        // AntAnimStepAsideAfterEnter,
  0, 0, -6, -6, -12, -12, -18,                                      // 7,        // AntAnimEnterDoor,
  0, 0, 0, 0,                                                       // 4,        // AntAnimXXX9,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                                  // 11,       // AntAnimStruggingAgainsFallLeft,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                                  // 11,       // AntAnimStruggingAgainsFallRight,
  0, 0, 0, 0, 0, 0, 0, 0,                                           // 8,        // AntAnimVictory,
  0, 0, 0, 0, 0, 0, 0, 0,                                           // 8,        // AntAnimShrugging,
  0, 0, 0, 0, 0, 0, 0, 0,                                           // 8,        // AntAnimNoNo,
  0,                                                                // 1,        // AntAnimXXXA,
  0,                                                                // 1,        // AntAnimDominoDying,
  -24, -12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                         // 13        // AntAnimLandDying,
};

static const unsigned char numDominos[levelData_c::DominoNumber] = {
  0,    // DominoTypeEmpty
  15,   // DominoTypeStandard,
  15,   // DominoTypeStopper,
  14,   // DominoTypeSplitter,
  8,    // DominoTypeExploder,
  15,   // DominoTypeDelay,
  15,   // DominoTypeTumbler,
  15,   // DominoTypeBridger,
  15,   // DominoTypeVanish,
  15,   // DominoTypeTrigger,
  17,   // DominoTypeAscender,
  15,   // DominoTypeConnectedA,
  15,   // DominoTypeConnectedB,
  15,   // DominoTypeCounter1,
  15,   // DominoTypeCounter2,
  15,   // DominoTypeCounter3,
  6,    // DominoTypeCrash0,
  6,    // DominoTypeCrash1,
  6,    // DominoTypeCrash2,
  6,    // DominoTypeCrash3,
  6,    // DominoTypeCrash4,
  6,    // DominoTypeCrash5,
  8,    // DominoTypeRiserCont,
};


#define antDisplace (6*3)

int timeXPos(void) { return 5*18/2; }
int timeYPos(void) { return 3*186; }
int getDominoYStart(void) { return 3*4; }
int convertDominoX(int x) { return 5*x/2; }
int convertDominoY(int y) { return 3*y; }
int splitterY(void) { return 3*12; }

graphicsN_c::graphicsN_c(const std::string & path) : dataPath(path) {
  background = 0;
  ant = 0;
  level = 0;

  dominos.resize(levelData_c::DominoNumber);
  carriedDominos.resize(levelData_c::DominoNumber);

  for (unsigned int i = 0; i < levelData_c::DominoNumber; i++) {
    dominos[i].resize(numDominos[i]);
    carriedDominos[i].resize(levelData_c::DominoTypeLastNormal+1);
    carriedDominos[i][0] = 0;
  }

  antImages.resize((int)AntAnimNothing);

  for (unsigned int i = 0; i < AntAnimNothing; i++) {
    antImages[i].resize(ant_c::getAntImages((AntAnimationState)i));
  }

  // if no data path has been set, we don't even try to load something...
  if (dataPath == "") return;

  /* load domino sprites */

  /* the number of sprites for each domino type is fixed */

  // all domino sprites are in a PNG image load the image and then copy
  // the information to the destination sprites

  {
    pngLoader_c png(dataPath+"/data/dominos.png");

    for (unsigned int i = 0; i < 18; i++)
      for (unsigned int j = 0; j < numDominos[i]; j++) {

        SDL_Surface * v = SDL_CreateRGBSurface(0, png.getWidth(), 58, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
        SDL_SetAlpha(v, SDL_SRCALPHA | SDL_RLEACCEL, 0);

        png.getPart(v);

        dominos[i][j] = v;
        png.skipLines(2);
      }
  }

  // load the ant images

  {
    pngLoader_c png(dataPath+"/data/ant.png");

    // load images from first file
    for (unsigned int i = 0; i <= 27; i++)
      getAnimation((AntAnimationState)i, &png);

    // load animation 28 and 29 these animations are the same as the 2 animations before but
    // the inverse order
    for (unsigned int i = 28; i <= 29; i++)
      for (unsigned int j = 0; j < ant_c::getAntImages((AntAnimationState)(i-2)); j++)
        addAnt(i, j, antImages[i-2][ant_c::getAntImages((AntAnimationState)(i-2))-j-1].ofs,
                     antImages[i-2][ant_c::getAntImages((AntAnimationState)(i-2))-j-1].v, false);

    for (unsigned int i = 30; i <= 43; i++)
      getAnimation((AntAnimationState)i, &png);

    // 44 and 45 are again copied from for animations before
    for (unsigned int i = 44; i <= 45; i++)
      for (unsigned int j = 0; j < ant_c::getAntImages((AntAnimationState)(i-4)); j++)
        addAnt(i, j, antImages[i-4][j].ofs, antImages[i-4][j].v, false);

    for (unsigned int i = 46; i <= 49; i++)
      getAnimation((AntAnimationState)i, &png);

    // 50 is copied it is the last images of the animation before
    addAnt(50, 0, antImages[49][ant_c::getAntImages((AntAnimationState)(49))-1].ofs,
                  antImages[49][ant_c::getAntImages((AntAnimationState)(49))-1].v, false);

    for (unsigned int i = 51; i <= 65; i++)
      getAnimation((AntAnimationState)i, &png);
  }

  {
    pngLoader_c png(dataPath+"/data/carried.png");

    for (unsigned int i = 0; i < 7; i++) {
      for (unsigned int j = 1; j <= levelData_c::DominoTypeLastNormal; j++) {
        SDL_Surface * v = SDL_CreateRGBSurface(0, png.getWidth(), 55, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
        SDL_SetAlpha(v, SDL_SRCALPHA | SDL_RLEACCEL, 0);

        png.getPart(v);

        carriedDominos[i][j] = v;
      }
    }

    // copy some surfaces surfaces
    for (unsigned int i = 7; i < 10; i++) {
      for (unsigned int j = 1; j <= levelData_c::DominoTypeLastNormal; j++) {
        carriedDominos[i][j] = carriedDominos[i-1][j];

      }
    }

    for (unsigned int i = 10; i < 12; i++) {
      for (unsigned int j = 1; j <= levelData_c::DominoTypeLastNormal; j++) {
        carriedDominos[i][j] = carriedDominos[i&1][j];
      }
    }

    // load the final surfaces
    for (unsigned int i = 12; i < 16; i++) {
      for (unsigned int j = 1; j <= levelData_c::DominoTypeLastNormal; j++) {
        SDL_Surface * v = SDL_CreateRGBSurface(0, png.getWidth(), 55, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
        SDL_SetAlpha(v, SDL_SRCALPHA | SDL_RLEACCEL, 0);

        png.getPart(v);

        carriedDominos[i][j] = v;
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

        boxBlocks.push_back(w);
      }
    }
  }
}


graphicsN_c::~graphicsN_c(void) {

  for (unsigned int i = 0; i < bgTiles.size(); i++)
    for (unsigned int j = 0; j < bgTiles[i].size(); j++)
      SDL_FreeSurface(bgTiles[i][j]);

  for (unsigned int i = 0; i < fgTiles.size(); i++)
    for (unsigned int j = 0; j < fgTiles[i].size(); j++)
      SDL_FreeSurface(fgTiles[i][j]);

  for (unsigned int i = 0; i < dominos.size(); i++)
    for (unsigned int j = 0; j < dominos[i].size(); j++)
      if (dominos[i][j])
        SDL_FreeSurface(dominos[i][j]);

  for (unsigned int i = 0; i < antImages.size(); i++)
    for (unsigned int j = 0; j < antImages[i].size(); j++)
      if (antImages[i][j].free)
        SDL_FreeSurface(antImages[i][j].v);

  for (unsigned int i = 0; i < 7; i++)
    for (unsigned int j = 0; j < carriedDominos[i].size(); j++)
      if (carriedDominos[i][j])
        SDL_FreeSurface(carriedDominos[i][j]);

  for (unsigned int i = 12; i < carriedDominos.size(); i++)
    for (unsigned int j = 0; j < carriedDominos[i].size(); j++)
      if (carriedDominos[i][j])
        SDL_FreeSurface(carriedDominos[i][j]);

  for (unsigned int j = 0; j < boxBlocks.size(); j++)
    SDL_FreeSurface(boxBlocks[j]);
}

void graphicsN_c::addAnt(unsigned int anim, unsigned int img, signed char yOffset, SDL_Surface * v, bool free) {

  antSprite s;
  s.v = v;
  s.ofs = yOffset;
  s.free = free;

  antImages[anim][img] = s;
}

void graphicsN_c::getAnimation(AntAnimationState anim, pngLoader_c * png) {

  static int antOffsetPos = 0;

  for (unsigned int j = 0; j < ant_c::getAntImages(anim); j++) {

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

signed int getCarryOffsetX(unsigned int animation, unsigned int image) { return 5*offsets[animation][2*image+0]/2; }
signed int getCarryOffsetY(unsigned int animation, unsigned int image) { return 3*offsets[animation][2*image+1]; }

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

signed int getMoveOffsetX(unsigned int animation, unsigned int image) { return 5*moveOffsets[animation][4*image+0]/2; }
signed int getMoveOffsetY(unsigned int animation, unsigned int image) { return 3*moveOffsets[animation][4*image+1]; }
signed int getMoveImage(unsigned int animation, unsigned int image) { return moveOffsets[animation][4*image+2]; }


void graphicsN_c::drawAnt(void)
{

  if (   !ant->isVisible()
      ||(ant->getAnimation() >= AntAnimLadder1 && ant->getAnimation() <= AntAnimLadder4)
      ||(ant->getAnimation() >= AntAnimCarryLadder1 && ant->getAnimation() <= AntAnimCarryLadder4)
      || (ant->getAnimation() >= AntAnimXXX1 && ant->getAnimation() <= AntAnimXXX4)
     )
  {
    // repaint the ladders
    for (unsigned int y = 0; y < 25; y++)
      for (unsigned int x = 0; x < 20; x++) {
        //      if (dirty.isDirty(x, y))
        {
          if (level->getLadder(x, y))
          {
            target->blitBlock(fgTiles[curTheme][8], x*blockX(), y*blockY()/2);
          }
        }
      }
  }

  if (!ant->isVisible()) return;

  if (ant->getCarriedDomino() != 0)
  {
    if (ant->getAnimation() >= AntAnimPullOutLeft && ant->getAnimation() <= AntAnimLoosingDominoLeft)
    {
      int a = ant->getAnimation() - AntAnimPullOutLeft;

      int x = (ant->getBlockX()-2)*blockX();
      int y = (ant->getBlockY())*blockY()/2+antImages[ant->getAnimation()][ant->getAnimationImage()].ofs+antDisplace;

      y += getMoveOffsetY(a, ant->getAnimationImage());
      x += getMoveOffsetX(a, ant->getAnimationImage());

      int img = getMoveImage(a, ant->getAnimationImage());

      if (img < 32)
      {
        // the ant normaly toppled the domino slightly over then lifting it up, but those
        // images don't exist with some dominos, so we stay with the vertical image
        if (img != 7 && (ant->getCarriedDomino() == levelData_c::DominoTypeSplitter || ant->getCarriedDomino() == levelData_c::DominoTypeExploder || ant->getCarriedDomino() == levelData_c::DominoTypeAscender))
        {
          img = 7;
        }

        target->blit(dominos[ant->getCarriedDomino()][img], x, y);
      }
      else
      {
        target->blit(carriedDominos[img-32][ant->getCarriedDomino()], x, y);
      }
    }
    if (ant->getAnimation() >= AntAnimCarryLeft && ant->getAnimation() <= AntAnimCarryStopRight)
    {
      /* put the domino image of the carried domino */
      int a = ant->getAnimation() - AntAnimCarryLeft;

      target->blit(carriedDominos[a][ant->getCarriedDomino()],
          (ant->getBlockX()-2)*blockX()+getCarryOffsetX(a, ant->getAnimationImage()),
          (ant->getBlockY())*blockY()/2+antImages[ant->getAnimation()][ant->getAnimationImage()].ofs+antDisplace+getCarryOffsetY(a, ant->getAnimationImage()));

    }
  }

  if (antImages[ant->getAnimation()][ant->getAnimationImage()].v)
  {
    target->blit(
        antImages[ant->getAnimation()][ant->getAnimationImage()].v, ant->getBlockX()*blockX()-45,
        (ant->getBlockY())*blockY()/2+antImages[ant->getAnimation()][ant->getAnimationImage()].ofs+antDisplace+3);
  }

  /* what comes now, is to put the ladders back in front of the ant,
   * we don't need to do that, if the ant is on the ladder
   */

  if (ant->getAnimation() >= AntAnimLadder1 && ant->getAnimation() <= AntAnimLadder4) return;
  if (ant->getAnimation() >= AntAnimCarryLadder1 && ant->getAnimation() <= AntAnimCarryLadder4) return;
  if (ant->getAnimation() >= AntAnimXXX1 && ant->getAnimation() <= AntAnimXXX4) return;

  // repaint the ladders
  for (unsigned int y = 0; y < 25; y++)
    for (unsigned int x = 0; x < 20; x++) {
      //      if (dirty.isDirty(x, y))
      {
        if (level->getLadder(x, y))
        {
          target->blitBlock(fgTiles[curTheme][8], x*blockX(), y*blockY()/2);
        }
      }
    }
}

void graphicsN_c::setTheme(const std::string & name)
{

  // if no data path has been set, we don't even try to load something...
  if (dataPath == "") return;

  for (unsigned int th = 0; th < themeNames.size(); th++) {
    if (themeNames[th] == name) {
      curTheme = th;
      return;
    }
  }

  themeNames.push_back(std::string(name));
  curTheme = themeNames.size()-1;

  bgTiles.resize(bgTiles.size()+1);
  fgTiles.resize(fgTiles.size()+1);

  luaClass_c l;
  l.doFile(dataPath+"/themes/tools.lua");
  l.doFile(dataPath+"/themes/"+name+".lua");

  pngLoader_c png(dataPath+"/themes/"+name+".png");

  SDL_Surface * v = SDL_CreateRGBSurface(0, png.getWidth(), blockY(), 32, 0xff, 0xff00, 0xff0000, 0xff000000);
  SDL_SetAlpha(v, SDL_SRCALPHA | SDL_RLEACCEL, 0);

  unsigned int xBlocks = png.getWidth()/blockX();
  unsigned int yBlocks = png.getHeight()/blockY();

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
          SDL_Surface * w1 = SDL_CreateRGBSurface(0, blockX(), blockY()/2, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
          SDL_SetAlpha(w1, SDL_SRCALPHA | SDL_RLEACCEL, 0);

          SDL_Surface * w2 = SDL_CreateRGBSurface(0, blockX(), blockY()/2, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
          SDL_SetAlpha(w2, SDL_SRCALPHA | SDL_RLEACCEL, 0);

          for (unsigned int y = 0; y < blockY()/2; y++)
          {
            memcpy((char*)w1->pixels+y*w1->pitch,
                   (char*)v->pixels+y*v->pitch+x*40*v->format->BytesPerPixel,
                   w1->pitch);
            memcpy((char*)w2->pixels+y*w2->pitch,
                   (char*)v->pixels+(y+blockY()/2)*v->pitch+x*40*v->format->BytesPerPixel,
                   w2->pitch);
          }

          if (2*i+1 >= fgTiles[curTheme].size())
            fgTiles[curTheme].resize(2*i+2);

          fgTiles[curTheme][2*i] = w1;
          fgTiles[curTheme][2*i+1] = w2;
        }
      }
    }

    // get possible background tiles in the current line
    for (unsigned int i = 0; i < backSize; i++) {
      if (l.getNumberArray("background", 2*i+2) == yPos) {

        unsigned int x = (unsigned int)l.getNumberArray("background", 2*i+1);

        if (x < xBlocks)
        {
          SDL_Surface * w1 = SDL_CreateRGBSurface(0, blockX(), blockY()/2, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
          SDL_SetAlpha(w1, SDL_SRCALPHA | SDL_RLEACCEL, 0);

          SDL_Surface * w2 = SDL_CreateRGBSurface(0, blockX(), blockY()/2, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
          SDL_SetAlpha(w2, SDL_SRCALPHA | SDL_RLEACCEL, 0);

          for (unsigned int y = 0; y < blockY()/2; y++)
          {
            memcpy((char*)w1->pixels+y*w1->pitch,
                   (char*)v->pixels+y*v->pitch+x*40*v->format->BytesPerPixel,
                   w1->pitch);
            memcpy((char*)w2->pixels+y*w2->pitch,
                   (char*)v->pixels+(y+blockY()/2)*v->pitch+x*40*v->format->BytesPerPixel,
                   w2->pitch);
          }

          if (2*i+1 >= bgTiles[curTheme].size())
            bgTiles[curTheme].resize(2*i+2);

          bgTiles[curTheme][2*i  ] = w1;
          bgTiles[curTheme][2*i+1] = w2;
        }
      }
    }

    yPos++;
  }

  SDL_FreeSurface(v);

}

#define FG_DOORSTART 34

void graphicsN_c::setPaintData(const levelData_c * l, const ant_c * a, surface_c * t)
{
  level = l;
  ant = a;
  target = t;

  setTheme(level->getTheme());

  dirtybg.markAllDirty();
  dirty.markAllDirty();

  Min = Sec = -1;

  if (!background) background = new surface_c(t->getIdentical());
}

void graphicsN_c::drawDominos(void)
{

  // update background
  for (unsigned int y = 0; y < 25; y++)
  {
    for (unsigned int x = 0; x < 20; x++)
    {
      // when the current block is dirty, recreate it
//      if (dirtybg.isDirty(x, y))
      {
        for (unsigned char b = 0; b < level->getNumBgLayer(); b++)
          background->blitBlock(bgTiles[curTheme][level->getBg(x, y, b)], x*blockX(), y*blockY()/2);

        // blit the doors
        if (x == level->getEntryX() && y+1 == level->getEntryY())
          background->blitBlock(fgTiles[curTheme][FG_DOORSTART+0+2*level->getEntryState()], x*blockX(), y*blockY()/2);
        if (x == level->getEntryX() && y   == level->getEntryY())
          background->blitBlock(fgTiles[curTheme][FG_DOORSTART+1+2*level->getEntryState()], x*blockX(), y*blockY()/2);

        if (x == level->getExitX() && y+1 == level->getExitY())
          background->blitBlock(fgTiles[curTheme][FG_DOORSTART+0+2*level->getExitState()], x*blockX(), y*blockY()/2);
        if (x == level->getExitX() && y   == level->getExitY())
          background->blitBlock(fgTiles[curTheme][FG_DOORSTART+1+2*level->getExitState()], x*blockX(), y*blockY()/2);

        // blit platforms
        if (y < 24 && level->getPlatform(x, y+1))
        {
          if (!level->getPlatform(x-1, y+1) && !level->getPlatform(x+1, y+1))
            background->blitBlock(fgTiles[curTheme][30], x*blockX(), y*blockY()/2);

          if (level->getPlatform(x-1, y+1) && !level->getPlatform(x+1, y+1))
            background->blitBlock(fgTiles[curTheme][4], x*blockX(), y*blockY()/2);

          if (!level->getPlatform(x-1, y+1) && level->getPlatform(x+1, y+1))
            background->blitBlock(fgTiles[curTheme][0], x*blockX(), y*blockY()/2);

          if (level->getPlatform(x-1, y+1) && level->getPlatform(x+1, y+1))
            background->blitBlock(fgTiles[curTheme][2], x*blockX(), y*blockY()/2);
        }

        if (y > 0 && level->getPlatform(x, y))
        {
          if (!level->getPlatform(x-1, y) && !level->getPlatform(x+1, y))
            background->blitBlock(fgTiles[curTheme][31], x*blockX(), y*blockY()/2);

          if (level->getPlatform(x-1, y) && !level->getPlatform(x+1, y))
            background->blitBlock(fgTiles[curTheme][5], x*blockX(), y*blockY()/2);

          if (!level->getPlatform(x-1, y) && level->getPlatform(x+1, y))
            background->blitBlock(fgTiles[curTheme][1], x*blockX(), y*blockY()/2);

          if (level->getPlatform(x-1, y) && level->getPlatform(x+1, y))
            background->blitBlock(fgTiles[curTheme][3], x*blockX(), y*blockY()/2);
        }

        background->gradient(blockX()*x, blockY()/2*y, blockX(), blockY()/2);
      }
    }

  }
  dirtybg.clearDirty();

  int timeLeft = level->getTimeLeft();

  // the dirty marks for the clock
  {

    // calculate the second left
    int tm = timeLeft/18;

    // if negative make positive again
    if (timeLeft < 0)
      tm = -tm+1;

    int newSec = tm%60;
    int newMin = tm/60;

    if (newSec != Sec || timeLeft == -1)
    {
      dirty.markDirty(3, 11);
      dirty.markDirty(3, 12);
    }

    if (newSec != Sec || newMin != Min || timeLeft % 18 == 17 || timeLeft % 18 == 8 || timeLeft == -1)
    {
      dirty.markDirty(2, 11);
      dirty.markDirty(2, 12);
    }

    if (newMin != Min || timeLeft == -1)
    {
      dirty.markDirty(1, 11);
      dirty.markDirty(1, 12);
    }

    Min = newMin;
    Sec = newSec;
  }

  // copy background, where necessary
  for (unsigned int y = 0; y < 13; y++)
    for (unsigned int x = 0; x < 20; x++)
      if (dirty.isDirty(x, y))
        target->copy(*background, x*blockX(), y*blockY(), blockX(), blockY());

  static int XposOffset[] = {-16, -16,  0,-16,  0,  0, 0, 0, 0,  0, 0, 16,  0, 16, 16, 0};
  static int YposOffset[] = { -8,  -6,  0, -4,  0, -2, 0, 0, 0, -2, 0, -4,  0, -6, -8, 0};
  static int StoneImageOffset[] = {  7, 6, 0, 5, 0, 4, 0, 0, 0, 3, 0, 2, 0, 1, 0, 0};

  // the idea behind this code is to repaint the dirty blocks. Dominos that are actually
  // within neighbor block must be repaint, too, when they might reach into the actual
  // block. But painting the neighbors is only necessary, when they are not drawn on
  // their own anyway, so always check for !dirty of the "home-block" of each domino

  int SpriteYPos = getDominoYStart();

  for (int y = 0; y < 25; y++, SpriteYPos += blockY()/2) {

    int SpriteXPos = -2*blockX();

    for (int x = 0; x < 20; x++, SpriteXPos += blockX()) {

//      if (!dirty.isDirty(x, y)) continue;

      // paint the left neighbor domino, if it leans in our direction and is not painted on its own
      if (y < 25 && x > 0 && !dirty.isDirty(x-1, y+1) && level->getDominoType(x-1, y+1) != levelData_c::DominoTypeEmpty &&
          (level->getDominoState(x-1, y+1) > 8 ||
           (level->getDominoType(x-1, y+1) == levelData_c::DominoTypeSplitter && level->getDominoState(x-1, y+1) != 8) ||
           level->getDominoState(x-1, y+1) >= levelData_c::DominoTypeCrash0))
      {
        target->blit(dominos[level->getDominoType(x-1, y+1)][level->getDominoState(x-1, y+1)-1],
            SpriteXPos-blockX(),
            SpriteYPos+convertDominoY(level->getDominoYOffset(x-1, y+1))+blockY()/2);
      }

      if (x > 0 && !dirty.isDirty(x-1, y) && level->getDominoType(x-1, y) != levelData_c::DominoTypeEmpty &&
          (level->getDominoState(x-1, y) > 8 ||
           (level->getDominoType(x-1, y) == levelData_c::DominoTypeSplitter && level->getDominoState(x-1, y) != 8) ||
           level->getDominoType(x-1, y) >= levelData_c::DominoTypeCrash0))
      {
        target->blit(dominos[level->getDominoType(x-1, y)][level->getDominoState(x-1, y)-1],
            SpriteXPos-blockX(),
            SpriteYPos+convertDominoY(level->getDominoYOffset(x-1, y)));
      }

      if (y < 25 && !dirty.isDirty(x, y+1) && level->getDominoType(x, y+1) != levelData_c::DominoTypeEmpty)
      {
        target->blit(dominos[level->getDominoType(x, y+1)][level->getDominoState(x, y+1)-1],
            SpriteXPos,
            SpriteYPos+convertDominoY(level->getDominoYOffset(x, y+1))+blockY()/2);
      }

      // paint the splitting domino for the splitter
      if (level->getDominoType(x, y) == levelData_c::DominoTypeSplitter &&
          level->getDominoState(x, y) == 6 &&
          level->getDominoExtra(x, y) != 0)
      {
        target->blit(dominos[level->getDominoExtra(x, y)][level->getDominoExtra(x, y)>=levelData_c::DominoTypeCrash0?0:7],
            SpriteXPos,
            SpriteYPos-splitterY());
      }

      // paint the actual domino but take care of the special cases of the ascender domino
      if (level->getDominoType(x, y) == levelData_c::DominoTypeAscender && level->getDominoExtra(x, y) == 0x60 &&
          level->getDominoState(x, y) < 16 && level->getDominoState(x, y) != 8)
      {
        target->blit(dominos[levelData_c::DominoTypeRiserCont][StoneImageOffset[level->getDominoState(x, y)-1]],
            SpriteXPos+convertDominoX(XposOffset[level->getDominoState(x, y)-1]),
            SpriteYPos+convertDominoY(YposOffset[level->getDominoState(x, y)-1]+level->getDominoYOffset(x, y)));
      }
      else if (level->getDominoType(x, y) == levelData_c::DominoTypeAscender && level->getDominoState(x, y) == 1 && level->getDominoExtra(x, y) == 0 &&
          !level->getPlatform(x-1, y-3))
      { // this is the case of the ascender domino completely horizontal and with the plank it is below not existing
        // so we see the above face of the domino. Normally there is a wall above us so we only see
        // the front face of the domino
        target->blit(dominos[levelData_c::DominoTypeRiserCont][StoneImageOffset[level->getDominoState(x, y)-1]],
            SpriteXPos+convertDominoX(XposOffset[level->getDominoState(x, y)-1]+6),
            SpriteYPos+convertDominoY(YposOffset[level->getDominoState(x, y)-1]+level->getDominoYOffset(x, y)));
      }
      else if (level->getDominoType(x, y) == levelData_c::DominoTypeAscender && level->getDominoState(x, y) == 15 && level->getDominoExtra(x, y) == 0 &&
          !level->getPlatform(x+1, y-3))
      {
        target->blit(dominos[levelData_c::DominoTypeRiserCont][StoneImageOffset[level->getDominoState(x, y)-1]],
            SpriteXPos+convertDominoX(XposOffset[level->getDominoState(x, y)-1]-2),
            SpriteYPos+convertDominoY(YposOffset[level->getDominoState(x, y)-1]+level->getDominoYOffset(x, y)));
      }
      else if (level->getDominoType(x, y) != levelData_c::DominoTypeEmpty)
      {
        target->blit(dominos[level->getDominoType(x, y)][level->getDominoState(x, y)-1],
            SpriteXPos,
            SpriteYPos+convertDominoY(level->getDominoYOffset(x, y)));
      }

      // paint the right neighbor if it is leaning in our direction
      if (x < 19 && y < 25 && !dirty.isDirty(x+1, y+1) && level->getDominoType(x+1, y+1) != levelData_c::DominoTypeEmpty &&
          (level->getDominoState(x+1, y+1) < 8 ||
           (level->getDominoType(x+1, y+1) == levelData_c::DominoTypeSplitter && level->getDominoState(x+1, y+1) != 8) ||
           level->getDominoType(x+1, y+1) >= levelData_c::DominoTypeCrash0))
      {
        target->blit(dominos[level->getDominoType(x+1, y+1)][level->getDominoState(x+1, y+1)-1],
            SpriteXPos+blockX(),
            SpriteYPos+convertDominoY(level->getDominoYOffset(x+1, y+1))+blockY()/2);
      }

      if (x < 19 && !dirty.isDirty(x+1, y) && level->getDominoType(x+1, y) != levelData_c::DominoTypeEmpty &&
          (level->getDominoState(x+1, y) < 8 ||
           (level->getDominoType(x+1, y) == levelData_c::DominoTypeSplitter && level->getDominoState(x+1, y) != 8) ||
           level->getDominoType(x+1, y) >= levelData_c::DominoTypeCrash0))
      {
        target->blit(dominos[level->getDominoType(x+1, y)][level->getDominoState(x+1, y)-1],
            SpriteXPos+blockX(),
            SpriteYPos+convertDominoY(level->getDominoYOffset(x+1, y)));
      }

      if (y >= 24) continue;

      if (!dirty.isDirty(x, y+2) && level->getDominoType(x, y+2) == levelData_c::DominoTypeAscender)
      {
        target->blit(dominos[level->getDominoType(x, y+2)][level->getDominoState(x, y+2)-1],
            SpriteXPos,
            SpriteYPos+convertDominoY(level->getDominoYOffset(x, y+2))+blockY());
      }

      if (x > 0 && !dirty.isDirty(x-1, y+2) && level->getDominoType(x-1, y+2) == levelData_c::DominoTypeAscender)
      {
        target->blit(dominos[level->getDominoType(x-1, y+2)][level->getDominoState(x-1, y+2)-1],
            SpriteXPos-blockX(),
            SpriteYPos+convertDominoY(level->getDominoYOffset(x-1, y+2))+blockY());
      }

      if (x < 19 && !dirty.isDirty(x+1, y+2) && level->getDominoType(x+1, y+2) == levelData_c::DominoTypeAscender)
      {
        target->blit(dominos[level->getDominoType(x+1, y+2)][level->getDominoState(x+1, y+2)-1],
            SpriteXPos+blockX(),
            SpriteYPos+convertDominoY(level->getDominoYOffset(x+1, y+2))+blockY());
      }

      if (level->getDominoType(x, y) != levelData_c::DominoTypeAscender) continue;

      if (!dirty.isDirty(x, y+2) && level->getDominoType(x, y+2) != levelData_c::DominoTypeEmpty)
      {
        target->blit(dominos[level->getDominoType(x, y+2)][level->getDominoState(x, y+2)-1],
            SpriteXPos,
            SpriteYPos+convertDominoY(level->getDominoYOffset(x, y+2))+blockY());
      }

      if (x > 0 && !dirty.isDirty(x-1, y+2) && level->getDominoType(x-1, y+2) != levelData_c::DominoTypeEmpty)
      {
        target->blit(dominos[level->getDominoType(x-1, y+2)][level->getDominoState(x-1, y+2)-1],
            SpriteXPos-blockX(),
            SpriteYPos+convertDominoY(level->getDominoYOffset(x-1, y+2))+blockY());
      }

      if (x >= 19) continue;

      if (!dirty.isDirty(x+1, y+2)) continue;

      if (level->getDominoType(x+1, y+2) == levelData_c::DominoTypeEmpty) continue;

      target->blit(dominos[level->getDominoType(x+1, y+2)][level->getDominoState(x+1, y+2)-1],
          SpriteXPos+blockX(),
          SpriteYPos+convertDominoY(level->getDominoYOffset(x+1, y+2))+blockY());
    }
  }

}

void graphicsN_c::drawLevel(void) {

  if (!level || !ant || !target) return;

  markAllDirty();

  drawDominos();
  drawAnt();

  int timeLeft = level->getTimeLeft();

  if (timeLeft < 60*60*18)
  { // output the time
    char time[6];
    snprintf(time, 6, "%02i:%02i", Min, Sec);

    fontParams_s pars;
    if (timeLeft >= 0)
    {
      pars.color.r = pars.color.g = 255; pars.color.b = 0;
    }
    else
    {
      pars.color.r = 255; pars.color.g = pars.color.b = 0;
    }
    pars.font = FNT_BIG;
    pars.alignment = ALN_TEXT;
    pars.box.x = timeXPos();
    pars.box.y = timeYPos();
    pars.box.w = 50;
    pars.box.h = 50;
    pars.shadow = 1;

    target->renderText(&pars, time);
  }
}



