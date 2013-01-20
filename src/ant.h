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

#ifndef __ANT_H__
#define __ANT_H__

#include "levelplayer.h"

#include <stdint.h>

// this class contains all necessary code for the ant animation

// the following defines are used for the keymask
#define KEY_LEFT 1
#define KEY_UP 2
#define KEY_RIGHT 4
#define KEY_DOWN 8
#define KEY_ACTION 16


typedef enum {
  AntAnimWalkLeft,
  AntAnimWalkRight,
  AntAnimJunpUpLeft,
  AntAnimJunpUpRight,
  AntAnimJunpDownLeft,
  AntAnimJunpDownRight,
  AntAnimLadder1,
  AntAnimLadder2,
  AntAnimLadder3,
  AntAnimLadder4,
  AntAnimCarryLeft,
  AntAnimCarryRight,
  AntAnimCarryUpLeft,
  AntAnimCarryUpRight,
  AntAnimCarryDownLeft,
  AntAnimCarryDownRight,
  AntAnimCarryLadder1,
  AntAnimCarryLadder2,
  AntAnimCarryLadder3,
  AntAnimCarryLadder4,
  AntAnimCarryStopLeft,
  AntAnimCarryStopRight,
  AntAnimPullOutLeft,
  AntAnimPullOutRight,
  AntAnimPushInLeft,
  AntAnimPushInRight,
  AntAnimXXX1,
  AntAnimXXX2,
  AntAnimXXX3,
  AntAnimXXX4,
  AntAnimLoosingDominoRight,
  AntAnimLoosingDominoLeft,
  AntAnimXXX7,
  AntAnimStop,
  AntAnimTapping,
  AntAnimYawning,
  AntAnimEnterLeft,
  AntAnimEnterRight,
  AntAnimPushLeft,
  AntAnimPushRight,
  AntAnimPushStopperLeft,
  AntAnimPushStopperRight,
  AntAnimPushRiserLeft,
  AntAnimPushRiserRight,
  AntAnimPushDelayLeft,
  AntAnimPushDelayRight,
  AntAnimSuddenFallRight,
  AntAnimSuddenFallLeft,
  AntAnimFalling,
  AntAnimInFrontOfExploder,
  AntAnimInFrontOfExploderWait,
  AntAnimLanding,
  AntAnimGhost1,
  AntAnimGhost2,
  AntAnimLeaveDoorEnterLevel,
  AntAnimStepAsideAfterEnter,
  AntAnimEnterDoor,
  AntAnimXXX9,
  AntAnimStruggingAgainsFallLeft,
  AntAnimStruggingAgainsFallRight,
  AntAnimVictory,
  AntAnimShrugging,
  AntAnimNoNo,
  AntAnimXXXA,
  AntAnimDominoDying,
  AntAnimLandDying,
  AntAnimNothing     // this needs to be the last as it is also used to count the different animations

} AntAnimationState;


class ant_c {

  private:

    AntAnimationState state;
    AntAnimationState animation;
    unsigned int animationImage;
    DominoType carriedDomino;
    unsigned int animationTimer;

    int16_t blockX, blockY;

    levelPlayer_c & level;

    unsigned int inactiveTimer;
    unsigned int fallingHight;
    signed int direction;
    unsigned int pushDelay;
    AntAnimationState pushAnimation;
    bool finalAnimationPlayed;
    bool downChecker, upChecker;
    int numPushsLeft;

    bool levelFail, levelSuccess;

    // has the level been checked for completion, that is
    // only done once, once a trigger has fallen
    bool finishCheckDone;

  public:

    // initialize the ant state for level entering
    // the level is saved and used later on for dirty block
    // marking, and level modification
    ant_c(levelPlayer_c & level);

    void initForLevel(void);

    // do one animation step for the ant
    int performAnimation(unsigned int keyMask);

    bool isLiving(void) { return state != AntAnimDominoDying && state != AntAnimLandDying && state != AntAnimGhost2; }

    bool isVisible(void) const;

    int getPushsLeft(void) const { return numPushsLeft; }

    int16_t getBlockX(void) const { return blockX; }
    int16_t getBlockY(void) const { return blockY; }
    unsigned int getCarriedDomino(void) const { return carriedDomino; }
    unsigned int getAnimation(void) const { return animation; }
    unsigned int getAnimationImage(void) const { return animationImage; }

    static uint16_t getAntImages(AntAnimationState ant);

  private:

    AntAnimationState callStateFunction(unsigned int state, unsigned int keyMask);
    bool animateAnt(unsigned int delay);


    AntAnimationState SFLeaveDoor(void);
    AntAnimationState SFStepAside(void);
    AntAnimationState SFWalkLeft(void);
    AntAnimationState SFWalkRight(void);
    AntAnimationState SFJumpUpLeft(void);
    AntAnimationState SFJumpUpRight(void);
    AntAnimationState SFJumpDownLeft(void);
    AntAnimationState SFJumpDownRight(void);
    AntAnimationState SFInFrontOfExploder(void);
    AntAnimationState SFInactive(void);
    AntAnimationState SFLazying(void);
    AntAnimationState SFFlailing(void);
    AntAnimationState SFStartFallingLeft(void);
    AntAnimationState SFStartFallingRight(void);
    AntAnimationState SFFalling(void);
    AntAnimationState SFLanding(void);
    AntAnimationState SFLadder1(void);
    AntAnimationState SFLadder2(void);
    AntAnimationState SFLadder3(void);
    AntAnimationState SFPullOutLeft(void);
    AntAnimationState SFPullOutRight(void);
    AntAnimationState SFPushInLeft(void);
    AntAnimationState SFPushInRight(void);
    AntAnimationState SFLeaveLadderRight(void);
    AntAnimationState SFLeaveLadderLeft(void);
    AntAnimationState SFEnterLadder(void);
    AntAnimationState SFLooseRight(void);
    AntAnimationState SFLooseLeft(void);
    AntAnimationState SFXXX7(void);   // TODO what's this state????
    AntAnimationState SFEnterDominosLeft(void);
    AntAnimationState SFEnterDominosRight(void);
    AntAnimationState SFPushLeft(void);
    AntAnimationState SFPushRight(void);
    AntAnimationState SFPushSpecialLeft(void);
    AntAnimationState SFPushSpecialRight(void);
    AntAnimationState SFPushDelayLeft(void);
    AntAnimationState SFPushDelayRight(void);
    AntAnimationState SFGhost1(void);
    AntAnimationState SFGhost2(void);
    AntAnimationState SFLandDying(void);
    AntAnimationState SFEnterDoor(void);
    AntAnimationState SFXXX9(void);   // TODO what's this state????
    AntAnimationState SFNoNo(void);
    AntAnimationState SFVictory(void);
    AntAnimationState SFShrugging(void);
    AntAnimationState SFStruck(void);


    AntAnimationState SFNextAction(unsigned int keyMask);

    AntAnimationState checkForNoKeyActions(void);
    bool CanPlaceDomino(int x, int y, int ofs);
    bool PushableDomino(int x, int y, int ofs);
};

#endif

