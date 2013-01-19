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

#include <stdint.h>

// this class contains all necessary code for the ant animation


class levelPlayer_c;
class graphicsN_c;
class surface_c;

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

    unsigned int state;
    AntAnimationState animation;
    unsigned int animationImage;
    unsigned int carriedDomino;
    unsigned int animationTimer;

    int blockX, blockY;

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

    bool carrySomething(void) { return carriedDomino != 0; }
    bool isLiving(void) { return state != 64 && state != 65 && state != 53; }

    void success(void);
    void fail(void);

    bool isVisible(void) const { return blockX >= 0 && blockX < 20 && blockY >= 0 && blockY < 25; }

    int getPushsLeft(void) const { return numPushsLeft; }

    int getBlockX(void) const { return blockX; }
    int getBlockY(void) const { return blockY; }
    unsigned int getCarriedDomino(void) const { return carriedDomino; }
    unsigned int getAnimation(void) const { return animation; }
    unsigned int getAnimationImage(void) const { return animationImage; }

    static uint16_t getAntImages(AntAnimationState ant);

  private:

    unsigned int callStateFunction(unsigned int state, unsigned int keyMask);
    bool animateAnt(unsigned int delay);


    unsigned int SFLeaveDoor(void);
    unsigned int SFStepAside(void);
    unsigned int SFWalkLeft(void);
    unsigned int SFWalkRight(void);
    unsigned int SFJumpUpLeft(void);
    unsigned int SFJumpUpRight(void);
    unsigned int SFJumpDownLeft(void);
    unsigned int SFJumpDownRight(void);
    unsigned int SFInFrontOfExploder(void);
    unsigned int SFInactive(void);
    unsigned int SFLazying(void);
    unsigned int SFFlailing(void);
    unsigned int SFStartFallingLeft(void);
    unsigned int SFStartFallingRight(void);
    unsigned int SFFalling(void);
    unsigned int SFLanding(void);
    unsigned int SFLadder1(void);
    unsigned int SFLadder2(void);
    unsigned int SFLadder3(void);
    unsigned int SFPullOutLeft(void);
    unsigned int SFPullOutRight(void);
    unsigned int SFPushInLeft(void);
    unsigned int SFPushInRight(void);
    unsigned int SFLeaveLadderRight(void);
    unsigned int SFLeaveLadderLeft(void);
    unsigned int SFEnterLadder(void);
    unsigned int SFLooseRight(void);
    unsigned int SFLooseLeft(void);
    unsigned int SFXXX7(void);   // TODO what's this state????
    unsigned int SFEnterDominosLeft(void);
    unsigned int SFEnterDominosRight(void);
    unsigned int SFPushLeft(void);
    unsigned int SFPushRight(void);
    unsigned int SFPushSpecialLeft(void);
    unsigned int SFPushSpecialRight(void);
    unsigned int SFPushDelayLeft(void);
    unsigned int SFPushDelayRight(void);
    unsigned int SFGhost1(void);
    unsigned int SFGhost2(void);
    unsigned int SFLandDying(void);
    unsigned int SFEnterDoor(void);
    unsigned int SFXXX9(void);   // TODO what's this state????
    unsigned int SFNoNo(void);
    unsigned int SFVictory(void);
    unsigned int SFShrugging(void);
    unsigned int SFStruck(void);


    AntAnimationState SFNextAction(unsigned int keyMask);

    AntAnimationState checkForNoKeyActions(void);
    bool CanPlaceDomino(int x, int y, int ofs);
    bool PushableDomino(int x, int y, int ofs);
};

#endif

