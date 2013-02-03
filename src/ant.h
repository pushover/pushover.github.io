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


// This enumeration contains all the possible animations that the ant can
// perform. The number of images in each animation is fixes and can be
// aquired using getAntImages
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
  AntAnimStepOutForLoosingDomino,
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
  AntAnimStepBackForDoor,
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

// these are the return values for performAnimation. When they are signalled is given in the comment
typedef enum
{
  LS_undecided,      // still all things open, level undecided
  LS_solved,         // successfully and in time solved,         when exit door has closed and ant is out
  LS_solvedTime,     // solved but not in time,                  when exit door has closed and ant is out
  LS_died,           // the ant has died                         a while after the ant died
  LS_crashes,        // something crashed                        and level has become quiet (no domino action)
  LS_triggerNotFlat, // the trigger is not flat on the ground    and level has become quiet
  LS_triggerNotLast, // the trigger was not the last to fall     and level has become quiet
  LS_someLeft,       // some dominos are left standing, no push left and level ha sbecome quiet
  LS_carrying        // you are still holding a domino           and level has become quiet

} LevelState;

class ant_c {

  private:

    // The current state of the ant. The state is the current action
    // Most stated do have their own animations, but some dont' in that
    AntAnimationState state;

    // The animation currently played, normally the animation and state
    // is identical, but some states do share animations, this is why we
    // do have this separated
    AntAnimationState animation;

    // The current image of the current animation
    unsigned int animationImage;

    // The domino that the ant currently carries
    DominoType carriedDomino;

    // this contains the number of ticks that have
    // to pass before the next animation image of
    // the current anmation is played
    unsigned int animationTimer;

    // current position of the ant
    int16_t blockX, blockY;

    // the level the ant acts on
    levelPlayer_c & level;

    // how many ticks the ant has been inactive
    unsigned int inactiveTimer;

    // how high have we falln
    unsigned int fallingHight;

    // current looking direction
    signed int direction;

    // in PushLeft or PushRight we only have the start of the push
    // animation, after that it branches off depending on the domino
    // pushed. This variable contains the number of ticks until
    // the 2nd part of the push animation is played
    unsigned int pushDelay;
    // this contains the 2nd part of the push animation
    AntAnimationState pushAnimation;

    // has the final jubilation animation been played?
    bool finalAnimationPlayed;
    bool downChecker, upChecker;

    // number of pushed left, normally 1 or 0
    int numPushsLeft;

    // if the fail or success of the level has been decided
    // it is shown in these 2 variables
    bool levelFail, levelSuccess;

    // set, when the trigger is falln, but something else still moving
    bool triggerNotLast;

  public:

    // initialize the ant state for level entering
    // the level is saved and used later on for dirty block
    // marking, and level modification
    ant_c(levelPlayer_c & level);

    void initForLevel(void);

    // do one animation step for the ant
    LevelState performAnimation(unsigned int keyMask);

    bool isLiving(void) { return state != AntAnimDominoDying && state != AntAnimLandDying && state != AntAnimGhost2; }

    bool isVisible(void) const;

    int16_t getBlockX(void) const { return blockX; }
    int16_t getBlockY(void) const { return blockY; }
    DominoType getCarriedDomino(void) const { return carriedDomino; }
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
    AntAnimationState SFStepOutForLoosingDomino(void);
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
    AntAnimationState SFStepBackForDoor(void);
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

