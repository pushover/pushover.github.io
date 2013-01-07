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

#include "graphics.h"

const unsigned char graphics_c::numAntAnimations = 66;
const unsigned char numAntAnimationsImages[graphics_c::numAntAnimations] = {

 6,        // AntAnimWalkLeft,
 6,        // AntAnimWalkRight,
 6,        // AntAnimJunpUpLeft,
 6,        // AntAnimJunpUpRight,
 4,        // AntAnimJunpDownLeft,
 4,        // AntAnimJunpDownRight,
 8,        // AntAnimLadder1,
 8,        // AntAnimLadder2,
 8,        // AntAnimLadder3,
 8,        // AntAnimLadder4,
 6,        // AntAnimCarryLeft,
 6,        // AntAnimCarryRight,
 6,        // AntAnimCarryUpLeft,
 6,        // AntAnimCarryUpRight,
 6,        // AntAnimCarryDownLeft,
 6,        // AntAnimCarryDownRight,
 8,        // AntAnimCarryLadder1,
 8,        // AntAnimCarryLadder2,
 8,        // AntAnimCarryLadder3,
 8,        // AntAnimCarryLadder4,
 1,        // AntAnimCarryStopLeft,
 1,        // AntAnimCarryStopRight,
 15,       // AntAnimPullOutLeft,
 15,       // AntAnimPullOutRight,
 16,       // AntAnimPushInLeft,
 16,       // AntAnimPushInRight,
 2,        // AntAnimXXX1,
 2,        // AntAnimXXX2,
 2,        // AntAnimXXX3,
 2,        // AntAnimXXX4,
 13,       // AntAnimLoosingDominoRight,
 17,       // AntAnimLoosingDominoLeft,
 0,        // AntAnimXXX7,
 1,        // AntAnimStop,
 2,        // AntAnimTapping,
 6,        // AntAnimYawning,
 7,        // AntAnimEnterLeft,
 7,        // AntAnimEnterRight,
 12,       // AntAnimPushLeft,
 12,       // AntAnimPushRight,
 8,        // AntAnimPushStopperLeft,
 8,        // AntAnimPushStopperRight,
 4,        // AntAnimPushRiserLeft,
 4,        // AntAnimPushRiserRight,
 8,        // AntAnimPushDelayLeft,
 8,        // AntAnimPushDelayRight,
 6,        // AntAnimSuddenFallRight,
 6,        // AntAnimSuddenFallLeft,
 4,        // AntAnimFalling,
 3,        // AntAnimInFrontOfExploder,
 1,        // AntAnimInFrontOfExploderWait,
 15,       // AntAnimLanding,
 2,        // AntAnimGhost1,
 4,        // AntAnimGhost2,
 7,        // AntAnimLeaveDoorEnterLevel,
 3,        // AntAnimStepAsideAfterEnter,
 7,        // AntAnimEnterDoor,
 4,        // AntAnimXXX9,
 11,       // AntAnimStruggingAgainsFallLeft,
 11,       // AntAnimStruggingAgainsFallRight,
 8,        // AntAnimVictory,
 8,        // AntAnimShrugging,
 8,        // AntAnimNoNo,
 1,        // AntAnimXXXA,
 1,        // AntAnimDominoDying,
 13        // AntAnimLandDying,

};

graphics_c::graphics_c(void) {

  dominos.resize(numDominoTypes);
  carriedDominos.resize(numDominoTypes);

  for (unsigned int i = 0; i < numDominoTypes; i++) {
    dominos[i].resize(numDominos[i]);
    carriedDominos[i].resize(15);
  }

  ant.resize(numAntAnimations);

  for (unsigned int i = 0; i < numAntAnimations; i++) {
    ant[i].resize(numAntAnimationsImages[i]);
  }
}

void graphics_c::setTheme(const std::string & name) {

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

  loadTheme(name);

}

void graphics_c::addBgTile(SDL_Surface * v) {
  bgTiles[curTheme].push_back(v);
}

void graphics_c::addFgTile(SDL_Surface * v) {
  fgTiles[curTheme].push_back(v);
}

void graphics_c::addBgTile(unsigned int idx, SDL_Surface * v) {
  if (idx >= bgTiles[curTheme].size())
    bgTiles[curTheme].resize(idx+1);

  bgTiles[curTheme][idx] = v;
}

void graphics_c::addFgTile(unsigned int idx, SDL_Surface * v) {
  if (idx >= fgTiles[curTheme].size())
    fgTiles[curTheme].resize(idx+1);

  fgTiles[curTheme][idx] = v;
}

void graphics_c::setDomino(unsigned int type, unsigned int num, SDL_Surface * v) {
  dominos[type][num] = v;
}

void graphics_c::setCarriedDomino(unsigned int image, unsigned int domino, SDL_Surface * v) {
  carriedDominos[image][domino] = v;
}

void graphics_c::addAnt(unsigned int anim, unsigned int img, signed char yOffset, SDL_Surface * v, bool free) {

  antSprite s;
  s.v = v;
  s.ofs = yOffset;
  s.free = free;

  ant[anim][img] = s;
}

const unsigned char graphics_c::numDominoTypes = 23;
const unsigned char graphics_c::numDominos[numDominoTypes] = {
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
  1     // DominoTypeQuaver,
};


graphics_c::~graphics_c(void) {

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

  for (unsigned int i = 0; i < ant.size(); i++)
    for (unsigned int j = 0; j < ant[i].size(); j++)
      if (ant[i][j].free)
        SDL_FreeSurface(ant[i][j].v);

  for (unsigned int i = 0; i < carriedDominos.size(); i++)
    for (unsigned int j = 0; j < carriedDominos[i].size(); j++)
      if (carriedDominos[i][j])
        SDL_FreeSurface(carriedDominos[i][j]);

  for (unsigned int j = 0; j < boxBlocks.size(); j++)
    SDL_FreeSurface(boxBlocks[j]);
}

void graphics_c::addBoxBlock(SDL_Surface * v) {
  boxBlocks.push_back(v);
}

