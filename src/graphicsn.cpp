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
#include "colors.h"

#include <SDL.h>
#include <string.h>
#include <iostream>

#include <assert.h>

// the following array contains the y offset to use for the different
// images of the different ant animation images
// each row represents one animation, the comment shows how many
// images the animation has and which animation it is
static const int antOffsets[] = {
  0, 0, 0, 0, 0, 0,                                                 // 6,   AntAnimWalkLeft,
  0, 0, 0, 0, 0, 0,                                                 // 6,   AntAnimWalkRight,
  0, 0, -9, -30, -24, -24,                                          // 6,   AntAnimJunpUpLeft,
  0, 0, -9, -27, -24, -24,                                          // 6,   AntAnimJunpUpRight,
  3, 18, 24, 24,                                                    // 4,   AntAnimJunpDownLeft,
  3, 18, 24, 24,                                                    // 4,   AntAnimJunpDownRight,
  0, -12, -12, -24, -24, -36, -36, -48,                             // 8,   AntAnimLadder1,
  0, -12, -12, -24, -24, -36, -36, -48,                             // 8,   AntAnimLadder2,
  0, 12, 12, 24, 24, 36, 36, 48,                                    // 8,   AntAnimLadder3,
  0, 12, 12, 24, 24, 36, 36, 48,                                    // 8,   AntAnimLadder4,
  0, 0, 0, 0, 0, 0,                                                 // 6,   AntAnimCarryLeft,
  0, 0, 0, 0, 0, 0,                                                 // 6,   AntAnimCarryRight,
  0, 0, 0, -12, -24, -24,                                           // 6,   AntAnimCarryUpLeft,
  0, 0, 0, -12, -24, -24,                                           // 6,   AntAnimCarryUpRight,
  6, 15, 24, 24, 24, 24,                                            // 6,   AntAnimCarryDownLeft,
  6, 15, 24, 24, 24, 24,                                            // 6,   AntAnimCarryDownRight,
  0, -12, -12, -24, -24, -36, -36, -48,                             // 8,   AntAnimCarryLadder1,
  0, -12, -12, -24, -24, -36, -36, -48,                             // 8,   AntAnimCarryLadder2,
  0, 12, 12, 24, 24, 36, 36, 48,                                    // 8,   AntAnimCarryLadder3,
  0, 12, 12, 24, 24, 36, 36, 48,                                    // 8,   AntAnimCarryLadder4,
  0,                                                                // 1,   AntAnimCarryStopLeft,
  0,                                                                // 1,   AntAnimCarryStopRight,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                      // 15,  AntAnimPullOutLeft,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                      // 15,  AntAnimPullOutRight,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                   // 16,  AntAnimPushInLeft,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                   // 16,  AntAnimPushInRight,
  0, 0,                                                             // 2,   AntAnimXXX1,               // 2,  AntAnimXXX4,
  0, 0,                                                             // 2,   AntAnimXXX2,               // 2,  AntAnimXXX3,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 24, 33,                         // 13,  AntAnimLoosingDominoRight,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 24, 33,             // 17,  AntAnimLoosingDominoLeft,
  0,                                                                // 1,   AntAnimStop,
  0, 0,                                                             // 2,   AntAnimTapping,
  0, 0, 0, 0, 0, 0,                                                 // 6,   AntAnimYawning,
  0, 0, 0, 0, 0, 0, -6,                                             // 7,   AntAnimEnterLeft,
  0, 0, 0, 0, 0, 0, -6,                                             // 7,   AntAnimEnterRight,
  -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, 0, 0,                     // 12,  AntAnimPushLeft,
  -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, 0, 0,                     // 12,  AntAnimPushRight,
  -6, -6, -6, -6, -6, -6, -6, -6,                                   // 8,   AntAnimPushStopperLeft,    // 8,  AntAnimPushDelayLeft,
  -6, -6, -6, -6, -6, -6, -6, -6,                                   // 8,   AntAnimPushStopperRight,   // 8,  AntAnimPushDelayRight,
  -6, -6, -6, -6,                                                   // 4,   AntAnimPushRiserLeft,
  -6, -6, -6, -6,                                                   // 4,   AntAnimPushRiserRight,
  0, 0, 0, 9, 18, 27,                                               // 6,   AntAnimSuddenFallRight,
  0, 0, 0, 9, 18, 27,                                               // 6,   AntAnimSuddenFallLeft,
  0, 12,                                                            // 4,   AntAnimFalling,
  0, 0, 0,                                                          // 3,   AntAnimInFrontOfExploder,  // 1,  AntAnimInFrontOfExploderWait,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                      // 15,  AntAnimLanding,
  -6, -6,                                                           // 2,   AntAnimGhost1,
  0, -12, -24, -36,                                                 // 4,   AntAnimGhost2,
  -18, -15, -15, -6, -6, 0, 0,                                      // 7,   AntAnimLeaveDoorEnterLevel,
  0, 0, 0,                                                          // 3,   AntAnimStepAsideAfterEnter,
  0, 0, -6, -6, -12, -12, -18,                                      // 7,   AntAnimEnterDoor,
  0, 0, 0, 0,                                                       // 4,   AntAnimXXX9,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                                  // 11,  AntAnimStruggingAgainsFallLeft,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                                  // 11,  AntAnimStruggingAgainsFallRight,
  0, 0, 0, 0, 0, 0, 0, 0,                                           // 8,   AntAnimVictory,
  0, 0, 0, 0, 0, 0, 0, 0,                                           // 8,   AntAnimShrugging,
  0, 0, 0, 0, 0, 0, 0, 0,                                           // 8,   AntAnimNoNo,
  0,                                                                // 1,   AntAnimXXXA,
  0,                                                                // 1,   AntAnimDominoDying,
  -24, -12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                         // 13   AntAnimLandDying,
};



#define CARRIED_DOMINO_START DO_ST_NUM  // 7 images
#define RISER_CONT_START (CARRIED_DOMINO_START+7)      // 8 images

// this array defines for all domino images of a certain comino
// in which array position they are supposed to go
// finalizing entry is -1
// drop image is done with -2
static const int8_t dominoImages[DominoNumber][135] = {

  /* DominoTypeEmpty      */ { -1 },
  /* DominoTypeStandard   */ {
    DO_ST_FALLING+0, DO_ST_FALLING+1, DO_ST_FALLING+2, DO_ST_FALLING+3, DO_ST_FALLING+4,
    DO_ST_FALLING+5, DO_ST_FALLING+6, DO_ST_FALLING+7, DO_ST_FALLING+8, DO_ST_FALLING+9,
    DO_ST_FALLING+10, DO_ST_FALLING+11, DO_ST_FALLING+12, DO_ST_FALLING+13, DO_ST_FALLING+14,
    CARRIED_DOMINO_START+0, CARRIED_DOMINO_START+1, CARRIED_DOMINO_START+2, CARRIED_DOMINO_START+3,
    CARRIED_DOMINO_START+4, CARRIED_DOMINO_START+5, CARRIED_DOMINO_START+6, -1 },
  /* DominoTypeStopper    */ {
    DO_ST_FALLING+5, DO_ST_FALLING+7, DO_ST_FALLING+9,
    CARRIED_DOMINO_START+0, CARRIED_DOMINO_START+1, CARRIED_DOMINO_START+2, CARRIED_DOMINO_START+3,
    CARRIED_DOMINO_START+4, CARRIED_DOMINO_START+5, CARRIED_DOMINO_START+6, -1 },
  /* DominoTypeSplitter   */ {
    DO_ST_FALLING+5, DO_ST_FALLING+7, DO_ST_FALLING+9,
    DO_ST_SPLIT+6, DO_ST_SPLIT+5, DO_ST_SPLIT+4, DO_ST_SPLIT+3, DO_ST_SPLIT+2, DO_ST_SPLIT+1, DO_ST_SPLIT+0,
    DO_ST_SPLIT+7, DO_ST_SPLIT+8, DO_ST_SPLIT+9, DO_ST_SPLIT+10, DO_ST_SPLIT+11, DO_ST_SPLIT+12,
    CARRIED_DOMINO_START+0, CARRIED_DOMINO_START+1, CARRIED_DOMINO_START+2, CARRIED_DOMINO_START+3,
    CARRIED_DOMINO_START+4, CARRIED_DOMINO_START+5, CARRIED_DOMINO_START+6, -1 },
  /* DominoTypeExploder   */ {
    DO_ST_FALLING+5, DO_ST_FALLING+7, DO_ST_FALLING+9,
    DO_ST_EXPLODE+0, DO_ST_EXPLODE+1, DO_ST_EXPLODE+2,
    DO_ST_EXPLODE+3, DO_ST_EXPLODE+4, DO_ST_EXPLODE+5, DO_ST_EXPLODE+6,
    CARRIED_DOMINO_START+0, CARRIED_DOMINO_START+1, CARRIED_DOMINO_START+2, CARRIED_DOMINO_START+3,
    CARRIED_DOMINO_START+4, CARRIED_DOMINO_START+5, CARRIED_DOMINO_START+6, -1 },
  /* DominoTypeDelay      */ {
    DO_ST_FALLING+0, DO_ST_FALLING+1, DO_ST_FALLING+2, DO_ST_FALLING+3, DO_ST_FALLING+4,
    DO_ST_FALLING+5, DO_ST_FALLING+6, DO_ST_FALLING+7, DO_ST_FALLING+8, DO_ST_FALLING+9,
    DO_ST_FALLING+10, DO_ST_FALLING+11, DO_ST_FALLING+12, DO_ST_FALLING+13, DO_ST_FALLING+14,
    CARRIED_DOMINO_START+0, CARRIED_DOMINO_START+1, CARRIED_DOMINO_START+2, CARRIED_DOMINO_START+3,
    CARRIED_DOMINO_START+4, CARRIED_DOMINO_START+5, CARRIED_DOMINO_START+6, -1 },
  /* DominoTypeTumbler    */ {
    DO_ST_FALLING+0, DO_ST_FALLING+1, DO_ST_FALLING+2, DO_ST_FALLING+3, DO_ST_FALLING+4,
    DO_ST_FALLING+5, DO_ST_FALLING+6, DO_ST_FALLING+7, DO_ST_FALLING+8, DO_ST_FALLING+9,
    DO_ST_FALLING+10, DO_ST_FALLING+11, DO_ST_FALLING+12, DO_ST_FALLING+13, DO_ST_FALLING+14,
    CARRIED_DOMINO_START+0, CARRIED_DOMINO_START+1, CARRIED_DOMINO_START+2, CARRIED_DOMINO_START+3,
    CARRIED_DOMINO_START+4, CARRIED_DOMINO_START+5, CARRIED_DOMINO_START+6, -1 },
  /* DominoTypeBridger    */ {
    DO_ST_FALLING+0, DO_ST_FALLING+1, DO_ST_FALLING+2, DO_ST_FALLING+3, DO_ST_FALLING+4,
    DO_ST_FALLING+5, DO_ST_FALLING+6, DO_ST_FALLING+7, DO_ST_FALLING+8, DO_ST_FALLING+9,
    DO_ST_FALLING+10, DO_ST_FALLING+11, DO_ST_FALLING+12, DO_ST_FALLING+13, DO_ST_FALLING+14,
    CARRIED_DOMINO_START+0, CARRIED_DOMINO_START+1, CARRIED_DOMINO_START+2, CARRIED_DOMINO_START+3,
    CARRIED_DOMINO_START+4, CARRIED_DOMINO_START+5, CARRIED_DOMINO_START+6, -1 },
  /* DominoTypeVanish     */ {
    DO_ST_FALLING+0, DO_ST_FALLING+1, DO_ST_FALLING+2, DO_ST_FALLING+3, DO_ST_FALLING+4,
    DO_ST_FALLING+5, DO_ST_FALLING+6, DO_ST_FALLING+7, DO_ST_FALLING+8, DO_ST_FALLING+9,
    DO_ST_FALLING+10, DO_ST_FALLING+11, DO_ST_FALLING+12, DO_ST_FALLING+13, DO_ST_FALLING+14,
    CARRIED_DOMINO_START+0, CARRIED_DOMINO_START+1, CARRIED_DOMINO_START+2, CARRIED_DOMINO_START+3,
    CARRIED_DOMINO_START+4, CARRIED_DOMINO_START+5, CARRIED_DOMINO_START+6, -1 },
  /* DominoTypeTrigger    */ {
    DO_ST_FALLING+0, DO_ST_FALLING+1, DO_ST_FALLING+2, DO_ST_FALLING+3, DO_ST_FALLING+4,
    DO_ST_FALLING+5, DO_ST_FALLING+6, DO_ST_FALLING+7, DO_ST_FALLING+8, DO_ST_FALLING+9,
    DO_ST_FALLING+10, DO_ST_FALLING+11, DO_ST_FALLING+12, DO_ST_FALLING+13, DO_ST_FALLING+14,
    -1 },
  /* DominoTypeAscender   */ {
    DO_ST_FALLING+0, DO_ST_FALLING+1, DO_ST_FALLING+2, DO_ST_FALLING+3, DO_ST_FALLING+4,
    DO_ST_FALLING+5, DO_ST_FALLING+6, DO_ST_FALLING+7, DO_ST_FALLING+8, DO_ST_FALLING+9,
    DO_ST_FALLING+10, DO_ST_FALLING+11, DO_ST_FALLING+12, DO_ST_FALLING+13, DO_ST_FALLING+14,
    CARRIED_DOMINO_START+0, CARRIED_DOMINO_START+1, CARRIED_DOMINO_START+2, CARRIED_DOMINO_START+3,
    CARRIED_DOMINO_START+4, CARRIED_DOMINO_START+5, CARRIED_DOMINO_START+6, -1 },
  /* DominoTypeConnectedA */ {
    DO_ST_FALLING+0, DO_ST_FALLING+1, DO_ST_FALLING+2, DO_ST_FALLING+3, DO_ST_FALLING+4,
    DO_ST_FALLING+5, DO_ST_FALLING+6, DO_ST_FALLING+7, DO_ST_FALLING+8, DO_ST_FALLING+9,
    DO_ST_FALLING+10, DO_ST_FALLING+11, DO_ST_FALLING+12, DO_ST_FALLING+13, DO_ST_FALLING+14,
    CARRIED_DOMINO_START+0, CARRIED_DOMINO_START+1, CARRIED_DOMINO_START+2, CARRIED_DOMINO_START+3,
    CARRIED_DOMINO_START+4, CARRIED_DOMINO_START+5, CARRIED_DOMINO_START+6, -1 },
  /* DominoTypeConnectedB */ {
    DO_ST_FALLING+0, DO_ST_FALLING+1, DO_ST_FALLING+2, DO_ST_FALLING+3, DO_ST_FALLING+4,
    DO_ST_FALLING+5, DO_ST_FALLING+6, DO_ST_FALLING+7, DO_ST_FALLING+8, DO_ST_FALLING+9,
    DO_ST_FALLING+10, DO_ST_FALLING+11, DO_ST_FALLING+12, DO_ST_FALLING+13, DO_ST_FALLING+14,
    CARRIED_DOMINO_START+0, CARRIED_DOMINO_START+1, CARRIED_DOMINO_START+2, CARRIED_DOMINO_START+3,
    CARRIED_DOMINO_START+4, CARRIED_DOMINO_START+5, CARRIED_DOMINO_START+6, -1 },
  /* DominoTypeCounter1   */ {
    DO_ST_FALLING+0, DO_ST_FALLING+1, DO_ST_FALLING+2, DO_ST_FALLING+3, DO_ST_FALLING+4,
    DO_ST_FALLING+5, DO_ST_FALLING+6, DO_ST_FALLING+7, DO_ST_FALLING+8, DO_ST_FALLING+9,
    DO_ST_FALLING+10, DO_ST_FALLING+11, DO_ST_FALLING+12, DO_ST_FALLING+13, DO_ST_FALLING+14,
    CARRIED_DOMINO_START+0, CARRIED_DOMINO_START+1, CARRIED_DOMINO_START+2, CARRIED_DOMINO_START+3,
    CARRIED_DOMINO_START+4, CARRIED_DOMINO_START+5, CARRIED_DOMINO_START+6, -1 },
  /* DominoTypeCounter2   */ {
    DO_ST_FALLING+0, DO_ST_FALLING+1, DO_ST_FALLING+2, DO_ST_FALLING+3, DO_ST_FALLING+4,
    DO_ST_FALLING+5, DO_ST_FALLING+6, DO_ST_FALLING+7, DO_ST_FALLING+8, DO_ST_FALLING+9,
    DO_ST_FALLING+10, DO_ST_FALLING+11, DO_ST_FALLING+12, DO_ST_FALLING+13, DO_ST_FALLING+14,
    CARRIED_DOMINO_START+0, CARRIED_DOMINO_START+1, CARRIED_DOMINO_START+2, CARRIED_DOMINO_START+3,
    CARRIED_DOMINO_START+4, CARRIED_DOMINO_START+5, CARRIED_DOMINO_START+6, -1 },
  /* DominoTypeCounter3   */ {
    DO_ST_FALLING+0, DO_ST_FALLING+1, DO_ST_FALLING+2, DO_ST_FALLING+3, DO_ST_FALLING+4,
    DO_ST_FALLING+5, DO_ST_FALLING+6, DO_ST_FALLING+7, DO_ST_FALLING+8, DO_ST_FALLING+9,
    DO_ST_FALLING+10, DO_ST_FALLING+11, DO_ST_FALLING+12, DO_ST_FALLING+13, DO_ST_FALLING+14,
    CARRIED_DOMINO_START+0, CARRIED_DOMINO_START+1, CARRIED_DOMINO_START+2, CARRIED_DOMINO_START+3,
    CARRIED_DOMINO_START+4, CARRIED_DOMINO_START+5, CARRIED_DOMINO_START+6, -1 },
  /* DominoTypeCrash0     */ {
    DO_ST_CRASH+0, DO_ST_CRASH+1, DO_ST_CRASH+2, DO_ST_CRASH+3, DO_ST_CRASH+4, DO_ST_CRASH+5, -1 },
  /* DominoTypeCrash1     */ {
    DO_ST_CRASH+0, DO_ST_CRASH+1, DO_ST_CRASH+2, DO_ST_CRASH+3, DO_ST_CRASH+4, DO_ST_CRASH+5, -1 },
  /* DominoTypeCrash2     */ {
    DO_ST_CRASH+0, DO_ST_CRASH+1, DO_ST_CRASH+2, DO_ST_CRASH+3, DO_ST_CRASH+4, DO_ST_CRASH+5, -1 },
  /* DominoTypeCrash3     */ {
    DO_ST_CRASH+0, DO_ST_CRASH+1, DO_ST_CRASH+2, DO_ST_CRASH+3, DO_ST_CRASH+4, DO_ST_CRASH+5, -1 },
  /* DominoTypeCrash4     */ {
    DO_ST_CRASH+0, DO_ST_CRASH+1, DO_ST_CRASH+2, DO_ST_CRASH+3, DO_ST_CRASH+4, DO_ST_CRASH+5, -1 },
  /* DominoTypeCrash5     */ {
    DO_ST_CRASH+0, DO_ST_CRASH+1, DO_ST_CRASH+2, DO_ST_CRASH+3, DO_ST_CRASH+4, DO_ST_CRASH+5, -1 },
};

#define antDisplace (6*3)
#define dominoYStart (3*4)
static int convertDominoY(int y) { return 3*y; }
#define splitterY 34


// The following defines are used to index into the fg element array
#define FG_PLAT_IDX 0
#define FG_LADDER_IDX 16  // 3 images, ladder top, section, bottom, 12, 24 and 12 pixel high
#define FG_DOORSTART 19   // 4 doors each composed out of 2 halves, first top then bottom.. makes 8 images

graphicsN_c::graphicsN_c(const std::string & path) : dataPath(path) {
  background = 0;
  ant = 0;
  level = 0;
  tutorial = 0;

#ifdef DEBUG
  grid = false;
#endif

  dominos.resize(DominoNumber);

  for (unsigned int i = 0; i < DominoNumber; i++) {
    dominos[i].resize(80);  // TODO right number
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

    uint16_t domino = 0;
    uint16_t dominoIndex = 0;

    while (true)
    {
      while (domino < DominoNumber && dominoImages[domino][dominoIndex] == -1)
      {
        domino++;
        dominoIndex = 0;
      }

      if (domino >= DominoNumber) break;

      if (dominoImages[domino][dominoIndex] == -2)
      {
        png.skipLines(60);
      }
      else
      {
        surface_c * v = new surface_c(png.getWidth(), 58);
        png.getPart(*v);

        assert(dominos[domino][dominoImages[domino][dominoIndex]] == 0);
        dominos[domino][dominoImages[domino][dominoIndex]] = v;
        png.skipLines(2);
      }

      dominoIndex++;
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
    pngLoader_c png(dataPath+"/data/box.png");

    if (png.getWidth() != 40*3 || png.getHeight() != 48*3)
    {
      std::cout << " oops box image hasn't the right dimensions\n";
      return;
    }

    surface_c v(png.getWidth(), 48, false);

    for (int i = 0; i < 3; i++) {

      png.getPart(v);

      for (int x = 0; x < 3; x++) {

        surface_c * w = new surface_c(40, 48);

        w->copy(v, x*40, 0, 40, 48);

        boxBlocks.push_back(w);
      }
    }
  }
}


graphicsN_c::~graphicsN_c(void) {

  for (unsigned int i = 0; i < bgTiles.size(); i++)
    for (unsigned int j = 0; j < bgTiles[i].size(); j++)
      delete bgTiles[i][j];

  for (unsigned int i = 0; i < fgTiles.size(); i++)
    for (unsigned int j = 0; j < fgTiles[i].size(); j++)
      delete fgTiles[i][j];

  for (unsigned int i = 0; i < dominos.size(); i++)
    for (unsigned int j = 0; j < dominos[i].size(); j++)
      if (dominos[i][j])
        delete dominos[i][j];

  for (unsigned int i = 0; i < antImages.size(); i++)
    for (unsigned int j = 0; j < antImages[i].size(); j++)
      if (antImages[i][j].free)
        delete antImages[i][j].v;

  for (unsigned int j = 0; j < boxBlocks.size(); j++)
    delete boxBlocks[j];

  if (tutorial) delete tutorial;
}

void graphicsN_c::addAnt(unsigned int anim, unsigned int img, signed char yOffset, surface_c * v, bool free) {

  antSprite s;
  s.v = v;
  s.ofs = yOffset;
  s.free = free;

  antImages[anim][img] = s;
}

void graphicsN_c::getAnimation(AntAnimationState anim, pngLoader_c * png) {

  static int antOffsetPos = 0;

  for (unsigned int j = 0; j < ant_c::getAntImages(anim); j++) {

    surface_c * v = new surface_c(png->getWidth(), 75);
    png->getPart(*v);
    v->fillRect(0, 0, 1, 75, 0, 0, 0, SDL_ALPHA_TRANSPARENT);

    addAnt(anim, j, antOffsets[antOffsetPos], v);
    antOffsetPos++;

  }

  // TODO remove those images from the ant image
  if (   anim == AntAnimFalling
     )
  {
    png->skipLines(75*2);
  }
}


static int16_t moveOffsets[22][16*3] =
{
  // This array defines for the ant animations where and which image of a domino to draw
  // This is required for the animations where the ant carries a domino, because in those
  // animations the domino is not drawn with the level
  // the table contains entrys for each ant animation where the ant carries
  // a domino the following information:
  // - an x and y offset to add to the ann position
  // - the domino image to draw
  {    // AntAnimCarryLeft,        6
    -18, -6, CARRIED_DOMINO_START+0,
    -20, -6, CARRIED_DOMINO_START+0,
    -28, -6, CARRIED_DOMINO_START+0,
    -35, -6, CARRIED_DOMINO_START+0,
    -40, -6, CARRIED_DOMINO_START+0,
    -50, -6, CARRIED_DOMINO_START+0,
  }, { // AntAnimCarryRight,       6
     13, -6, CARRIED_DOMINO_START+1,
     15, -6, CARRIED_DOMINO_START+1,
     23, -6, CARRIED_DOMINO_START+1,
     30, -6, CARRIED_DOMINO_START+1,
     35, -6, CARRIED_DOMINO_START+1,
     45, -6, CARRIED_DOMINO_START+1,
  }, { // AntAnimCarryUpLeft,      6
    -10, -6, CARRIED_DOMINO_START+0,
    -15,-12, CARRIED_DOMINO_START+0,
    -18,-18, CARRIED_DOMINO_START+0,
    -28,-12, CARRIED_DOMINO_START+0,
    -40, -9, CARRIED_DOMINO_START+0,
    -50, -6, CARRIED_DOMINO_START+0,
  }, { // AntAnimCarryUpRight,     6
      5, -6, CARRIED_DOMINO_START+1,
     10,-12, CARRIED_DOMINO_START+1,
     13,-18, CARRIED_DOMINO_START+1,
     23,-12, CARRIED_DOMINO_START+1,
     35, -9, CARRIED_DOMINO_START+1,
     45, -6, CARRIED_DOMINO_START+1,
  }, { // AntAnimCarryDownLeft,    6
    -30, -6, CARRIED_DOMINO_START+0,
    -40, -9, CARRIED_DOMINO_START+0,
    -48,  0, CARRIED_DOMINO_START+0,
    -48,  3, CARRIED_DOMINO_START+0,
    -48,  3, CARRIED_DOMINO_START+0,
    -50,  0, CARRIED_DOMINO_START+0,
  }, { // AntAnimCarryDownRight,   6
     23, -6, CARRIED_DOMINO_START+1,
     33, -9, CARRIED_DOMINO_START+1,
     40,  0, CARRIED_DOMINO_START+1,
     40,  3, CARRIED_DOMINO_START+1,
     40,  3, CARRIED_DOMINO_START+1,
     43,  0, CARRIED_DOMINO_START+1,
  }, { // AntAnimCarryLadder1,     8
    -20, -6, CARRIED_DOMINO_START+2,
    -20, -3, CARRIED_DOMINO_START+2,
    -18, -6, CARRIED_DOMINO_START+2,
    -18, -3, CARRIED_DOMINO_START+2,
    -20, -6, CARRIED_DOMINO_START+2,
    -20, -3, CARRIED_DOMINO_START+2,
    -18, -6, CARRIED_DOMINO_START+2,
    -18, -3, CARRIED_DOMINO_START+2,
  }, { // AntAnimCarryLadder2,     8
    -20, -6, CARRIED_DOMINO_START+2,
    -20, -3, CARRIED_DOMINO_START+2,
    -18, -6, CARRIED_DOMINO_START+2,
    -18, -3, CARRIED_DOMINO_START+2,
    -20, -6, CARRIED_DOMINO_START+2,
    -20, -3, CARRIED_DOMINO_START+2,
    -18, -6, CARRIED_DOMINO_START+2,
    -18, -3, CARRIED_DOMINO_START+2,
  }, { // AntAnimCarryLadder3,     8
    -18, -3, CARRIED_DOMINO_START+2,
    -18, -6, CARRIED_DOMINO_START+2,
    -20, -3, CARRIED_DOMINO_START+2,
    -20, -6, CARRIED_DOMINO_START+2,
    -18, -3, CARRIED_DOMINO_START+2,
    -18, -6, CARRIED_DOMINO_START+2,
    -20, -3, CARRIED_DOMINO_START+2,
    -20, -6, CARRIED_DOMINO_START+2,
  }, { // AntAnimCarryLadder4,     8
    -18, -3, CARRIED_DOMINO_START+2,
    -18, -6, CARRIED_DOMINO_START+2,
    -20, -3, CARRIED_DOMINO_START+2,
    -20, -6, CARRIED_DOMINO_START+2,
    -18, -3, CARRIED_DOMINO_START+2,
    -18, -6, CARRIED_DOMINO_START+2,
    -20, -3, CARRIED_DOMINO_START+2,
    -20, -6, CARRIED_DOMINO_START+2,
  }, { // AntAnimCarryStopLeft,    1
    -13, -6, CARRIED_DOMINO_START+0,
  }, { // AntAnimCarryStopRight,   1
     13, -6, CARRIED_DOMINO_START+1,
  }, { // AntAnimPullOutLeft       15
      0, -6, DO_ST_UPRIGHT,
      0, -6, DO_ST_UPRIGHT,
      0, -6, DO_ST_UPRIGHT,
     -3, -3, DO_ST_UPRIGHT,
     -5,  0, DO_ST_UPRIGHT,
     -5,  0, DO_ST_UPRIGHT,
     -5,  0, DO_ST_UPRIGHT,
     -5,  0, DO_ST_UPRIGHT,
     -5,  0, DO_ST_UPRIGHT,
     -5,  0, DO_ST_UPRIGHT,
     -5,  0, DO_ST_UPRIGHT,
     -5, -3, DO_ST_UPRIGHT+2,
      0, -6, CARRIED_DOMINO_START+0,
     -3, -6, CARRIED_DOMINO_START+0,
    -10, -6, CARRIED_DOMINO_START+0,
  }, { // AntAnimPullOutRight      15
      0, -6, DO_ST_UPRIGHT,
      0, -6, DO_ST_UPRIGHT,
      0, -6, DO_ST_UPRIGHT,
     -3, -3, DO_ST_UPRIGHT,
     -5,  0, DO_ST_UPRIGHT,
     -5,  0, DO_ST_UPRIGHT,
     -5,  0, DO_ST_UPRIGHT,
     -5,  0, DO_ST_UPRIGHT,
     -5,  0, DO_ST_UPRIGHT,
     -5,  0, DO_ST_UPRIGHT,
     -5,  0, DO_ST_UPRIGHT,
     -5, -3, DO_ST_UPRIGHT-2,
     -8, -6, CARRIED_DOMINO_START+1,
     -5, -6, CARRIED_DOMINO_START+1,
     -3, -6, CARRIED_DOMINO_START+1,
  }, {  // AntAnimPushInLeft,      16
    -18, -6, CARRIED_DOMINO_START+0,
    -20, -6, CARRIED_DOMINO_START+0,
    -28, -6, CARRIED_DOMINO_START+0,
    -35, -6, CARRIED_DOMINO_START+0,
    -43, -6, CARRIED_DOMINO_START+0,
    -43,  0, CARRIED_DOMINO_START+0,
    -45,  0, DO_ST_UPRIGHT+2,
    -45,  0, DO_ST_UPRIGHT,
    -45,  0, DO_ST_UPRIGHT,
    -45,  0, DO_ST_UPRIGHT,
    -45,  0, DO_ST_UPRIGHT,
    -45,  0, DO_ST_UPRIGHT,
    -43, -3, DO_ST_UPRIGHT,
    -40, -6, DO_ST_UPRIGHT,
    -40, -6, DO_ST_UPRIGHT,
    -40, -6, DO_ST_UPRIGHT,
  }, {  // AntAnimPushInRight,     16
     13, -6, CARRIED_DOMINO_START+1,
     15, -6, CARRIED_DOMINO_START+1,
     25, -3, CARRIED_DOMINO_START+1,
     28, -6, CARRIED_DOMINO_START+1,
     35, -6, CARRIED_DOMINO_START+1,
     35,  0, CARRIED_DOMINO_START+1,
     35,  0, DO_ST_UPRIGHT-2,
     35,  0, DO_ST_UPRIGHT,
     35,  0, DO_ST_UPRIGHT,
     35,  0, DO_ST_UPRIGHT,
     35,  0, DO_ST_UPRIGHT,
     35,  0, DO_ST_UPRIGHT,
     38, -3, DO_ST_UPRIGHT,
     40, -6, DO_ST_UPRIGHT,
     40, -6, DO_ST_UPRIGHT,
     40, -6, DO_ST_UPRIGHT,
  }, {  //AntAnimXXX1              2
      0, -6, CARRIED_DOMINO_START+3,
      0, -6, CARRIED_DOMINO_START+4,
  }, { //AntAnimXXX2               2
     -8, -6, CARRIED_DOMINO_START+5,
     -8, -6, CARRIED_DOMINO_START+6,
  }, { //AntAnimXXX3               2
      0, -6, CARRIED_DOMINO_START+4,
      0, -6, CARRIED_DOMINO_START+3,
  }, { //AntAnimXXX4               2
     -8, -6, CARRIED_DOMINO_START+6,
     -8, -6, CARRIED_DOMINO_START+5,
  }, { //AntAnimLoosingDominoRight 13
     40, -9, DO_ST_UPRIGHT,
     40, -9, DO_ST_UPRIGHT,
     40, -9, DO_ST_UPRIGHT,
     40, -9, DO_ST_UPRIGHT, // after this image the domino is put down and no longer in the hand of the ant
  }, { //AntAnimLoosingDominoLeft  17
    -18, -6, CARRIED_DOMINO_START+0,
    -20, -6, CARRIED_DOMINO_START+0,
    -28, -6, CARRIED_DOMINO_START+0,
    -35, -6, CARRIED_DOMINO_START+0,
      0, -9, DO_ST_UPRIGHT,
      0, -9, DO_ST_UPRIGHT,
      0, -9, DO_ST_UPRIGHT,
      0, -9, DO_ST_UPRIGHT, // after this image the domino is put down and no longer in the hand of the ant
  }
};

signed int getMoveOffsetX(unsigned int animation, unsigned int image) { return moveOffsets[animation][3*image+0]; }
signed int getMoveOffsetY(unsigned int animation, unsigned int image) { return moveOffsets[animation][3*image+1]; }
signed int getMoveImage(unsigned int animation, unsigned int image) { return moveOffsets[animation][3*image+2]; }

// this function calculates the indes to use for the table above
static int16_t getCarryAnimationIndex(AntAnimationState a)
{
  switch (a)
  {
    case AntAnimCarryLeft         : return 0;
    case AntAnimCarryRight        : return 1;
    case AntAnimCarryUpLeft       : return 2;
    case AntAnimCarryUpRight      : return 3;
    case AntAnimCarryDownLeft     : return 4;
    case AntAnimCarryDownRight    : return 5;
    case AntAnimCarryLadder1      : return 6;
    case AntAnimCarryLadder2      : return 7;
    case AntAnimCarryLadder3      : return 8;
    case AntAnimCarryLadder4      : return 9;
    case AntAnimCarryStopLeft     : return 10;
    case AntAnimCarryStopRight    : return 11;
    case AntAnimPullOutLeft       : return 12;
    case AntAnimPullOutRight      : return 13;
    case AntAnimPushInLeft        : return 14;
    case AntAnimPushInRight       : return 15;
    case AntAnimXXX1              : return 16;
    case AntAnimXXX2              : return 17;
    case AntAnimXXX3              : return 18;
    case AntAnimXXX4              : return 19;
    case AntAnimLoosingDominoRight: return 20;
    case AntAnimLoosingDominoLeft : return 21;
    default                       : return -1;
  }
}

static bool antOnLadder(AntAnimationState a)
{
  switch (a)
  {
    case AntAnimLadder1:
    case AntAnimLadder2:
    case AntAnimLadder3:
    case AntAnimLadder4:
    case AntAnimCarryLadder1:
    case AntAnimCarryLadder2:
    case AntAnimCarryLadder3:
    case AntAnimCarryLadder4:
    case AntAnimXXX1:
    case AntAnimXXX2:
    case AntAnimXXX3:
    case AntAnimXXX4:
      return true;
    default:
      return false;
  }
}

void graphicsN_c::drawLadders(bool before)
{
  // the problem with the ladders is that we sometimes need to
  // draw them behind and sometimes before the ant, depending on its
  // animation... when on the ladder the ladder needs to be behind the ant
  // otherwise in front
  // that is why this function is called once bafore and once after drawing the ant
  // the argument is true, when called before the ant drawing
  if (before == antOnLadder(ant ? ant->getAnimation() : AntAnimStop))
    for (unsigned int y = 0; y < level->levelY(); y++)
      for (unsigned int x = 0; x < level->levelX(); x++)
        if (dirty.isDirty(x, y))
        {
          if (level->getLadder(x, y))
            target->blitBlock(*fgTiles[curTheme][FG_LADDER_IDX+1], x*blockX(), y*blockY()/2);
          else
          {
            if (y > 0 && level->getLadder(x, y-1))
              target->blitBlock(*fgTiles[curTheme][FG_LADDER_IDX+2], x*blockX(), y*blockY()/2);
            if (y+1 < level->levelY() && level->getLadder(x, y+1))
              target->blitBlock(*fgTiles[curTheme][FG_LADDER_IDX+0], x*blockX(), y*blockY()/2+blockY()/4);
          }
        }
}


void graphicsN_c::drawAnt(void)
{
  if (ant && ant->isVisible())
  {
    if (ant->getCarriedDomino() != 0)
    {
      int16_t a = getCarryAnimationIndex(ant->getAnimation());

      if (a >= 0)
      {
        int x = (ant->getBlockX()-2)*blockX();
        int y = (ant->getBlockY())*blockY()/2+antImages[ant->getAnimation()][ant->getAnimationImage()].ofs+antDisplace;

        y += getMoveOffsetY(a, ant->getAnimationImage());
        x += getMoveOffsetX(a, ant->getAnimationImage());

        int img = getMoveImage(a, ant->getAnimationImage());

        assert(dominos[ant->getCarriedDomino()][img] != 0);
        target->blit(*dominos[ant->getCarriedDomino()][img], x, y);
      }
    }

    if (antImages[ant->getAnimation()][ant->getAnimationImage()].v)
    {
      target->blit(*antImages[ant->getAnimation()][ant->getAnimationImage()].v, ant->getBlockX()*blockX()-45,
          (ant->getBlockY())*blockY()/2+antImages[ant->getAnimation()][ant->getAnimationImage()].ofs+antDisplace+3);
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

  surface_c v(png.getWidth(), blockY(), false);

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
          if (0 <= i && i <= 3)
          {
            // load a platform image, each platform image is plit into 4 parts each blockX/2 x blockY/2 in size
            surface_c * w1 = new surface_c(blockX()/2, blockY()/2);
            surface_c * w2 = new surface_c(blockX()/2, blockY()/2);
            surface_c * w3 = new surface_c(blockX()/2, blockY()/2);
            surface_c * w4 = new surface_c(blockX()/2, blockY()/2);

            w1->copy(v, x*blockX()           ,          0, blockX()/2, blockY()/2);
            w2->copy(v, x*blockX()+blockX()/2,          0, blockX()/2, blockY()/2);
            w3->copy(v, x*blockX()           , blockY()/2, blockX()/2, blockY()/2);
            w4->copy(v, x*blockX()+blockX()/2, blockY()/2, blockX()/2, blockY()/2);

            if (FG_PLAT_IDX+16 >= fgTiles[curTheme].size())
              fgTiles[curTheme].resize(FG_PLAT_IDX+16);

            fgTiles[curTheme][FG_PLAT_IDX + 4*(i-0) + 0] = w1;
            fgTiles[curTheme][FG_PLAT_IDX + 4*(i-0) + 1] = w2;
            fgTiles[curTheme][FG_PLAT_IDX + 4*(i-0) + 2] = w3;
            fgTiles[curTheme][FG_PLAT_IDX + 4*(i-0) + 3] = w4;
          }
          else if (i == 4)
          {
            // load the ladder image, the ladder image is divided into 3 parts,
            // the top 12 pixel, middle 24 pixels and the bottom 12 pixels
            //
            surface_c * w1 = new surface_c(blockX(), blockY()/4);
            surface_c * w2 = new surface_c(blockX(), blockY()/2);
            surface_c * w3 = new surface_c(blockX(), blockY()/4);

            w1->copy(v, x*blockX(),            0, blockX(), blockY()/4);
            w2->copy(v, x*blockX(),   blockY()/4, blockX(), blockY()/2);
            w3->copy(v, x*blockX(), 3*blockY()/4, blockX(), blockY()/4);

            if (FG_LADDER_IDX+3 >= fgTiles[curTheme].size())
              fgTiles[curTheme].resize(FG_LADDER_IDX+3);

            fgTiles[curTheme][FG_LADDER_IDX + 0] = w1;
            fgTiles[curTheme][FG_LADDER_IDX + 1] = w2;
            fgTiles[curTheme][FG_LADDER_IDX + 2] = w3;
          }
          else if (5 <= i && i <= 8)
          {
            // load door image, the door image is simply split
            // into the 2 haves each 24 pixel high

            surface_c * w1 = new surface_c(blockX(), blockY()/2);
            surface_c * w2 = new surface_c(blockX(), blockY()/2);

            w1->copy(v, x*blockX(),          0, blockX(), blockY()/2);
            w2->copy(v, x*blockX(), blockY()/2, blockX(), blockY()/2);

            if (FG_DOORSTART+9 >= fgTiles[curTheme].size())
              fgTiles[curTheme].resize(FG_DOORSTART+9);

            fgTiles[curTheme][FG_DOORSTART + 2*(i-5) +0] = w1;
            fgTiles[curTheme][FG_DOORSTART + 2*(i-5) +1] = w2;
          }
        }
      }
    }

    // get possible background tiles in the current line
    for (unsigned int i = 0; i < backSize; i++) {
      if (l.getNumberArray("background", 2*i+2) == yPos) {

        unsigned int x = (unsigned int)l.getNumberArray("background", 2*i+1);

        if (x < xBlocks)
        {
          surface_c * w1 = new surface_c(blockX(), blockY()/2);
          surface_c * w2 = new surface_c(blockX(), blockY()/2);

          w1->copy(v, x*blockX(),          0, blockX(), blockY()/2);
          w2->copy(v, x*blockX(), blockY()/2, blockX(), blockY()/2);

          if (2*i+1 >= bgTiles[curTheme].size())
            bgTiles[curTheme].resize(2*i+2);

          bgTiles[curTheme][2*i  ] = w1;
          bgTiles[curTheme][2*i+1] = w2;
        }
      }
    }

    yPos++;
  }
}

void graphicsN_c::setPaintData(const levelData_c * l, const ant_c * a, surface_c * t)
{
  level = l;
  ant = a;
  target = t;

  if (!level) return;

  setTheme(level->getTheme());

  assert(l->levelX() == 20);
  assert(l->levelY() == 25);

  dirtybg.markAllDirty();
  dirty.markAllDirty();

  antX = antY = 200;
  antAnim = 1000;
  antImage = 1000;


  Min = Sec = -1;

  if (!background) background = new surface_c(t->getIdentical());

  if (tutorial) delete tutorial;
  tutorial = 0;
}

void graphicsN_c::getPlatformImage(size_t x, size_t y, uint16_t out[])
{
  // so what's the idea behind the whole level platform drawing mess???
  // The 40x48 pixel tiles are divided into 4 smaller tiles, each 20x24 pixels
  // Of these images we can stack up to 2 on top of one another to create the
  // actual image
  //
  // This function will return in out those up to tiles for the 2 tiles side
  // by side for the given 40x24 tile in x;y
  //
  // the 4 big tiles given in the theme file are names b0 - b3, the small
  // tiles are s0-s3 in b0, s4-s7 in b1, and so on
  //
  // b0 and b1 contain the image for a start and an end of a platform
  // the platform image is usualy 2 level tiles high. The tile above the
  // platform contains some kind of planks, while the image at the platform
  // position contains the front of the platform
  //
  // depending on whether the neighbor platforms are occupied the output is
  // assembled out of the 4 columns os b0 and b1.
  //
  // When there is a platform directly above another platform the plans of
  // the lower platform are overlaid with the front of the upper layer.
  // The front of this upper layer may have a different appearance and this
  // is put into the top row ob b2 and b3.
  //
  // The lower row of b2 contains 2 special cases. Have a look at the theme
  // file for the aztecs in those images it is pretty clear what those
  // images stand for (in connection with the conditions below

  out[0] = out[1] = out[2] = out[3] = 99;

  if (y+1 < level->levelY() && level->getPlatform(x, y+1))
  {
    if (x > 0 && level->getPlatform(x-1, y+1))                       out[0] = FG_PLAT_IDX+4;
    else if (level->getPlatform(x-1, y) && level->getPlatform(x, y)) out[0] = FG_PLAT_IDX+10;
    else                                                             out[0] = FG_PLAT_IDX+0;
    if (x+1 < level->levelX() && level->getPlatform(x+1, y+1))       out[2] = FG_PLAT_IDX+1;
    else                                                             out[2] = FG_PLAT_IDX+5;

    if (level->getPlatform(x, y))
    {
      if (x > 0 && level->getPlatform(x-1, y))                       out[1] = FG_PLAT_IDX+12;
      else                                                           out[1] = FG_PLAT_IDX+8;
      if (x+1 >= level->levelX() || !level->getPlatform(x+1, y))     out[3] = FG_PLAT_IDX+13;
      else if (level->getPlatform(x+1, y+1))                         out[3] = FG_PLAT_IDX+9;
      else                                                           out[3] = FG_PLAT_IDX+11;
    }
  }
  else if (level->getPlatform(x, y))
  {
    if (x > 0 && level->getPlatform(x-1, y))                         out[0] = FG_PLAT_IDX+6;
    else                                                             out[0] = FG_PLAT_IDX+2;
    if (x+1 < level->levelX() && level->getPlatform(x+1, y))         out[2] = FG_PLAT_IDX+3;
    else                                                             out[2] = FG_PLAT_IDX+7;
  }
}

void graphicsN_c::drawDomino(uint16_t x, uint16_t y)
{
  static int XposOffset[] = { -40, -40, -31, -23, -10, -4, -2, 0, 2,  5, 10, 23,  31,  40,  40};
  static int YposOffset[] = { -30, -21,  -9,  -4,   0,  0,  0, 0, 0,  0,  0, -4,  -9, -21, -30};
  static int StoneImageOffset[] = {
    DO_ST_FALLING+14, DO_ST_FALLING+13, DO_ST_FALLING+12, DO_ST_FALLING+11, DO_ST_FALLING+10,
    DO_ST_FALLING+9, DO_ST_FALLING+8, DO_ST_FALLING+7, DO_ST_FALLING+6, DO_ST_FALLING+5,
    DO_ST_FALLING+4, DO_ST_FALLING+3, DO_ST_FALLING+2, DO_ST_FALLING+1, DO_ST_FALLING+0 };

  DominoType dt = level->getDominoType(x, y);

  if (dt == DominoTypeEmpty) return;

  int SpriteYPos = dominoYStart + y*blockY()/2;
  int SpriteXPos = -2*blockX() + x*blockX();

  int ds = level->getDominoState(x, y);
  int de = level->getDominoExtra(x, y);
  int dx;
  int dy;
  int dc = 0;

  switch (ds)
  {
    case DO_ST_ASCENDER+ 0:
    case DO_ST_ASCENDER+ 1:
    case DO_ST_ASCENDER+ 2:
    case DO_ST_ASCENDER+ 3:
    case DO_ST_ASCENDER+ 4:
    case DO_ST_ASCENDER+ 5:
    case DO_ST_ASCENDER+ 6:
    case DO_ST_ASCENDER+ 7:
    case DO_ST_ASCENDER+ 8:
    case DO_ST_ASCENDER+ 9:
    case DO_ST_ASCENDER+10:
    case DO_ST_ASCENDER+11:
    case DO_ST_ASCENDER+12:
    case DO_ST_ASCENDER+13:
    case DO_ST_ASCENDER+14:

      // clipping of the ascender...
      // this is the normal clipping case, where a platform get's near
      // enough to shave off the top of the image.
      if (level->getPlatform(x, y-3))
        dc = (y-2)*blockY()/2-10;

      // when the ascender has rested against the wall is is transformed 2
      // blocks higher, this is the clipping case for that position
      if (((ds == DO_ST_ASCENDER) || (ds == DO_ST_ASCENDER_E)) && level->getPlatform(x, y-1))
        dc = y*blockY()/2-10;

      ds = StoneImageOffset[level->getDominoState(x, y)-DO_ST_ASCENDER];
      dx = SpriteXPos+XposOffset[level->getDominoState(x, y)-DO_ST_ASCENDER];
      dy = SpriteYPos+YposOffset[level->getDominoState(x, y)-DO_ST_ASCENDER]+convertDominoY(level->getDominoYOffset(x, y));

      break;

    case DO_ST_SPLIT+1:
      // case, where the splitter just opened up
      // here we need to draw the splitting stone first
      if (de != 0)
      {
        assert(dominos[de][de>=DominoTypeCrash0?DO_ST_CRASH:DO_ST_UPRIGHT]);
        target->blit(*dominos[de][de>=DominoTypeCrash0?DO_ST_CRASH:DO_ST_UPRIGHT], SpriteXPos, SpriteYPos-splitterY);
      }
      // fall through intentionally to paint actual domino

    default:
      dx = SpriteXPos;
      dy = SpriteYPos+convertDominoY(level->getDominoYOffset(x, y));
      break;
  }

  assert(dominos[dt][ds] != 0);
  target->blit(*dominos[dt][ds], dx, dy, dc);
}

const surface_c * graphicsN_c::getHelpDominoImage(unsigned int domino) {
  assert(dominos[domino][DO_ST_UPRIGHT] != 0);
  return dominos[domino][DO_ST_UPRIGHT];
}

void graphicsN_c::drawDominos(void)
{

  // update background
  for (unsigned int y = 0; y < level->levelY(); y++)
  {
    for (unsigned int x = 0; x < level->levelX(); x++)
    {
      // when the current block is dirty, recreate it
      if (dirtybg.isDirty(x, y))
      {
        for (unsigned char b = 0; b < level->getNumBgLayer(); b++)
          background->blitBlock(*(bgTiles[curTheme][level->getBg(x, y, b)]), x*blockX(), y*blockY()/2);

        // blit the doors
        if (x == level->getEntryX() && y+2 == level->getEntryY())
          background->blitBlock(*fgTiles[curTheme][FG_DOORSTART+0+2*level->getEntryState()], x*blockX(), y*blockY()/2);
        if (x == level->getEntryX() && y+1 == level->getEntryY())
          background->blitBlock(*fgTiles[curTheme][FG_DOORSTART+1+2*level->getEntryState()], x*blockX(), y*blockY()/2);

        if (x == level->getExitX() && y+2 == level->getExitY())
          background->blitBlock(*fgTiles[curTheme][FG_DOORSTART+0+2*level->getExitState()], x*blockX(), y*blockY()/2);
        if (x == level->getExitX() && y+1 == level->getExitY())
          background->blitBlock(*fgTiles[curTheme][FG_DOORSTART+1+2*level->getExitState()], x*blockX(), y*blockY()/2);

        // blit the platform images
        {
          uint16_t s[4];

          getPlatformImage(x, y, s);

          if (s[0] < fgTiles[curTheme].size())
            background->blitBlock(*fgTiles[curTheme][s[0]], x*blockX(), y*blockY()/2);
          if (s[1] < fgTiles[curTheme].size())
            background->blitBlock(*fgTiles[curTheme][s[1]], x*blockX(), y*blockY()/2);

          if (s[2] < fgTiles[curTheme].size())
            background->blitBlock(*fgTiles[curTheme][s[2]], x*blockX()+blockX()/2, y*blockY()/2);
          if (s[3] < fgTiles[curTheme].size())
            background->blitBlock(*fgTiles[curTheme][s[3]], x*blockX()+blockX()/2, y*blockY()/2);
        }

        background->gradient(blockX()*x, blockY()/2*y, blockX(), blockY()/2);
      }
    }
  }
  dirtybg.clearDirty();

  // copy background, where necessary
  for (unsigned int y = 0; y < level->levelY(); y++)
    for (unsigned int x = 0; x < level->levelX(); x++)
      if (dirty.isDirty(x, y))
        target->copy(*background, x*blockX(), y*blockY()/2, blockX(), blockY()/2, x*blockX(), y*blockY()/2);


  // the idea behind this code is to repaint the dirty blocks. Dominos that are actually
  // within neighbor block must be repaint, too, when they might reach into the actual
  // block. But painting the neighbors is only necessary, when they are not drawn on
  // their own anyway, so always check for !dirty of the "home-block" of each domino

  for (size_t y = 0; y < level->levelY(); y++) {
    for (size_t x = 0; x < level->levelX(); x++) {

      if (!dirty.isDirty(x, y)) continue;

      // paint the left neighbor domino, if it leans in our direction and is not painted on its own
      if (y < level->levelY() && x > 0 && !dirty.isDirty(x-1, y+1) && level->dominoLeansRight(x-1, y+1))
      {
        drawDomino(x-1, y+1);
      }

      if (x > 0 && !dirty.isDirty(x-1, y) && level->dominoLeansRight(x-1, y))
      {
        drawDomino(x-1, y);
      }

      if (y < level->levelY() && !dirty.isDirty(x, y+1) && level->getDominoType(x, y+1) != DominoTypeEmpty)
      {
        drawDomino(x, y+1);
      }

      if (y+2 < level->levelY() && !dirty.isDirty(x, y+2) && level->getDominoType(x, y+2) != DominoTypeEmpty)
      {
        drawDomino(x, y+2);
      }

      drawDomino(x, y);

      // paint the right neighbor if it is leaning in our direction
      if (x+1 < level->levelX() && y+1 < level->levelY() && !dirty.isDirty(x+1, y+1) && level->dominoLeansLeft(x+1, y+1))
      {
        drawDomino(x+1, y+1);
      }

      if (x+1 < level->levelX() && !dirty.isDirty(x+1, y) && level->dominoLeansLeft(x+1, y))
      {
        drawDomino(x+1, y);
      }

      if (y+2 >= level->levelY()) continue;

      if (!dirty.isDirty(x, y+2) && level->getDominoType(x, y+2) == DominoTypeAscender)
      {
        drawDomino(x, y+2);
      }

      if (x > 0 && !dirty.isDirty(x-1, y+2) && level->getDominoType(x-1, y+2) == DominoTypeAscender)
      {
        drawDomino(x+1, y+2);
      }

      if (x+1 < level->levelX() && !dirty.isDirty(x+1, y+2) && level->getDominoType(x+1, y+2) == DominoTypeAscender)
      {
        drawDomino(x+1, y+2);
      }

      if (level->getDominoType(x, y) != DominoTypeAscender) continue;

      if (!dirty.isDirty(x, y+2) && level->getDominoType(x, y+2) != DominoTypeEmpty)
      {
        drawDomino(x, y+2);
      }

      if (x > 0 && !dirty.isDirty(x-1, y+2) && level->getDominoType(x-1, y+2) != DominoTypeEmpty)
      {
        drawDomino(x-1, y+2);
      }

      if (x+1 >= level->levelX()) continue;
      if (!dirty.isDirty(x+1, y+2)) continue;
      if (level->getDominoType(x+1, y+2) == DominoTypeEmpty) continue;

      drawDomino(x+1, y+2);
    }
  }

}

void graphicsN_c::findDirtyBlocks(void)
{
  // check level structur for changes

  for (size_t y = 0; y < level->levelY(); y++)
    for (size_t x = 0; x < level->levelX(); x++)
    {
      if (level->getPlatform(x, y) != l2.getPlatform(x, y))
      {
        for (int i = 0; i < 3; i++)
          for (int j = 0; j < 3; j++)
          {
            dirty.markDirty(x-1+i, y-1+j);
            dirtybg.markDirty(x-1+i, y-1+j);
          }

        l2.setPlatform(x,y, level->getPlatform(x, y));
      }

      if (level->getDominoType(x, y) != l2.getDominoType(x, y) ||
          level->getDominoState(x, y) != l2.getDominoState(x, y) ||
          level->getDominoYOffset(x, y) != l2.getDominoYOffset(x, y)
         )
      {
        for (int i = 0; i < 3; i++)
          for (int j = 0; j < 5; j++)
          {
            dirty.markDirty(x-1+i, y-3+j);
          }

        l2.setDominoType(x, y, level->getDominoType(x, y));
        l2.setDominoState(x, y, level->getDominoState(x, y));
        l2.setDominoYOffset(x, y, level->getDominoYOffset(x, y));
      }
    }

  // check for changes in doors
  //
  // The thing with the door is that it is drawn one level _above_
  // the position where it is, and also the door position is the base
  // is is 2 levels high, so we need to mark 2 blocks as dirty and they
  // are 1 and 2 levels above the door position
  if (level->getEntryState() != l2.getEntryState())
  {
    dirty.markDirty(level->getEntryX(), level->getEntryY()-1);
    dirty.markDirty(level->getEntryX(), level->getEntryY()-2);
    dirtybg.markDirty(level->getEntryX(), level->getEntryY()-1);
    dirtybg.markDirty(level->getEntryX(), level->getEntryY()-2);

    while (level->getEntryState() < l2.getEntryState()) l2.closeEntryDoorStep();
    while (level->getEntryState() > l2.getEntryState()) l2.openEntryDoorStep();
  }

  if (level->getExitState() != l2.getExitState())
  {
    dirty.markDirty(level->getExitX(), level->getExitY()-1);
    dirty.markDirty(level->getExitX(), level->getExitY()-2);
    dirtybg.markDirty(level->getExitX(), level->getExitY()-1);
    dirtybg.markDirty(level->getExitX(), level->getExitY()-2);

    while (level->getExitState() < l2.getExitState()) l2.closeExitDoorStep();
    while (level->getExitState() > l2.getExitState()) l2.openExitDoorStep();
  }

  if (ant)
  {
    // check for ant movement
    if (ant->getBlockX() != antX || ant->getBlockY() != antY || ant->getAnimation() != antAnim || ant->getAnimationImage() != antImage)
    {
      antX = ant->getBlockX();
      antY = ant->getBlockY();
      antAnim = ant->getAnimation();
      antImage = ant->getAnimationImage();

      for (int i = 0; i < 5; i++)
        for (int j = 0; j < 7; j++)
        {
          dirty.markDirty(antX-2+i, antY-4+j);
        }
    }
  }

  int timeLeft = level->getTimeLeft();

  // calculate the second left
  int tm = timeLeft/18;

  if (timeLeft < 0)
    tm = -tm+1;

  int newSec = tm%60;
  int newMin = tm/60;

  if (newSec != Sec || timeLeft == -1)
  {
    dirty.markDirty(2, 24);
    dirty.markDirty(2, 23);
    dirty.markDirty(1, 24);
    dirty.markDirty(1, 23);
  }

  if (newMin != Min || timeLeft == -1)
  {
    dirty.markDirty(1, 23);
    dirty.markDirty(1, 24);
    dirty.markDirty(0, 23);
    dirty.markDirty(0, 24);
  }

  Sec = newSec;
  Min = newMin;
}

void graphicsN_c::calcTutorial(void)
{
    fontParams_s pars;

    pars.font = FNT_SMALL;
    pars.alignment = ALN_TEXT;

    pars.box.x = blockX()/2;
    pars.box.y = blockY()/4;

    pars.shadow = 0;

    pars.color.r = TXT_COL_R;
    pars.color.g = TXT_COL_G;
    pars.color.b = TXT_COL_B;

    uint16_t blx = 7;
    bool found = false;

    while (blx < level->levelX())
    {
      pars.box.w = blockX()*(blx-2);
      pars.box.h = getFontHeight(pars.font);

      uint16_t h = getTextHeight(&pars, _(level->getTutorial()));
      uint16_t bly = 1+(h+blockY()/2-2)/(blockY()/2);

      // find an empty space in the level with the given proportions
      for (uint16_t y = 1; y < level->levelY()-bly; y++)
        if (levelForegroundEmpty(*level, level->levelX()-blx, y-1, blx, bly+2))
        {
          found = true;
          tutorial_x = level->levelX()-blx;
          tutorial_y = y;
          tutorial_w = blx;
          tutorial_h = bly;

          break;
        }

      if (!found)
      {
        for (uint16_t y = 1; y < level->levelY()-bly; y++)
          if (levelForegroundEmpty(*level, 0, y-1, blx, bly+2))
          {
            found = true;
            tutorial_x = 0;
            tutorial_y = y;
            tutorial_w = blx;
            tutorial_h = bly;

            break;
          }
      }

      if (found) break;

      blx++;
    }

    // last possibility... place upper right corner
    if (!found)
    {
      blx = 7;

      pars.box.w = blockX()*(blx-2);
      pars.box.h = getFontHeight(pars.font);

      uint16_t h = getTextHeight(&pars, _(level->getTutorial()));
      uint16_t bly = 1+(h+blockY()/2-2)/(blockY()/2);

      tutorial_x = level->levelX()-blx;
      tutorial_y = 1;
      tutorial_w = blx;
      tutorial_h = bly;
    }

    tutorial = new surface_c((tutorial_w-1)*blockX(), (tutorial_h)*blockY()/2);

    tutorial->fillRect(0, 0, (tutorial_w-1)*blockX()-0, (tutorial_h)*blockY()/2-0, TXT_COL_R, TXT_COL_G, TXT_COL_B);
    tutorial->fillRect(2, 2, (tutorial_w-1)*blockX()-4, (tutorial_h)*blockY()/2-4, BG_COL_R, BG_COL_G,BG_COL_B, TUT_COL_A);

    pars.box.w = blockX()*(blx-2);
    tutorial->renderText(&pars, _(level->getTutorial()));
}

void graphicsN_c::drawLevel(void) {

  if (!target) return;

  if (!level)
  {
    target->fillRect(0, 0, resolutionX(), resolutionY(), 0, 0, 0);
    return;
  }

  findDirtyBlocks();

  drawDominos();
  drawLadders(true);
  drawAnt();
  drawLadders(false);

#ifdef DEBUG
  if (grid && target)
    for (size_t y = 0; y < level->levelY(); y++)
      for (size_t x = 0; x < level->levelX(); x++)
        if (dirty.isDirty(x, y))
        {
          target->fillRect(x*blockX(), y*blockY()/2, blockX(), 1, 128, 128, 128);
          target->fillRect(x*blockX(), (y+1)*blockY()/2-1, blockX(), 1, 128, 128, 128);
          target->fillRect(x*blockX(), y*blockY()/2, 1, blockY()/2, 128, 128, 128);
          target->fillRect((x+1)*blockX()-1, y*blockY()/2, 1, blockY()/2, 128, 128, 128);
        }
#endif

  if (Min != -1)
  {
    char time[6];
    snprintf(time, 6, "%02i:%02i", Min, Sec);

    fontParams_s pars;
    if (level->getTimeLeft() >= 0)
    {
      pars.color.r = HLP_COL_R;
      pars.color.g = HLP_COL_G;
      pars.color.b = HLP_COL_B;
    }
    else
    {
      pars.color.r = TIME_COL_R;
      pars.color.g = TIME_COL_G;
      pars.color.b = TIME_COL_B;
    }
    pars.font = FNT_BIG;
    pars.alignment = ALN_CENTER;
    pars.box.x = 45;
    pars.box.y = 558;
    pars.box.w = 50;
    pars.box.h = 50;
    pars.shadow = 1;

    target->renderText(&pars, time);
  }

  {
    std::string txt = _("F1: Help");
    fontParams_s pars;

    pars.font = FNT_SMALL;
    pars.alignment = ALN_TEXT;
    pars.box.w = getTextWidth(pars.font, txt);
    pars.box.h = getFontHeight(pars.font);
    pars.box.x = 800-pars.box.w-10;
    pars.box.y = 600-pars.box.h-10;
    pars.shadow = 1;
    pars.color.r = HLP_COL_R;
    pars.color.g = HLP_COL_G;
    pars.color.b = HLP_COL_B;

    target->renderText(&pars, txt);
  }

  // redraw tutorial box

  if (level->getTutorial() != "") {

    if (!tutorial)
    {
      calcTutorial();
    }

    // check, if one of the blocks that the tutorial box overlaps
    // is dirty

    bool tutdirty = false;

    for (int y = 0; y < tutorial_h; y++)
      for (int x = 0; x < tutorial_w; x++)
        if (dirty.isDirty(x+tutorial_x, y+tutorial_y))
          tutdirty = true;


    if (tutdirty)
      target->blitBlock(*tutorial, tutorial_x*blockX()+blockX()/2, tutorial_y*blockY()/2);
  }
}



