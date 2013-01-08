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
#include "graphics.h"
#include "levelplayer.h"
#include "soundsys.h"
#include "screen.h"

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
  AntAnimNothing

} AntAnimationState;



// initialize the ant state for level entering
// the level is saved and used later on for dirty block
// marking, and level modification
ant_c::ant_c(levelPlayer_c & level, graphics_c & gr, surface_c & target): level(level), gr(gr), vid(target) {

  initForLevel();
}

void ant_c::initForLevel(void) {
  // ant invisible, outside the screen
  blockX = blockY = 200;
  subBlock = screenBlock = 0;

  state = animation = AntAnimLeaveDoorEnterLevel;
  animationImage = 0;
  animationTimer = 0;
  carriedDomino = 0;
  inactiveTimer = 0;
  numPushsLeft = 1;
  fallingHight = 0;
  direction = 1;
  pushDelay = 0;
  downChecker = false;

  finalAnimationPlayed = levelFail = levelSuccess = false;
}

void ant_c::draw(void) {

  if (blockX < 0 || blockX > 19 || blockY < 0 || blockY > 13) return;

  if (carriedDomino != 0)
  {
    if (animation >= AntAnimPullOutLeft && animation <= AntAnimLoosingDominoLeft)
    {
      int a = animation - AntAnimPullOutLeft;

      int x = (blockX-2)*gr.blockX();
      int y = (blockY)*gr.blockY()+screenBlock+gr.antDisplace();

      y += gr.getMoveOffsetY(a, animationImage);
      x += gr.getMoveOffsetX(a, animationImage);

      int img = gr.getMoveImage(a, animationImage);

      if (img < 32)
      {
        if (img != 7 && (carriedDomino == 2 || carriedDomino == 3 || carriedDomino == 4 || carriedDomino == 10))
        {
          img = 7;
        }

        if (carriedDomino > gr.numDominoTypes)
          vid.blit(gr.getDomino(0, img), x, y);
        else
          vid.blit(gr.getDomino(carriedDomino-1, img), x, y);
      }
      else
      {
        if (carriedDomino > gr.numDominoTypes)
          vid.blit(gr.getCarriedDomino(img-32, 0), x, y);
        else
          vid.blit(gr.getCarriedDomino(img-32, carriedDomino-1), x, y);
      }
    }
    if (animation >= AntAnimCarryLeft && animation <= AntAnimCarryStopRight)
    {
      /* put the domino image of the carried domino */
      int a = animation - AntAnimCarryLeft;

      vid.blit(gr.getCarriedDomino(a, carriedDomino-1),
          (blockX-2)*gr.blockX()+gr.getCarryOffsetX(a, animationImage),
          (blockY)*gr.blockY()+screenBlock+gr.antDisplace()+gr.getCarryOffsetY(a, animationImage));

    }
  }

  if (gr.getAnt(animation, animationImage))
  {
    vid.blit(gr.getAnt(animation, animationImage), blockX*gr.blockX()-45, (blockY)*gr.blockY()+screenBlock+gr.antDisplace()+3);
  }

  /* what comes now, is to put the ladders back in front of the ant,
   * we don't need to do that, if the ant is on the ladder
   */

  if (animation >= AntAnimLadder1 && animation <= AntAnimLadder4) return;
  if (animation >= AntAnimCarryLadder1 && animation <= AntAnimCarryLadder4) return;
  if (animation >= AntAnimXXX1 && animation <= AntAnimXXX4) return;

  if (level.getFg(blockX, blockY) == levelData_c::FgElementPlatformLadderUp)
  {
    vid.blitBlock(gr.getFgTile(levelData_c::FgElementLadderMiddle), (blockX)*gr.blockX(), (blockY)*gr.blockY());
  }
  else
  {
    if ((level.getFg(blockX, blockY) == levelData_c::FgElementPlatformLadderDown) ||
        (level.getFg(blockX, blockY) == levelData_c::FgElementLadder))
    {
      vid.blitBlock(gr.getFgTile(levelData_c::FgElementLadder), (blockX)*gr.blockX(), (blockY)*gr.blockY());
    }
  }

  if ((blockY > 0) &&
      ((level.getFg(blockX, blockY-1) == levelData_c::FgElementPlatformLadderDown) ||
       (level.getFg(blockX, blockY-1) == levelData_c::FgElementLadder)))
  {
    vid.blitBlock(gr.getFgTile(levelData_c::FgElementLadder), (blockX)*gr.blockX(), (blockY-1)*gr.blockY());
  }

  if (blockX > 0 && vid.isDirty(blockX-1, blockY))
  {
    if (level.getFg(blockX-1, blockY) == levelData_c::FgElementPlatformLadderUp)
    {
      vid.blitBlock(gr.getFgTile(levelData_c::FgElementLadderMiddle), (blockX-1)*gr.blockX(), (blockY)*gr.blockY());
    }
    else
    {
      if ((level.getFg(blockX-1, blockY) == levelData_c::FgElementPlatformLadderDown) ||
          (level.getFg(blockX-1, blockY) == levelData_c::FgElementLadder))
      {
        vid.blitBlock(gr.getFgTile(levelData_c::FgElementLadder), (blockX-1)*gr.blockX(), (blockY)*gr.blockY());
      }
    }
    if ((blockY > 0) &&
        ((level.getFg(blockX-1, blockY-1) == levelData_c::FgElementPlatformLadderDown) ||
         (level.getFg(blockX-1, blockY-1) == levelData_c::FgElementLadder)))
    {
      vid.blitBlock(gr.getFgTile(levelData_c::FgElementLadder), (blockX-1)*gr.blockX(), (blockY-1)*gr.blockY());
    }
  }

  if ((blockX < 19) && vid.isDirty(blockX+1, blockY))
  {
    if (level.getFg(blockX+1, blockY) == levelData_c::FgElementPlatformLadderUp)
    {
      vid.blitBlock(gr.getFgTile(levelData_c::FgElementLadderMiddle), (blockX+1)*gr.blockX(), (blockY)*gr.blockY());
    }
    else
    {
      if ((level.getFg(blockX+1, blockY) == levelData_c::FgElementPlatformLadderDown) ||
          (level.getFg(blockX+1, blockY) == levelData_c::FgElementLadder))
      {
        vid.blitBlock(gr.getFgTile(levelData_c::FgElementLadder), (blockX+1)*gr.blockX(), (blockY)*gr.blockY());
      }
    }
    if ((blockY > 0) &&
        ((level.getFg(blockX+1, blockY-1) == levelData_c::FgElementPlatformLadderDown) ||
         (level.getFg(blockX+1, blockY-1) == levelData_c::FgElementLadder)))
    {
      vid.blitBlock(gr.getFgTile(levelData_c::FgElementLadder), (blockX+1)*gr.blockX(), (blockY-1)*gr.blockY());
    }
  }
}

void ant_c::setKeyStates(unsigned int km) {
  keyMask = km;
}

// do one animation step for the ant
void ant_c::performAnimation(void) {

  unsigned int oldAnimation = animation;
  unsigned int oldImage = animationImage;

  if (state == AntAnimNothing) {
    state = callStateFunction(state);
  }

  if (state != AntAnimNothing) {
    state = callStateFunction(state);
  }

  if (oldAnimation != animation || oldImage != animationImage)
  {
    vid.markDirty(blockX, blockY-1);
    vid.markDirty(blockX-1, blockY-1);
    vid.markDirty(blockX+1, blockY-1);

    vid.markDirty(blockX, blockY);
    vid.markDirty(blockX, blockY-2);

    vid.markDirty(blockX-1, blockY-2);
    vid.markDirty(blockX-1, blockY);
    vid.markDirty(blockX-2, blockY-1);
    vid.markDirty(blockX-2, blockY);
    vid.markDirty(blockX+1, blockY-2);
    vid.markDirty(blockX+1, blockY);
    vid.markDirty(blockX+2, blockY-1);
    vid.markDirty(blockX+2, blockY);

    vid.markDirty(blockX+1, blockY+1);
    vid.markDirty(blockX-1, blockY+1);
  }
}

unsigned int ant_c::callStateFunction(unsigned int state) {

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
    case AntAnimXXX7:                      return SFXXX7();
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
    case AntAnimXXX9:                      return SFXXX9();
    case AntAnimStruggingAgainsFallLeft:   return SFFlailing();
    case AntAnimStruggingAgainsFallRight:  return SFFlailing();
    // 60
    case AntAnimVictory:                   return SFVictory();
    case AntAnimShrugging:                 return SFShrugging();
    case AntAnimNoNo:                      return SFNoNo();
    case AntAnimXXXA:                      return SFNextAction();
    case AntAnimDominoDying:               return SFStruck();
    case AntAnimLandDying:                 return SFLandDying();
    case AntAnimNothing:                   return SFNextAction();
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
  if (animationImage >= gr.getAntImages(animation)) {
    animationImage = 0;
    screenBlock = subBlock;
    animationTimer = 0;
    return true;
  }

  screenBlock = subBlock + gr.getAntOffset(animation, animationImage);
  animationTimer = delay;

  return false;
}

// now lot of functions follow that are called when the ant is within
// a certain state. The ant state and the ant animation is nearly always
// the same but it can be different. For example when the current animation
// is finished and we return to the state that decides on the next animation
// only the state changes, the animation stays the same.

// I will not comment all these functions

unsigned int ant_c::SFStruck(void) {
  if (animateAnt(2))
    ;

  return animation;
}

unsigned int ant_c::SFNoNo(void) {
  if (animateAnt(3))
    return AntAnimNothing;

  return animation;
}

unsigned int ant_c::SFShrugging(void) {

  if (animationImage == 1)
    soundSystem_c::instance()->startSound(soundSystem_c::SE_SHRUGGING);

  if (animateAnt(3))
    return AntAnimNothing;

  return animation;
}

unsigned int ant_c::SFVictory(void) {

  if (animationImage == 1)
    soundSystem_c::instance()->startSound(soundSystem_c::SE_VICTORY);

  if (animateAnt(3))
    return AntAnimNothing;

  return animation;
}

unsigned int ant_c::SFXXX9(void) {
  if (animateAnt(0)) {
    animation = AntAnimEnterDoor;
    blockX--;
  }

  return animation;
}

unsigned int ant_c::SFEnterDoor(void) {

  if (animateAnt(0)) {

    // make ant invisible
    blockX = 200;
    blockY = 200;

    // close door
    level.openExitDoor(false);
  }

  return animation;
}

unsigned int ant_c::SFGhost1(void) {
  if (animateAnt(0))
    animation = AntAnimGhost2;

  return animation;
}

unsigned int ant_c::SFGhost2(void) {

  if (animateAnt(2)) {
    animationImage = gr.getAntImages(animation) - 1;
    screenBlock = subBlock + gr.getAntOffset(animation, animationImage);
    animationTimer = 2;
  }

  return animation;
}

unsigned int ant_c::SFLandDying(void) {

  if (animationImage == 3)
    soundSystem_c::instance()->startSound(soundSystem_c::SE_ANT_LANDING);

  if (animateAnt(2))
    animationImage = 12;

  return animation;
}

// in the delay push functions we wait until the domino finally falls

unsigned int ant_c::SFPushDelayLeft(void) {
  if (animateAnt(5) || level.pushDomino(blockX-1, blockY, -1)) {
    animationImage = 9;
    animation = AntAnimPushLeft;
  }

  return animation;
}

unsigned int ant_c::SFPushDelayRight(void) {
  if (animateAnt(5) || level.pushDomino(blockX+1, blockY, -1)) {
    animationImage = 9;
    animation = AntAnimPushRight;
  }

  return animation;
}

unsigned int ant_c::SFPushSpecialLeft(void) {
  if (animateAnt(5)) {
    animationImage = 9;
    animation = AntAnimPushLeft;
  }

  return animation;
}

unsigned int ant_c::SFPushSpecialRight(void) {
  if (animateAnt(5)) {
    animationImage = 9;
    animation = AntAnimPushRight;
  }

  return animation;
}

// pushing dominos functions, when the domino is not falling
// normally the pushDOmino function returns false then
// we branch into another animation

unsigned int ant_c::SFPushLeft(void) {
  if (animationImage == 1) {
    if (!level.pushDomino(blockX-1, blockY, -1)) {
      if (pushDelay == 0) {

        switch(level.getDominoType(blockX-1, blockY)) {
          case levelData_c::DominoTypeStopper:
          case levelData_c::DominoTypeCounter1:
          case levelData_c::DominoTypeCounter2:
          case levelData_c::DominoTypeCounter3:
            pushDelay = 5;
            pushAnimation = AntAnimPushStopperLeft;
            break;

          case levelData_c::DominoTypeExploder:
            animationImage = 9;
            break;

          case levelData_c::DominoTypeDelay:
            pushDelay = 5;
            pushAnimation = AntAnimPushDelayLeft;
            break;

          case levelData_c::DominoTypeAscender:
            pushDelay = 2;
            pushAnimation = AntAnimPushRiserLeft;
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

unsigned int ant_c::SFPushRight(void) {
  if (animationImage == 1) {
    if (!level.pushDomino(blockX+1, blockY, 1)) {
      if (pushDelay == 0) {

        switch(level.getDominoType(blockX+1, blockY)) {
          case levelData_c::DominoTypeStopper:
          case levelData_c::DominoTypeCounter1:
          case levelData_c::DominoTypeCounter2:
          case levelData_c::DominoTypeCounter3:
            pushDelay = 5;
            pushAnimation = AntAnimPushStopperRight;
            break;

          case levelData_c::DominoTypeExploder:
            animationImage = 9;
            break;

          case levelData_c::DominoTypeDelay:
            pushDelay = 5;
            pushAnimation = AntAnimPushDelayRight;
            break;

          case levelData_c::DominoTypeAscender:
            pushDelay = 2;
            pushAnimation = AntAnimPushRiserRight;
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

unsigned int ant_c::SFEnterDominosLeft(void) {
  if (!animateAnt(0))
    return animation;

  animation = AntAnimPushLeft;
  screenBlock = subBlock + gr.getAntOffset(animation, animationImage);

  return AntAnimNothing;
}

unsigned int ant_c::SFEnterDominosRight(void) {
  if (!animateAnt(0))
    return animation;

  animation = AntAnimPushRight;
  screenBlock = subBlock + gr.getAntOffset(animation, animationImage);

  return AntAnimNothing;
}

unsigned int ant_c::SFXXX7(void) {
  if (animateAnt(0))
  {
    animation = AntAnimLoosingDominoRight;
    return animation;
  }
  return AntAnimXXX7;
}

unsigned int ant_c::SFLooseRight(void) {
  if (animateAnt(0)) {
    animation = AntAnimFalling;
    blockY++;
    if (blockY > 12) blockY = 12;
    fallingHight++;
    return AntAnimNothing;
  }

  if (animationImage == 4) {
    level.putDownDomino(blockX+1, blockY, carriedDomino, false);
    level.fallingDomino(blockX+1, blockY);
    carriedDomino = 0;
  } else if (animationImage == 6) {
    blockX++;
  } else if (animationImage == 10) {
    soundSystem_c::instance()->startSound(soundSystem_c::SE_ANT_FALLING);
  }

  return animation;
}

unsigned int ant_c::SFLooseLeft(void) {
  if (animateAnt(0)) {
    animation = AntAnimFalling;
    blockY++;
    if (blockY > 12) blockY = 12;
    fallingHight++;
    return AntAnimNothing;
  }

  if (animationImage == 4) {
    blockX--;
  } else if (animationImage == 8) {
    level.putDownDomino(blockX, blockY, carriedDomino, false);
    level.fallingDomino(blockX, blockY);
    carriedDomino = 0;
  } else if (animationImage == 14) {
    soundSystem_c::instance()->startSound(soundSystem_c::SE_ANT_FALLING);
  }

  return animation;
}

unsigned int ant_c::SFLeaveLadderRight(void) {

  if (!animateAnt(0))
    return animation;

  animation = AntAnimCarryRight;
  return animation;

}

unsigned int ant_c::SFLeaveLadderLeft(void) {

  if (!animateAnt(0))
    return animation;

  animation = AntAnimCarryLeft;
  return animation;
}

unsigned int ant_c::SFEnterLadder(void) {

  if (!animateAnt(0))
    return animation;

  if (direction == -20) {
    animation = AntAnimCarryLadder1;

  } else {
    animation = AntAnimCarryLadder4;
  }

  return animation;
}

unsigned int ant_c::SFPushInLeft(void) {

  if (!animateAnt(0))
    return animation;

  blockX--;
  animation = AntAnimWalkLeft;
  level.putDownDomino(blockX, blockY, carriedDomino, true);
  carriedDomino = levelData_c::DominoTypeEmpty;

  return AntAnimNothing;
}

unsigned int ant_c::SFPushInRight(void) {
  if (!animateAnt(0))
    return animation;

  blockX++;
  animation = AntAnimWalkRight;
  level.putDownDomino(blockX, blockY, carriedDomino, true);
  carriedDomino = levelData_c::DominoTypeEmpty;

  return AntAnimNothing;
}

unsigned int ant_c::SFPullOutLeft(void) {

  if (animationImage == 3)
    soundSystem_c::instance()->startSound(soundSystem_c::SE_PICK_UP_DOMINO);

  if (!animateAnt(0))
    return animation;

  animation = AntAnimCarryLeft;
  return AntAnimNothing;
}

unsigned int ant_c::SFPullOutRight(void) {

  if (animationImage == 3)
    soundSystem_c::instance()->startSound(soundSystem_c::SE_PICK_UP_DOMINO);

  if (!animateAnt(0))
    return animation;

  animation = AntAnimCarryRight;
  return AntAnimNothing;

}

unsigned int ant_c::SFLadder1(void) {

  if (!animateAnt(0))
    return animation;

  blockY--;
  return AntAnimNothing;
}

unsigned int ant_c::SFLadder2(void) {

  if (!animateAnt(0))
    return animation;

  if (carriedDomino)
  {
    animation = AntAnimCarryLadder4;
  } else {
    animation = AntAnimLadder4;
  }

  blockY--;
  return AntAnimNothing;
}

unsigned int ant_c::SFLadder3(void) {

  if (!animateAnt(0))
    return animation;

  blockY++;
  return AntAnimNothing;
}

unsigned int ant_c::SFWalkLeft(void) {

  if (animateAnt(0)) {
    blockX--;
    return AntAnimNothing;
  }

  return animation;
}

unsigned int ant_c::SFWalkRight(void) {

  if (animateAnt(0)) {
    blockX++;
    return AntAnimNothing;
  }

  return animation;
}

unsigned int ant_c::SFJumpUpLeft(void) {
  if (!animateAnt(0)) {
    return animation;
  }

  if (carriedDomino) {
    animation = AntAnimCarryStopLeft;
  } else {
    animation = AntAnimWalkLeft;
  }

  blockX--;

  if (screenBlock == 0) {
    blockY--;
    screenBlock = subBlock = gr.halveBlockDisplace();
  } else {
    screenBlock = subBlock = 0;
  }

  return AntAnimNothing;
}

unsigned int ant_c::SFJumpUpRight(void) {
  if (!animateAnt(0)) {
    return animation;
  }

  if (carriedDomino) {
    animation = AntAnimCarryStopRight;
  } else {
    animation = AntAnimWalkRight;
  }

  blockX++;

  if (screenBlock == 0) {
    blockY--;
    screenBlock = subBlock = gr.halveBlockDisplace();
  } else {
    screenBlock = subBlock = 0;
  }

  return AntAnimNothing;
}

unsigned int ant_c::SFJumpDownLeft(void) {
  if (!animateAnt(0)) {
    return animation;
  }

  if (carriedDomino) {
    animation = AntAnimCarryStopLeft;
  } else {
    animation = AntAnimWalkLeft;
  }

  blockX--;

  if (screenBlock == 0) {
    screenBlock = subBlock = gr.halveBlockDisplace();
  } else {
    blockY++;
    screenBlock = subBlock = 0;
  }

  return AntAnimNothing;
}

unsigned int ant_c::SFJumpDownRight(void) {
  if (!animateAnt(0)) {
    return animation;
  }

  if (carriedDomino) {
    animation = AntAnimCarryStopRight;
  } else {
    animation = AntAnimWalkRight;
  }

  blockX++;

  if (screenBlock == 0) {
    screenBlock = subBlock = gr.halveBlockDisplace();
  } else {
    blockY++;
    screenBlock = subBlock = 0;
  }

  return AntAnimNothing;
}


// we wait until the door is open to enter
// the level
unsigned int ant_c::SFLeaveDoor(void) {
  level.openEntryDoor(true);

  if (!level.isEntryDoorOpen())
    return state;

  if (animationImage == 0) {
    blockX = level.getEntryX();
    blockY = level.getEntryY()+1;

    screenBlock = subBlock + gr.getAntOffset(animation, 0);
  }

  if (animateAnt(0)) {
    animation = AntAnimStepAsideAfterEnter;
  }

  return animation;
}

unsigned int ant_c::SFStepAside(void) {

  if (animateAnt(0)) {
    animation = AntAnimStop;
    level.openEntryDoor(false);
    return AntAnimNothing;
  }

  return animation;
}

unsigned int ant_c::SFInFrontOfExploder(void) {

  if (animateAnt(3)) {
    animation = AntAnimInFrontOfExploderWait;
  }

  return animation;
}

unsigned int ant_c::SFLazying(void) {

  if (animateAnt(3)) {
    return AntAnimNothing;
  }

  return animation;
}

unsigned int ant_c::SFInactive(void) {
  return AntAnimNothing;
}

unsigned int ant_c::SFFlailing(void) {
  if (animateAnt(0)) {
    return AntAnimNothing;
  }

  return animation;
}

unsigned int ant_c::SFStartFallingLeft(void) {

  if (animationImage == 2)
  {
    soundSystem_c::instance()->startSound(soundSystem_c::SE_ANT_FALLING);
  }

  if (animateAnt(0)) {
    blockX--;
    animation = AntAnimFalling;
    blockY = blockY+1 < 12 ? blockY+1 : 12;
    fallingHight++;
    return AntAnimNothing;
  }

  return animation;
}

unsigned int ant_c::SFStartFallingRight(void) {

  if (animationImage == 2)
  {
    soundSystem_c::instance()->startSound(soundSystem_c::SE_ANT_FALLING);
  }

  if (animateAnt(0)) {
    blockX++;

    animation = AntAnimFalling;
    blockY = blockY+1 < 12 ? blockY+1 : 12;
    fallingHight++;
    return AntAnimNothing;
  }

  return animation;
}

unsigned int ant_c::SFFalling(void) {

  if (animateAnt(0)) {

    if (fallingHight == 1 && carriedDomino != 0) {
      level.putDownDomino(blockX, blockY, carriedDomino, false);
      level.fallingDomino(blockX, blockY);
      carriedDomino = 0;
    }

    animation = AntAnimFalling;
    if (blockY+1 < 13) {
      blockY++;
    } else {
      animation = AntAnimGhost1;
      return animation;
    }

    return AntAnimNothing;
  }

  return animation;
}

unsigned int ant_c::SFLanding(void) {

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
unsigned int ant_c::checkForNoKeyActions(void) {

  unsigned int ReturnAntState = AntAnimStop;

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

    if (level.getFg(blockX, blockY) == levelData_c::FgElementPlatformLadderDown ||
        level.getFg(blockX, blockY) == levelData_c::FgElementPlatformLadderUp)
    {
      if (level.getFg(blockX-1, blockY) > levelData_c::FgElementEmpty &&
          level.getFg(blockX-1, blockY) != levelData_c::FgElementLadder)
      {
        if (carriedDomino != 0)
        {
          animation = ReturnAntState = AntAnimXXX4;
        }
        else
        {
          animation = ReturnAntState = 0;
        }

        direction = -1;
        return ReturnAntState;
      }
      if (level.getFg(blockX+1, blockY) == 0)
      {
        direction = -1;
        return ReturnAntState;
      }
      if (level.getFg(blockX+1, blockY) == 5)
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
        animation = ReturnAntState = 1;
        direction = 1;
        return ReturnAntState;
      }
    }
    if ((level.getFg(blockX, blockY-1) == 4 ||
         level.getFg(blockX, blockY-1) == 5) &&
         inactiveTimer > 0x0A0)
    {

      animation = ReturnAntState = AntAnimLadder1;
      return ReturnAntState;
    }
    if ((level.getFg(blockX, blockY+1) == 5 ||
         level.getFg(blockX, blockY+1) == 6 ||
         level.getFg(blockX, blockY+1) == 4) &&
        level.getFg(blockX, blockY) == 5)
    {
      animation = ReturnAntState = AntAnimLadder3;
    }
    return ReturnAntState;
  }

  // if we carry a domino and wait too long we try to set the domino down
  if (carriedDomino != 0)
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
  if (level.getDominoType(blockX, blockY) == levelData_c::DominoTypeExploder)
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

  if (x < 0 && ofs == -1) return false;
  if (x > 19 && ofs == 1) return false;

  // if the target position is not empty, we can't put the domino there
  if (level.getDominoType(x, y) != levelData_c::DominoTypeEmpty) return false;

  // we can't place dominos at the edge, if we are too close to them
  // because we need to step back to do that
  if (level.getFg(x, y) == levelData_c::FgElementPlatformStep1)
  {
    if (ofs  == -1) return false;
  }
  else if (level.getFg(x, y) == levelData_c::FgElementPlatformStep6)
  {
    if (ofs == 1) return false;
  }
  else if (level.getFg(x, y) != levelData_c::FgElementPlatformStart &&
      level.getFg(x, y) != levelData_c::FgElementPlatformMiddle &&
      level.getFg(x, y) != levelData_c::FgElementPlatformEnd &&
      level.getFg(x, y) != levelData_c::FgElementPlatformLadderDown &&
      level.getFg(x, y) != levelData_c::FgElementPlatformLadderUp)
  {   // we need a proper ground for the domino
    return false;
  }
  else if (carriedDomino != levelData_c::DominoTypeVanish && y > 0 &&
      level.getFg(x, y-1) >= levelData_c::FgElementDoor0 &&
      level.getFg(x, y-1) <= levelData_c::FgElementDoor3)
  {   // No domino, except for vanishers may be placed in front of doors
    return false;
  }

  // check neighbor places, if there is a domino falling in our direction, we
  // must not place the domino there....
  if ((x < 19) && level.getDominoType(x+1, y) != levelData_c::DominoTypeEmpty && level.getDominoState(x+1, y) < 8)
    return false;

  if ((x >  0) && level.getDominoType(x-1, y) != levelData_c::DominoTypeEmpty && level.getDominoState(x-1, y) > 8)
    return false;

  // no other reason to not place the domino
  return true;
}

bool ant_c::PushableDomino(int x, int y, int ofs) {

  if (carriedDomino != 0) return false;

  if (level.getDominoType(x+ofs, y) == levelData_c::DominoTypeEmpty) return false;
  if (level.getFg(x+ofs, y) == levelData_c::FgElementEmpty) return false;
  if (level.getFg(x+ofs, y) == levelData_c::FgElementLadder) return false;
  if (subBlock != 0) return false;
  if (level.getDominoType(x+ofs, y) == levelData_c::DominoTypeSplitter) return false;
  if (level.getDominoState(x+ofs, y) != 8) return false;

  if (level.getDominoType(x, y) == levelData_c::DominoTypeEmpty) return true;

  if (ofs == -1)
  {
      return level.getDominoState(x, y) >= 8;
  }

  return level.getDominoState(x, y) <= 8;
}

// ok, this huge function determines what comes next
// it decides this on the currently pressed keys and the surroundings
unsigned int ant_c::SFNextAction(void) {

  animationImage = 0;
  unsigned int returnState;

  // is true, when the ant is on a ladder
  bool onLadder = (animation >= AntAnimLadder1 && animation <= AntAnimLadder4) ||
                  (animation >= AntAnimCarryLadder1 && animation <= AntAnimCarryLadder4);

  // when we have no ground below us and are not on a ladder we need
  // to fall down
  if (level.noGround(blockX, blockY, onLadder))
  {
    fallingHight++;
    if (blockY == 13)
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
      if (blockX < 19)
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
      if (level.getFg(blockX, blockY) != levelData_c::FgElementPlatformStep2 &&
          level.getFg(blockX, blockY) != levelData_c::FgElementPlatformStep5)
      {
      }
      else
      {
        subBlock = screenBlock = 8;
      }
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
      if (numPushsLeft == 0)
      {
      }
      else if (!(keyMask & KEY_ACTION))
      {
      }
      else
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
      else if (!PushableDomino(blockX+1, blockY, -1))
      {
      }
      else
      {
        blockX++;
        animation = AntAnimPushLeft;
        direction = -1;
      }
    }
    else if (level.getFg(blockX, blockY) == levelData_c::FgElementPlatformStep2)
    {
      animation = returnState = AntAnimJunpUpLeft;
      direction = -1;
    }
    else if (level.getFg(blockX, blockY) == levelData_c::FgElementPlatformStep5)
    {
      animation = returnState = AntAnimJunpDownLeft;
      direction = -1;
    }
    else if (level.getFg(blockX, blockY) == levelData_c::FgElementLadder)
    {
    }
    else if (level.getFg(blockX-1, blockY) == 0 ||
        level.getFg(blockX-1, blockY) == 5)
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
    else if (level.getFg(blockX-1, blockY) == 0x0A)
    {
      animation = returnState = AntAnimJunpUpLeft;
      direction = -1;
    }
    else if (level.getFg(blockX-1, blockY) == 0x0B)
    {
      animation = returnState = AntAnimJunpDownLeft;
      direction = -1;
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
      animation = returnState = 0;
      direction = -1;
    }
  }
  else if (keyMask & KEY_RIGHT)
  {
    if (blockX >= 19)
    {
      direction = -1;
    }
    else if (animation == AntAnimPushRight)
    {
      downChecker = true;
      if (numPushsLeft == 0)
      {
      }
      else if (!(keyMask & KEY_ACTION))
      {
      }
      else
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
      else if (!PushableDomino(blockX-1, blockY, 1))
      {
      }
      else
      {
        blockX--;
        animation = AntAnimPushRight;
        direction = 1;
      }
    }
    else if (level.getFg(blockX, blockY) == levelData_c::FgElementPlatformStep2)
    {
      animation = returnState = AntAnimJunpDownRight;
      direction = 1;
    }
    else if (level.getFg(blockX, blockY) == levelData_c::FgElementPlatformStep5)
    {
      animation = returnState = AntAnimJunpUpRight;
      direction = 1;
    }
    else if (level.getFg(blockX, blockY) == levelData_c::FgElementLadder)
    {
    }
    else if (level.getFg(blockX+1, blockY) == levelData_c::FgElementEmpty ||
        level.getFg(blockX+1, blockY) == levelData_c::FgElementLadder)
    {
      if (carriedDomino != 0)
      {
        returnState = AntAnimXXX7;
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
    else if (level.getFg(blockX+1, blockY) == levelData_c::FgElementPlatformStep2)
    {
      animation = returnState = AntAnimJunpDownRight;
      direction = 1;
    }
    else if (level.getFg(blockX+1, blockY) == levelData_c::FgElementPlatformStep7)
    {
      animation = returnState = AntAnimJunpUpRight;
      direction = 1;
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
    if ((level.getFg(blockX, blockY-1) == levelData_c::FgElementDoor3) &&
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
        animation = returnState = AntAnimXXX9;
        upChecker = true;
      }
    }
    else if (level.getFg(blockX, blockY) == levelData_c::FgElementPlatformLadderUp)
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
    else if (level.getFg(blockX, blockY-1) == levelData_c::FgElementLadder)
    {
      animation = returnState = AntAnimLadder1;
      direction = -20;
      upChecker = true;
    }
    else if (level.getFg(blockX, blockY-1) == levelData_c::FgElementPlatformLadderDown)
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
    else if (level.getFg(blockX, blockY) == levelData_c::FgElementPlatformLadderDown)
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
      else if (downChecker)
      {
      }
      else
      {
        animation = returnState = AntAnimLadder4;
        direction = 20;
      }
    }
    else if (level.getFg(blockX, blockY+1) == levelData_c::FgElementLadder)
    {
      animation = returnState = AntAnimLadder3;
      direction = 20;
    }
    else if (level.getFg(blockX, blockY+1) == 6 ||
        (level.getFg(blockX, blockY+1) == 4 &&
         level.getFg(blockX, blockY) == 5))

    {
      animation = returnState = AntAnimLadder3;
      direction = 20;
    }
  }
  else if (keyMask & KEY_ACTION)
  {
    if (animation == AntAnimPushRight)
    {
    }
    else if (animation == AntAnimPushLeft)
    {
    }
    else if ((carriedDomino == 0)
        && (level.getDominoType(blockX, blockY) != 0)
        && (level.getDominoState(blockX, blockY) == 8)
        && (level.getDominoType(blockX, blockY) != levelData_c::DominoTypeAscender || level.getDominoExtra(blockX, blockY) != 0x60)
        && (level.getFg(blockX, blockY) != levelData_c::FgElementLadder)
        )
    {
      if (level.getDominoType(blockX, blockY) == levelData_c::DominoTypeTrigger)
      {
        animation = returnState = AntAnimNoNo;
      }
      else
      {
        carriedDomino = level.pickUpDomino(blockX, blockY);

        if (direction == -1)
        {
          animation = returnState = AntAnimPullOutLeft;
          level.removeDomino(blockX, blockY);
        }
        else
        {
          animation = returnState = AntAnimPullOutRight;
          level.removeDomino(blockX, blockY);
        }
      }
    }
    else if (carriedDomino == 0)
    {
    }
    else if (CanPlaceDomino(blockX, blockY, 0))
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
    else if (!CanPlaceDomino(blockX, blockY, direction))
    {
    }
    else if (direction == -1)
    {
      animation = returnState = AntAnimPushInLeft;
    }
    else if (direction != 1)
    {
    }
    else
    {
      animation = returnState = AntAnimPushInRight;
    }
  }
  else if (direction == -20)
  {
    if (level.getFg(blockX, blockY-1) == 5)
    {
      animation = returnState = AntAnimLadder1;
      direction = -20;
    }
    else if (level.getFg(blockX, blockY-1) != 4)
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
    if (level.getFg(blockX, blockY+1) == 5)
    {
      if (level.getFg(blockX, blockY) == 4)
      {
        direction = 20;
      }
      else
      {
        animation = returnState = AntAnimLadder3;
        direction = 20;
      }
    } else if ((level.getFg(blockX, blockY+1) == 6 ||
                level.getFg(blockX, blockY+1) == 4) &&
               level.getFg(blockX, blockY) == 5)
    {
      animation = returnState = AntAnimLadder3;
      direction = 20;
    }
  }

  // some final checks

  if (returnState != AntAnimStop)
  {
    inactiveTimer = 0;
  }
  else if (carriedDomino != 0 || level.getFg(blockX, blockY) == levelData_c::FgElementLadder || finalAnimationPlayed)
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
    animation += 10;
  }

  fallingHight = 0;
  return returnState;

}

void ant_c::success(void) {
  levelSuccess = true;
  level.openExitDoor(true);
}

void ant_c::fail(void) {
  levelFail = true;
}

