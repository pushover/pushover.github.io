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

#include "ant.h"

#include "soundsys.h"
#include "screen.h"

#include <assert.h>

// the statemashine for the ant works as follows:
// - detect what the ant has to do next
// - play the complete animation resuling maybe in a new position or somethin else
// - repeat


// all the ant animations do have a certain fixed number of images
const unsigned char numAntAnimationsImages[AntAnimNothing] = {

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
 0,        // AntAnimStepOutForLoosingDomino,
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
 4,        // AntAnimStepBackForDoor,
 11,       // AntAnimStruggingAgainsFallLeft,
 11,       // AntAnimStruggingAgainsFallRight,
 8,        // AntAnimVictory,
 8,        // AntAnimShrugging,
 8,        // AntAnimNoNo,
 1,        // AntAnimXXXA,
 1,        // AntAnimDominoDying,
 13        // AntAnimLandDying,

};


uint16_t ant_c::getAntImages(AntAnimationState ant)
{
  assert(ant < AntAnimNothing);

  return numAntAnimationsImages[ant];
}


// Initialisation
ant_c::ant_c(levelPlayer_c & level): level(level) {

  initForLevel();
}

void ant_c::initForLevel(void) {
  // ant invisible, outside the screen
  blockX = blockY = 200;

  state = animation = AntAnimLeaveDoorEnterLevel;
  animationImage = 0;
  animationTimer = 0;
  carriedDomino = DominoTypeEmpty;
  inactiveTimer = 0;
  numPushsLeft = 1;
  fallingHight = 0;
  direction = 1;
  pushDelay = 0;
  downChecker = false;
  finishCheckDone = false;

  finalAnimationPlayed = levelFail = levelSuccess = false;
}

// do one animation step for the level
int ant_c::performAnimation(unsigned int keyMask)
{
  // first do the doors
  level.performDoors();

  // first check, if the any has no active action, if not try to find one
  if (state == AntAnimNothing) {
    state = callStateFunction(state, keyMask);
  }

  // do the action animation
  if (state != AntAnimNothing) {
    state = callStateFunction(state, keyMask);
  }

  // now the level update for all the dominos
  level.performDominos();

  int reason = 0;

  if (finishCheckDone && !level.levelCompleted(reason))
  {
    // we failed after all
    level.openExitDoor(false);

    return reason;
  }

  if (level.triggerIsFalln() && !finishCheckDone)
  {
    finishCheckDone = true;

    if (level.levelCompleted(reason) && carriedDomino == DominoTypeEmpty && isLiving())
    {
      levelSuccess = true;
      level.openExitDoor(true);
    }
    else
    {
      levelFail = true;

      if (reason)
        return reason;
      else if (carriedDomino != DominoTypeEmpty)
        return 4;
      else
        return 5;
    }
  }

  // if level is inactive for a longer time and no pushes are left
  if (numPushsLeft == 0 && level.levelLongInactive()) {
    // search for a trigger
    //
    if (level.triggerNotFlat())
      return 7;
  }

  return 0;
}

AntAnimationState ant_c::callStateFunction(unsigned int state, unsigned int keyMask) {

  switch (state) {

    // 0
    case AntAnimWalkLeft:                  return SFWalkLeft();
    case AntAnimWalkRight:                 return SFWalkRight();
    case AntAnimJunpUpLeft:                return SFJumpUpLeft();
    case AntAnimJunpUpRight:               return SFJumpUpRight();
    case AntAnimJunpDownLeft:              return SFJumpDownLeft();
    case AntAnimJunpDownRight:             return SFJumpDownRight();
    case AntAnimLadder1:                   return SFLadder1();
    case AntAnimLadder2:                   return SFLadder2();
    case AntAnimLadder3:                   return SFLadder3();
    case AntAnimLadder4:                   return SFLadder3();
    case AntAnimCarryLeft:                 return SFWalkLeft();
    case AntAnimCarryRight:                return SFWalkRight();
    // 12
    case AntAnimCarryUpLeft:               return SFJumpUpLeft();
    case AntAnimCarryUpRight:              return SFJumpUpRight();
    case AntAnimCarryDownLeft:             return SFJumpDownLeft();
    case AntAnimCarryDownRight:            return SFJumpDownRight();
    case AntAnimCarryLadder1:              return SFLadder1();
    case AntAnimCarryLadder2:              return SFLadder2();
    case AntAnimCarryLadder3:              return SFLadder3();
    case AntAnimCarryLadder4:              return SFLadder3();
    case AntAnimCarryStopLeft:             return SFInactive();
    case AntAnimCarryStopRight:            return SFInactive();
    case AntAnimPullOutLeft:               return SFPullOutLeft();
    case AntAnimPullOutRight:              return SFPullOutRight();
    // 24
    case AntAnimPushInLeft:                return SFPushInLeft();
    case AntAnimPushInRight:               return SFPushInRight();
    case AntAnimXXX1:                      return SFEnterLadder();
    case AntAnimXXX2:                      return SFEnterLadder();
    case AntAnimXXX3:                      return SFLeaveLadderRight();
    case AntAnimXXX4:                      return SFLeaveLadderLeft();
    case AntAnimLoosingDominoRight:        return SFLooseRight();
    case AntAnimLoosingDominoLeft:         return SFLooseLeft();
    case AntAnimStepOutForLoosingDomino:   return SFStepOutForLoosingDomino();
    case AntAnimStop:                      return SFInactive();
    case AntAnimTapping:                   return SFLazying();
    case AntAnimYawning:                   return SFLazying();
    // 36
    case AntAnimEnterLeft:                 return SFEnterDominosLeft();
    case AntAnimEnterRight:                return SFEnterDominosRight();
    case AntAnimPushLeft:                  return SFPushLeft();
    case AntAnimPushRight:                 return SFPushRight();
    case AntAnimPushStopperLeft:           return SFPushSpecialLeft();
    case AntAnimPushStopperRight:          return SFPushSpecialRight();
    case AntAnimPushRiserLeft:             return SFPushSpecialLeft();
    case AntAnimPushRiserRight:            return SFPushSpecialRight();
    case AntAnimPushDelayLeft:             return SFPushDelayLeft();
    case AntAnimPushDelayRight:            return SFPushDelayRight();
    case AntAnimSuddenFallRight:           return SFStartFallingRight();
    case AntAnimSuddenFallLeft:            return SFStartFallingLeft();
    // 48
    case AntAnimFalling:                   return SFFalling();
    case AntAnimInFrontOfExploder:         return SFInFrontOfExploder();
    case AntAnimInFrontOfExploderWait:     return SFInactive();
    case AntAnimLanding:                   return SFLanding();
    case AntAnimGhost1:                    return SFGhost1();
    case AntAnimGhost2:                    return SFGhost2();
    case AntAnimLeaveDoorEnterLevel:       return SFLeaveDoor();
    case AntAnimStepAsideAfterEnter:       return SFStepAside();
    case AntAnimEnterDoor:                 return SFEnterDoor();
    case AntAnimStepBackForDoor:           return SFStepBackForDoor();
    case AntAnimStruggingAgainsFallLeft:   return SFFlailing();
    case AntAnimStruggingAgainsFallRight:  return SFFlailing();
    // 60
    case AntAnimVictory:                   return SFVictory();
    case AntAnimShrugging:                 return SFShrugging();
    case AntAnimNoNo:                      return SFNoNo();
    case AntAnimXXXA:                      return SFNextAction(keyMask);
    case AntAnimDominoDying:               return SFStruck();
    case AntAnimLandDying:                 return SFLandDying();
    case AntAnimNothing:                   return SFNextAction(keyMask);
    default:                               return AntAnimNothing;
  }
}

// this function performs the animation by
// increasing the animation step counter when
// the animation delay has passed
//
// when the animation is finished the function returns true
bool ant_c::animateAnt(unsigned int delay) {

  // to conform the the original, we should use
  // animationTimer > 1 here, so the original
  // game is a tiny bit faster for some animations
  //
  // but doing that would destroy a huge set of our
  // recordings, so we don't and live with the tiny
  // difference
  if (animationTimer > 0) {
    animationTimer--;
    return false;
  }

  animationImage++;
  if (animationImage >= getAntImages(animation)) {
    animationImage = 0;
    animationTimer = 0;
    return true;
  }

  animationTimer = delay;

  return false;
}

// now lot of functions follow that are called when the ant is within
// a certain state. The ant state and the ant animation is nearly always
// the same but it can be different. For example when the current animation
// is finished and we return to the state that decides on the next animation
// only the state changes, the animation stays the same.

// I will not comment all these functions

AntAnimationState ant_c::SFStruck(void) {
  if (animateAnt(2))
    ;

  return animation;
}

AntAnimationState ant_c::SFNoNo(void) {
  if (animateAnt(3))
    return AntAnimNothing;

  return animation;
}

AntAnimationState ant_c::SFShrugging(void) {

  if (animationImage == 1)
    soundSystem_c::instance()->startSound(soundSystem_c::SE_SHRUGGING);

  if (animateAnt(3))
    return AntAnimNothing;

  return animation;
}

AntAnimationState ant_c::SFVictory(void) {

  if (animationImage == 1)
    soundSystem_c::instance()->startSound(soundSystem_c::SE_VICTORY);

  if (animateAnt(3))
    return AntAnimNothing;

  return animation;
}

AntAnimationState ant_c::SFStepBackForDoor(void) {
  if (animateAnt(0)) {
    animation = AntAnimEnterDoor;
    blockX--;
  }

  return animation;
}

AntAnimationState ant_c::SFEnterDoor(void) {

  if (animateAnt(0)) {

    // make ant invisible
    blockX = 200;
    blockY = 200;

    // close door
    level.openExitDoor(false);
  }

  return animation;
}

AntAnimationState ant_c::SFGhost1(void) {
  if (animateAnt(0))
    animation = AntAnimGhost2;

  return animation;
}

AntAnimationState ant_c::SFGhost2(void) {

  if (animateAnt(2)) {
    animationImage = getAntImages(animation) - 1;
    animationTimer = 2;
  }

  return animation;
}

AntAnimationState ant_c::SFLandDying(void) {

  if (animationImage == 3)
    soundSystem_c::instance()->startSound(soundSystem_c::SE_ANT_LANDING);

  if (animateAnt(2))
    animationImage = 12;

  return animation;
}

// in the delay push functions we wait until the domino finally falls

AntAnimationState ant_c::SFPushDelayLeft(void) {
  if (animateAnt(5) || level.pushDomino(blockX-1, blockY, -1)) {
    animationImage = 9;
    animation = AntAnimPushLeft;
  }

  return animation;
}

AntAnimationState ant_c::SFPushDelayRight(void) {
  if (animateAnt(5) || level.pushDomino(blockX+1, blockY, -1)) {
    animationImage = 9;
    animation = AntAnimPushRight;
  }

  return animation;
}

// This is the push stopper animation, after a while
// we simply abort the animation and return
AntAnimationState ant_c::SFPushSpecialLeft(void) {
  if (animateAnt(5)) {
    animationImage = 9;
    animation = AntAnimPushLeft;
  }

  return animation;
}

AntAnimationState ant_c::SFPushSpecialRight(void) {
  if (animateAnt(5)) {
    animationImage = 9;
    animation = AntAnimPushRight;
  }

  return animation;
}

// pushing dominos functions, when the domino is not falling
// normally the pushDOmino function returns false then
// we branch into another animation

AntAnimationState ant_c::SFPushLeft(void) {
  if (animationImage == 1) {
    if (!level.pushDomino(blockX-1, blockY, -1)) {
      if (pushDelay == 0) {

        switch(level.getDominoType(blockX-1, blockY)) {
          case DominoTypeStopper:
          case DominoTypeCounter1:
          case DominoTypeCounter2:
          case DominoTypeCounter3:
            pushDelay = 5;
            pushAnimation = AntAnimPushStopperLeft;
            break;

          case DominoTypeExploder:
            animationImage = 9;
            break;

          case DominoTypeDelay:
            pushDelay = 5;
            pushAnimation = AntAnimPushDelayLeft;
            break;

          case DominoTypeAscender:
            pushDelay = 2;
            pushAnimation = AntAnimPushRiserLeft;
            break;

          default:
            assert(0);
            break;
        }
      }
    }
  }

  if (pushDelay) {
    pushDelay--;
    if (pushDelay == 0)
      animation = pushAnimation;

  } else {
    if (animateAnt(0)) {
      blockX--;
      animation = AntAnimStop;
      return AntAnimNothing;
    }
  }

  return animation;
}

AntAnimationState ant_c::SFPushRight(void) {
  if (animationImage == 1) {
    if (!level.pushDomino(blockX+1, blockY, 1)) {
      if (pushDelay == 0) {

        switch(level.getDominoType(blockX+1, blockY)) {
          case DominoTypeStopper:
          case DominoTypeCounter1:
          case DominoTypeCounter2:
          case DominoTypeCounter3:
            pushDelay = 5;
            pushAnimation = AntAnimPushStopperRight;
            break;

          case DominoTypeExploder:
            animationImage = 9;
            break;

          case DominoTypeDelay:
            pushDelay = 5;
            pushAnimation = AntAnimPushDelayRight;
            break;

          case DominoTypeAscender:
            pushDelay = 2;
            pushAnimation = AntAnimPushRiserRight;
            break;

          default:
            assert(0);
            break;
        }
      }
    }
  }

  if (pushDelay) {
    pushDelay--;
    if (pushDelay == 0)
      animation = pushAnimation;

  } else {
    if (animateAnt(0)) {
      blockX++;
      animation = AntAnimStop;
      return AntAnimNothing;
    }
  }

  return animation;
}

AntAnimationState ant_c::SFEnterDominosLeft(void) {
  if (!animateAnt(0))
    return animation;

  animation = AntAnimPushLeft;

  return AntAnimNothing;
}

AntAnimationState ant_c::SFEnterDominosRight(void) {
  if (!animateAnt(0))
    return animation;

  animation = AntAnimPushRight;

  return AntAnimNothing;
}

AntAnimationState ant_c::SFStepOutForLoosingDomino(void) {
  if (animateAnt(0))
  {
    animation = AntAnimLoosingDominoRight;
    return animation;
  }
  return AntAnimStepOutForLoosingDomino;
}

AntAnimationState ant_c::SFLooseRight(void) {
  if (animateAnt(0)) {
    // start falling, we fall 2 blocks at the start as if there is a block one step below us
    // we don't fall but rather jump down
    animation = AntAnimFalling;
    blockY += 2;
    if ((size_t)(blockY+1) > level.levelY()) blockY = level.levelY()-1;
    fallingHight++;
    return AntAnimNothing;
  }

  if (animationImage == 4) {
    level.putDownDomino(blockX+1, blockY, carriedDomino, false);
    level.fallingDomino(blockX+1, blockY);
    carriedDomino = DominoTypeEmpty;
  } else if (animationImage == 6) {
    blockX++;
  } else if (animationImage == 10) {
    soundSystem_c::instance()->startSound(soundSystem_c::SE_ANT_FALLING);
  }

  return animation;
}

AntAnimationState ant_c::SFLooseLeft(void) {
  if (animateAnt(0)) {
    animation = AntAnimFalling;
    blockY += 2;
    if ((size_t)(blockY+1) > level.levelY()) blockY = level.levelY()-1;
    fallingHight++;
    return AntAnimNothing;
  }

  if (animationImage == 4) {
    blockX--;
  } else if (animationImage == 8) {
    level.putDownDomino(blockX, blockY, carriedDomino, false);
    level.fallingDomino(blockX, blockY);
    carriedDomino = DominoTypeEmpty;
  } else if (animationImage == 14) {
    soundSystem_c::instance()->startSound(soundSystem_c::SE_ANT_FALLING);
  }

  return animation;
}

AntAnimationState ant_c::SFLeaveLadderRight(void) {

  if (!animateAnt(0))
    return animation;

  animation = AntAnimCarryRight;
  return animation;

}

AntAnimationState ant_c::SFLeaveLadderLeft(void) {

  if (!animateAnt(0))
    return animation;

  animation = AntAnimCarryLeft;
  return animation;
}

AntAnimationState ant_c::SFEnterLadder(void) {

  if (!animateAnt(0))
    return animation;

  if (direction == -20) {
    animation = AntAnimCarryLadder1;

  } else {
    animation = AntAnimCarryLadder4;
  }

  return animation;
}

AntAnimationState ant_c::SFPushInLeft(void) {

  if (!animateAnt(0))
    return animation;

  blockX--;
  animation = AntAnimWalkLeft;
  level.putDownDomino(blockX, blockY, carriedDomino, true);
  carriedDomino = DominoTypeEmpty;

  return AntAnimNothing;
}

AntAnimationState ant_c::SFPushInRight(void) {
  if (!animateAnt(0))
    return animation;

  blockX++;
  animation = AntAnimWalkRight;
  level.putDownDomino(blockX, blockY, carriedDomino, true);
  carriedDomino = DominoTypeEmpty;

  return AntAnimNothing;
}

AntAnimationState ant_c::SFPullOutLeft(void) {

  if (animationImage == 3)
    soundSystem_c::instance()->startSound(soundSystem_c::SE_PICK_UP_DOMINO);

  if (!animateAnt(0))
    return animation;

  animation = AntAnimCarryLeft;
  return AntAnimNothing;
}

AntAnimationState ant_c::SFPullOutRight(void) {

  if (animationImage == 3)
    soundSystem_c::instance()->startSound(soundSystem_c::SE_PICK_UP_DOMINO);

  if (!animateAnt(0))
    return animation;

  animation = AntAnimCarryRight;
  return AntAnimNothing;

}

AntAnimationState ant_c::SFLadder1(void) {

  if (!animateAnt(0))
    return animation;

  blockY -= 2;
  return AntAnimNothing;
}

AntAnimationState ant_c::SFLadder2(void) {

  if (!animateAnt(0))
    return animation;

  if (carriedDomino)
  {
    animation = AntAnimCarryLadder4;
  } else {
    animation = AntAnimLadder4;
  }

  blockY -= 2;
  return AntAnimNothing;
}

AntAnimationState ant_c::SFLadder3(void) {

  if (!animateAnt(0))
    return animation;

  blockY += 2;
  return AntAnimNothing;
}

AntAnimationState ant_c::SFWalkLeft(void) {

  if (animateAnt(0)) {
    blockX--;
    return AntAnimNothing;
  }

  return animation;
}

AntAnimationState ant_c::SFWalkRight(void) {

  if (animateAnt(0)) {
    blockX++;
    return AntAnimNothing;
  }

  return animation;
}

AntAnimationState ant_c::SFJumpUpLeft(void) {
  if (!animateAnt(0)) {
    return animation;
  }

  if (carriedDomino) {
    animation = AntAnimCarryStopLeft;
  } else {
    animation = AntAnimWalkLeft;
  }

  blockX--;
  blockY--;

  return AntAnimNothing;
}

AntAnimationState ant_c::SFJumpUpRight(void) {
  if (!animateAnt(0)) {
    return animation;
  }

  if (carriedDomino) {
    animation = AntAnimCarryStopRight;
  } else {
    animation = AntAnimWalkRight;
  }

  blockX++;
  blockY--;

  return AntAnimNothing;
}

AntAnimationState ant_c::SFJumpDownLeft(void) {
  if (!animateAnt(0)) {
    return animation;
  }

  if (carriedDomino) {
    animation = AntAnimCarryStopLeft;
  } else {
    animation = AntAnimWalkLeft;
  }

  blockX--;
  blockY++;

  return AntAnimNothing;
}

AntAnimationState ant_c::SFJumpDownRight(void) {
  if (!animateAnt(0)) {
    return animation;
  }

  if (carriedDomino) {
    animation = AntAnimCarryStopRight;
  } else {
    animation = AntAnimWalkRight;
  }

  blockX++;
  blockY++;

  return AntAnimNothing;
}


// we wait until the door is open to enter
// the level
AntAnimationState ant_c::SFLeaveDoor(void) {
  level.openEntryDoor(true);

  if (!level.isEntryDoorOpen())
    return state;

  if (animationImage == 0) {
    blockX = level.getEntryX();
    blockY = level.getEntryY();
  }

  if (animateAnt(0)) {
    animation = AntAnimStepAsideAfterEnter;
  }

  return animation;
}

AntAnimationState ant_c::SFStepAside(void) {

  if (animateAnt(0)) {
    animation = AntAnimStop;
    level.openEntryDoor(false);
    return AntAnimNothing;
  }

  return animation;
}

AntAnimationState ant_c::SFInFrontOfExploder(void) {

  if (animateAnt(3)) {
    animation = AntAnimInFrontOfExploderWait;
  }

  return animation;
}

AntAnimationState ant_c::SFLazying(void) {

  if (animateAnt(3)) {
    return AntAnimNothing;
  }

  return animation;
}

AntAnimationState ant_c::SFInactive(void) {
  return AntAnimNothing;
}

AntAnimationState ant_c::SFFlailing(void) {
  if (animateAnt(0)) {
    return AntAnimNothing;
  }

  return animation;
}

AntAnimationState ant_c::SFStartFallingLeft(void) {

  if (animationImage == 2)
  {
    soundSystem_c::instance()->startSound(soundSystem_c::SE_ANT_FALLING);
  }

  if (animateAnt(0)) {
    blockX--;
    animation = AntAnimFalling;
    blockY = (size_t)(blockY+2) < level.levelY() ? blockY+2 : level.levelY();
    fallingHight++;
    return AntAnimNothing;
  }

  return animation;
}

AntAnimationState ant_c::SFStartFallingRight(void) {

  if (animationImage == 2)
  {
    soundSystem_c::instance()->startSound(soundSystem_c::SE_ANT_FALLING);
  }

  if (animateAnt(0)) {
    blockX++;

    animation = AntAnimFalling;
    blockY = (size_t)(blockY+2) < level.levelY() ? blockY+2 : level.levelY();
    fallingHight++;
    return AntAnimNothing;
  }

  return animation;
}

AntAnimationState ant_c::SFFalling(void) {

  if (animateAnt(0)) {

    if (fallingHight == 1 && carriedDomino != DominoTypeEmpty) {
      level.putDownDomino(blockX, blockY, carriedDomino, false);
      level.fallingDomino(blockX, blockY);
      carriedDomino = DominoTypeEmpty;
    }

    animation = AntAnimFalling;
    if (blockY+2 < 26) {
      blockY += 2;
    } else {
      animation = AntAnimGhost1;
      return animation;
    }

    return AntAnimNothing;
  }

  return animation;
}

AntAnimationState ant_c::SFLanding(void) {

  if (animationImage == 2)
    soundSystem_c::instance()->startSound(soundSystem_c::SE_ANT_LANDING);

  if (animateAnt(0)) {
    animation = AntAnimStop;
    return AntAnimNothing;
  }

  return animation;
}

// this function checks for possible actions when
// the user is inactive and the ant in stop state
AntAnimationState ant_c::checkForNoKeyActions(void) {

  AntAnimationState ReturnAntState = AntAnimStop;

  // if the user has prepared a push, we don't do anything
  if (animation == AntAnimPushRight || animation == AntAnimPushLeft)
  {
    return AntAnimNothing;
  }

  // if we are on a ladder
  if (direction == -20 || direction == 20)
  {

    // not long enough inactive
    if (inactiveTimer <= 40)
    {
      return ReturnAntState;
    }

    if (level.getLadder(blockX, blockY))
    {
      if (level.getPlatform(blockX-1, blockY+1))
      {
        if (carriedDomino != 0)
        {
          animation = ReturnAntState = AntAnimXXX4;
        }
        else
        {
          animation = ReturnAntState = AntAnimWalkLeft;
        }

        direction = -1;
        return ReturnAntState;
      }
      if (!level.getPlatform(blockX+1, blockY+1))
      {
        direction = -1;
        return ReturnAntState;
      }
      if (level.getLadder(blockX+1, blockY+1))
      {
        direction = -1;
        return ReturnAntState;
      }
      if (carriedDomino != 0)
      {
        animation = ReturnAntState = AntAnimXXX3;
        direction = 1;
        return ReturnAntState;
      }
      else
      {
        animation = ReturnAntState = AntAnimWalkRight;
        direction = 1;
        return ReturnAntState;
      }
    }
    if (level.getLadder(blockX, blockY-1) &&
        inactiveTimer > 0x0A0)
    {
      animation = ReturnAntState = AntAnimLadder1;
      return ReturnAntState;
    }
    if (level.getLadder(blockX, blockY+2) &&
        level.getLadder(blockX, blockY+1))
    {
      animation = ReturnAntState = AntAnimLadder3;
    }
    return ReturnAntState;
  }

  // if we carry a domino and wait too long we try to set the domino down
  if (carriedDomino != DominoTypeEmpty)
  {
    // stop immediately when no cursor key is given
    if (inactiveTimer == 1)
    {
      if (direction ==  -1)
      {
        animation = ReturnAntState = AntAnimCarryStopLeft;
        return ReturnAntState;
      }
      else
      {
        animation = ReturnAntState = AntAnimCarryStopRight;
        return ReturnAntState;
      }
    }
    // when waited for too long ... set domino down
    if (inactiveTimer > 40 &&
        CanPlaceDomino(blockX, blockY, 0))
    {
      blockX++;
      animationImage = 4;
      animation = ReturnAntState = AntAnimPushInLeft;
      return ReturnAntState;
    }
    if (inactiveTimer > 40 &&
        CanPlaceDomino(blockX, blockY, -1))
    {
      animation = ReturnAntState = AntAnimPushInLeft;
      return ReturnAntState;
    }
    if (inactiveTimer > 40 &&
        CanPlaceDomino(blockX, blockY, 1))
    {
      animation = ReturnAntState = AntAnimPushInRight;
    }
    return ReturnAntState;
  }

  // is we are in front of an exploder -> push hands on ears
  if (level.getDominoType(blockX, blockY) == DominoTypeExploder)
  {
    if (animation == AntAnimInFrontOfExploderWait)
    {
      return ReturnAntState;
    }
    animation = AntAnimInFrontOfExploder;
    return AntAnimInFrontOfExploder;
  }

  // tapping and yawning when nothing else is going on
  if (((inactiveTimer & 0x20) == 0x20) && ((inactiveTimer & 0x1F) == 0))
  {
    soundSystem_c::instance()->startSound(soundSystem_c::SE_NU_WHAT);
    animation = AntAnimTapping;
    return AntAnimTapping;
  }

  if (((inactiveTimer & 0x20) == 0x20) && ((inactiveTimer & 0x1F) < 5))
  {
    animation = AntAnimTapping;
    return AntAnimTapping;
  }
  if (inactiveTimer == 204)
  {
    animation = AntAnimYawning;
    return AntAnimYawning;
  }

  if (inactiveTimer == 220)
  {
    inactiveTimer = 0;
    return ReturnAntState;
  }
  if (inactiveTimer > 4)
  {
    animation = AntAnimStop;
  }

  return ReturnAntState;
}

bool ant_c::CanPlaceDomino(int x, int y, int ofs) {

  if (ofs < -1 || ofs > 1) return false;

  x += ofs;

  // outside of the level
  if (x < 0) return false;
  if ((size_t)(x) >= level.levelX()) return false;

  // if the target position is not empty, we can't put the domino there
  if (level.getDominoType(x, y) != DominoTypeEmpty) return false;
  if (!level.getPlatform(x, y+1)) return false;

  // TODO should go away but required a new level record for 038_a
  if (y%2) return false;

  // all stones except for the vanisher may not be placed in front of doors
  if (   (carriedDomino != DominoTypeVanish)
      && (   ((x == level.getEntryX()) && (y == level.getEntryY()))
          || ((x == level.getExitX())  && (y == level.getExitY()))
         )
     )
  {
    return false;
  }

  // check neighbor places, if there is a domino falling in our direction, we
  // must not place the domino there....
  if (((size_t)(x+1) < level.levelX()) && level.getDominoType(x+1, y) != DominoTypeEmpty && level.getDominoState(x+1, y) < 8)
    return false;

  if ((x >  0) && level.getDominoType(x-1, y) != DominoTypeEmpty && level.getDominoState(x-1, y) > 8)
    return false;

  // no other reason to not place the domino
  return true;
}

bool ant_c::PushableDomino(int x, int y, int ofs) {

  if (carriedDomino != 0) return false;

  if (level.getDominoType(x+ofs, y) == DominoTypeEmpty) return false;
  if (!level.getPlatform(x+ofs, y+1)) return false;
//  if (level.getLadder(x+ofs, y-1)) return false;  // TODO there is something with ladders in the original
  if (level.getDominoType(x+ofs, y) == DominoTypeSplitter) return false;
  if (level.getDominoState(x+ofs, y) != 8) return false;

  if (level.getDominoType(x, y) == DominoTypeEmpty) return true;

  if (ofs == -1)
  {
      return level.getDominoState(x, y) >= 8;
  }

  return level.getDominoState(x, y) <= 8;
}

// ok, this huge function determines what comes next
// it decides this on the currently pressed keys and the surroundings
AntAnimationState ant_c::SFNextAction(unsigned int keyMask) {

  animationImage = 0;
  AntAnimationState returnState;

  // is true, when the ant is on a ladder
  bool onLadder = (animation >= AntAnimLadder1 && animation <= AntAnimLadder4) ||
                  (animation >= AntAnimCarryLadder1 && animation <= AntAnimCarryLadder4);

  // when we have no ground below us and are not on a ladder we need
  // to fall down
  if (!level.getPlatform(blockX, blockY+1) && (!onLadder || !level.getLadder(blockX, blockY)))
  {
    fallingHight++;
    if ((size_t)blockY >= level.levelY()+1)
    {
      animation = returnState = AntAnimLandDying;
      animationImage = 12;
      return returnState;
    }
    animation = returnState = AntAnimFalling;
    if (fallingHight == 1)
      soundSystem_c::instance()->startSound(soundSystem_c::SE_ANT_FALLING);
    return returnState;
  }

  // we can get killed by a domino when we are currently pushing something and a
  // domino comes falling towards us
  if (animation == AntAnimPushLeft &&
      animationImage == 0)
  {
    if ((level.getDominoType(blockX, blockY) != 0 &&
         level.getDominoState(blockX, blockY) < 8) ||
        (level.getDominoType(blockX-1, blockY) != 0 &&
         level.getDominoState(blockX-1, blockY) > 8))
    {
      animation = returnState = AntAnimDominoDying;
      return returnState;
    }
  }
  if (animation == AntAnimPushRight &&
      animationImage == 0)
  {
    if ((level.getDominoType(blockX, blockY) != 0 &&
         level.getDominoState(blockX, blockY) > 8) ||
        (level.getDominoType(blockX+1, blockY) != 0 &&
         level.getDominoState(blockX+1, blockY) < 8))
    {
      if ((size_t)(blockX+1) < level.levelX())
      {
        blockX++;
        animation = returnState = AntAnimDominoDying;
        return returnState;
      }
      animation = returnState = AntAnimDominoDying;
      return returnState;
    }
  }

  returnState = AntAnimStop;

  if (!(keyMask & KEY_DOWN))
  {
    downChecker = false;
  }
  if (!(keyMask & KEY_UP))
  {
    upChecker = false;
  }
  if (fallingHight != 0)
  {
    if (fallingHight > 3)
    {
      animation = returnState = AntAnimLandDying;
    }
    else
    {
      animation = returnState = AntAnimLanding;
    }
  }
  else if (keyMask & KEY_LEFT)
  {
    if (blockX <= 0)
    {
      direction = 1;
    }
    else if (animation == AntAnimPushLeft)
    {
      downChecker = true;
      if ((numPushsLeft > 0) && (keyMask & KEY_ACTION))
      {
        numPushsLeft--;
        returnState = AntAnimPushLeft;
        direction = -1;
      }
    }
    else if (animation == AntAnimPushRight)
    {
      downChecker = true;
      if (numPushsLeft && keyMask & KEY_ACTION)
      {
        blockX++;
        animation = returnState = AntAnimPushLeft;
        if (!PushableDomino(blockX, blockY, -1))
        {
          animationImage = 9;
          direction = -1;
        }
        else
        {
          numPushsLeft--;
          direction = -1;
        }
      }
      else if (PushableDomino(blockX+1, blockY, -1))
      {
        blockX++;
        animation = AntAnimPushLeft;
        direction = -1;
      }
    }
    else if (level.getPlatform(blockX-1, blockY))
    {
      animation = returnState = AntAnimJunpUpLeft;
      direction = -1;
    }
    else if (!level.getPlatform(blockX-1, blockY+1) && level.getPlatform(blockX-1, blockY+2))
    {
      animation = returnState = AntAnimJunpDownLeft;
      direction = -1;
    }
    else if (!level.getPlatform(blockX, blockY+1) && level.getLadder(blockX, blockY))
    {
    }
    else if (!level.getPlatform(blockX-1, blockY+1))
    {
      if (carriedDomino != 0)
      {
        animation = returnState = AntAnimLoosingDominoLeft;
        direction = -1;
      }
      else if (animation == AntAnimStruggingAgainsFallLeft)
      {
        animation = returnState = AntAnimSuddenFallLeft;
        direction = -1;
      }
      else
      {
        animation = returnState = AntAnimStruggingAgainsFallLeft;
        direction = -1;
      }
    }
    else if (carriedDomino != 0 &&
        (direction == 20 || direction == -20))
    {
      animation = returnState = AntAnimXXX4;
      direction = -1;
    }
    else if ((keyMask & KEY_UP) && PushableDomino(blockX, blockY, -1))
    {
      animation = returnState = AntAnimEnterLeft;
      direction = -1;
    }
    else
    {
      animation = returnState = AntAnimWalkLeft;
      direction = -1;
    }
  }
  else if (keyMask & KEY_RIGHT)
  {
    if ((size_t)(blockX+1) >= level.levelX())
    {
      direction = -1;
    }
    else if (animation == AntAnimPushRight)
    {
      downChecker = true;
      if ((numPushsLeft > 0) && (keyMask & KEY_ACTION))
      {
        numPushsLeft--;
        returnState = AntAnimPushRight;
        direction = 1;
      }
    }
    else if (animation == AntAnimPushLeft)
    {
      downChecker = true;
      if (numPushsLeft != 0 && keyMask & KEY_ACTION)
      {
        blockX--;
        animation = returnState = AntAnimPushRight;
        if (!PushableDomino(blockX, blockY, 1))
        {
          animationImage = 9;
          direction = 1;
        }
        else
        {
          numPushsLeft--;
          direction = 1;
        }
      }
      else if (PushableDomino(blockX-1, blockY, 1))
      {
        blockX--;
        animation = AntAnimPushRight;
        direction = 1;
      }
    }
    else if (!level.getPlatform(blockX+1, blockY+1) && level.getPlatform(blockX+1, blockY+2))
    {
      animation = returnState = AntAnimJunpDownRight;
      direction = 1;
    }
    else if (level.getPlatform(blockX+1, blockY))
    {
      animation = returnState = AntAnimJunpUpRight;
      direction = 1;
    }
    else if (!level.getPlatform(blockX, blockY+1) && level.getLadder(blockX, blockY))
    {
    }
    else if (!level.getPlatform(blockX+1, blockY+1))
    {
      if (carriedDomino != 0)
      {
        returnState = AntAnimStepOutForLoosingDomino;
        animation = AntAnimCarryRight;
        direction = 1;
      }
      else if (animation == AntAnimStruggingAgainsFallRight)
      {
        animation = returnState = AntAnimSuddenFallRight;
        direction = 1;
      }
      else
      {
        animation = returnState = AntAnimStruggingAgainsFallRight;
        direction = 1;
      }
    }
    else if (carriedDomino != 0 &&
        (direction == 20 || direction == -20))
    {
      animation = returnState = AntAnimXXX3;
      direction = 1;
    }
    else if ((keyMask & KEY_UP) && PushableDomino(blockX, blockY, 1))
    {
      animation = returnState = AntAnimEnterRight;
      direction = 1;
    }
    else
    {
      animation = returnState = AntAnimWalkRight;
      direction = 1;
    }
  }
  else if (keyMask & KEY_UP)
  {
    if ((level.getExitX() == blockX && level.getExitY() == blockY && level.isExitDoorOpen()) &&
        (level.getDominoType(blockX, blockY) == 0 ||
         level.getDominoState(blockX, blockY) > 8) &&
        (level.getDominoType(blockX-1, blockY) == 0 ||
         level.getDominoState(blockX-1, blockY) <= 8)
       )
    {
      if (direction == -1)
      {
        animation = returnState = AntAnimEnterDoor;
        blockX--;
        upChecker = true;
      }
      else
      {
        animation = returnState = AntAnimStepBackForDoor;
        upChecker = true;
      }
    }
    else if (level.getLadder(blockX, blockY) && !level.getLadder(blockX, blockY+1))
    {
      if (carriedDomino == 0)
      {
        animation = returnState = AntAnimLadder1;
        direction = -20;
        upChecker = true;
      }
      else if (direction == -1)
      {
        animation = returnState = AntAnimXXX2;
        direction = -20;
        upChecker = true;
      }
      else
      {
        animation = returnState = AntAnimXXX1;
        direction = -20;
        upChecker = true;
      }
    }
    else if (level.getLadder(blockX, blockY-1) && !level.getPlatform(blockX, blockY-1))
    {
      animation = returnState = AntAnimLadder1;
      direction = -20;
      upChecker = true;
    }
    else if (level.getLadder(blockX, blockY-1) && level.getPlatform(blockX, blockY-1))
    {
      animation = returnState = AntAnimLadder2;
      direction = -20;
      upChecker = true;
    }
    else if (animation == AntAnimPushLeft
        || animation == AntAnimPushRight)
    {
      returnState = AntAnimNothing;
      upChecker = true;
    }
    else if (carriedDomino != 0)
    {
      returnState = AntAnimStop;
      upChecker = true;
    }
    else if (direction == -1 &&
        PushableDomino(blockX, blockY, -1))
    {
      animation = returnState = AntAnimEnterLeft;
      upChecker = true;
    }
    else if (direction == 1 &&
      PushableDomino(blockX, blockY, 1))
    {
      animation = returnState = AntAnimEnterRight;
      upChecker = true;
    }
    else if (!upChecker)
    {
      if (PushableDomino(blockX, blockY, -1))
      {
        animation = returnState = AntAnimEnterLeft;
        upChecker = true;
      }
      else if (PushableDomino(blockX, blockY, 1))
      {
        animation = returnState = AntAnimEnterRight;
        upChecker = true;
      }
      else
      {
        upChecker = true;
      }
    }
  }
  else if (keyMask & KEY_DOWN)
  {
    if (animation == AntAnimPushLeft ||
        animation == AntAnimPushRight)
    {
      returnState = animation ;
      animationImage = 9;
      downChecker = true;
    }
    else if (level.getPlatform(blockX, blockY+1) && level.getLadder(blockX, blockY+1))
    {
      if (carriedDomino != 0)
      {
        if (direction == -1)
        {
          animation = returnState = AntAnimXXX2;
          direction = 20;
        }
        else
        {
          animation = returnState = AntAnimXXX1;
          direction = 20;
        }
      }
      else if (!downChecker)
      {
        animation = returnState = AntAnimLadder4;
        direction = 20;
      }
    }
    else if (level.getLadder(blockX, blockY+2))
    {
      animation = returnState = AntAnimLadder3;
      direction = 20;
    }
  }
  else if (keyMask & KEY_ACTION)
  {
    if ((animation != AntAnimPushRight) && (animation != AntAnimPushLeft) && (carriedDomino == 0)
        && (level.getDominoType(blockX, blockY) != 0)
        && (level.getDominoState(blockX, blockY) == 8)
        && (level.getDominoType(blockX, blockY) != DominoTypeAscender || level.getDominoExtra(blockX, blockY) != 0x60)
        && level.getPlatform(blockX, blockY+1)
          )
    {
      if (level.getDominoType(blockX, blockY) == DominoTypeTrigger)
      {
        animation = returnState = AntAnimNoNo;
      }
      else
      {
        carriedDomino = level.pickUpDomino(blockX, blockY);

        if (direction == -1)
        {
          animation = returnState = AntAnimPullOutLeft;
        }
        else
        {
          animation = returnState = AntAnimPullOutRight;
        }
      }
    }
    else if (carriedDomino != DominoTypeEmpty)
    {
      if (CanPlaceDomino(blockX, blockY, 0))
      {
        if (direction == -1)
        {
          blockX++;
          animation = returnState = AntAnimPushInLeft;
          animationImage = 4;
        }
        else
        {
          blockX--;
          animation = returnState = AntAnimPushInRight;
          animationImage = 4;
        }
      }
      else if (CanPlaceDomino(blockX, blockY, direction))
      {
        if (direction == -1)
        {
          animation = returnState = AntAnimPushInLeft;
        }
        else if (direction == 1)
        {
          animation = returnState = AntAnimPushInRight;
        }
      }
    }
  }
  else if (direction == -20)
  {
    if (level.getLadder(blockX, blockY-1) && !level.getPlatform(blockX, blockY-1))
    {
      animation = returnState = AntAnimLadder1;
      direction = -20;
    }
    else if (!level.getLadder(blockX, blockY-1) || !level.getPlatform(blockX, blockY-1))
    {
      direction = -1;
    }
    else
    {
      animation = returnState = AntAnimLadder2;
      direction = -1;
    }
  }
  else if (direction == 20)
  {
    if (level.getLadder(blockX, blockY+2) && !level.getPlatform(blockX, blockY+2))
    {
      if (level.getLadder(blockX, blockY+1) && level.getPlatform(blockX, blockY+1))
      {
        direction = 20;
      }
      else
      {
        animation = returnState = AntAnimLadder3;
        direction = 20;
      }
    }
  }

  // some final checks

  if (returnState != AntAnimStop)
  {
    inactiveTimer = 0;
  }
  else if (carriedDomino != 0 || (level.getLadder(blockX, blockY+1) && !level.getPlatform(blockX, blockY+1)) || finalAnimationPlayed)
  {
    inactiveTimer++;
    returnState = checkForNoKeyActions();
  }
  else if (levelSuccess)
  {
    animation = returnState = AntAnimVictory;
    finalAnimationPlayed = 1;
  }
  else if (levelFail)
  {
    animation = returnState = AntAnimShrugging;
    finalAnimationPlayed = 1;
  }
  else
  {
    inactiveTimer++;
    returnState = checkForNoKeyActions();
  }

  if (carriedDomino && animation < AntAnimCarryLeft)
  {
    switch (animation)
    {
      case AntAnimWalkLeft      : animation = AntAnimCarryLeft;      break;
      case AntAnimWalkRight     : animation = AntAnimCarryRight;     break;
      case AntAnimJunpUpLeft    : animation = AntAnimCarryUpLeft;    break;
      case AntAnimJunpUpRight   : animation = AntAnimCarryUpRight;   break;
      case AntAnimJunpDownLeft  : animation = AntAnimCarryDownLeft;  break;
      case AntAnimJunpDownRight : animation = AntAnimCarryDownRight; break;
      case AntAnimLadder1       : animation = AntAnimCarryLadder1;   break;
      case AntAnimLadder2       : animation = AntAnimCarryLadder2;   break;
      case AntAnimLadder3       : animation = AntAnimCarryLadder3;   break;
      case AntAnimLadder4       : animation = AntAnimCarryLadder4;   break;
      default: break;
    }
  }

  fallingHight = 0;
  return returnState;

}

bool ant_c::isVisible(void) const
{
  return blockX >= 0 && (size_t)blockX < level.levelX() && blockY >= 0 && (size_t)blockY < level.levelY();
}

