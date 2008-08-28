#include "ant.h"
#include "graphics.h"
#include "level.h"

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
  AntAnimPushDelayLeft,
  AntAnimPushDelayRight,
  AntAnimPushRiserLeft,
  AntAnimPushRiserRight,
  AntAnimXXX5,
  AntAnimXXX6,
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



// init the ant state for level entering
// the level is saved and used later on for dirty block
// marking, and level modification
void ant_c::init(level_c * l, graphics_c * graph) {

  // ant invisible, outside the screen
  blockX = blockY = 200;
  subBlock = screenBlock = 0;

  state = animation = AntAnimLeaveDoorEnterLevel;
  animationImage = 0;
  animationTimer = 0;
  carriedDomino = 0;
  inactiveTimer = 0;

  level = l;
  gr = graph;
}

void ant_c::draw(SDL_Surface * video) {

  SDL_Rect dst;

  dst.x = (blockX-2)*gr->blockX();
  dst.y = (blockY+1)*gr->blockY()-gr->getAnt(animation, animationImage)->h+screenBlock + gr->antDisplace();
  dst.w = gr->blockX();
  dst.h = gr->blockY();
  SDL_BlitSurface(gr->getAnt(animation, animationImage), 0, video, &dst);
}

void ant_c::setKeyStates(unsigned int km) {
  keyMask = km;
}

// do one animation step for the ant
void ant_c::performAnimation(void) {

  if (state == AntAnimNothing) {
    state = callStateFunction(state);
  }

  if (state != AntAnimNothing) {
    state = callStateFunction(state);
  }

  // TODO: we need to check for changes and create dirty blocks
  //
  // for the moment a simple version that marks some blocks aroud the ant
  level->markDirty(blockX, blockY);
  level->markDirty(blockX-1, blockY);
  level->markDirty(blockX+1, blockY);

  level->markDirty(blockX, blockY+1);
  level->markDirty(blockX, blockY-1);

  level->markDirty(blockX-1, blockY-1);
  level->markDirty(blockX-1, blockY+1);
  level->markDirty(blockX+1, blockY-1);
  level->markDirty(blockX+1, blockY+1);
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
    case AntAnimPushDelayLeft:             return SFPushSpecialLeft();
    case AntAnimPushDelayRight:            return SFPushSpecialRight();
    case AntAnimPushRiserLeft:             return SFPushSpecialLeft();
    case AntAnimPushRiserRight:            return SFPushSpecialRight();
    case AntAnimXXX5:                      return SFXXX5();
    case AntAnimXXX6:                      return SFXXX6();
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
    case AntAnimLandDying:                 return SFGhost2();
    case AntAnimNothing:                   return SFNextAction();
    default:                               return AntAnimNothing;
  }
}

bool ant_c::animateAnt(unsigned int delay) {

  if (animationTimer > 0) {
    animationTimer--;
    return false;
  }

  animationImage++;
  if (animationImage >= gr->getAntImages(animation)) {
    animationImage = 0;
    screenBlock = subBlock;
    animationTimer = 0;
    return true;
  }

  screenBlock = subBlock + gr->getAntOffset(animation, animationImage);
  animationTimer = delay;

  return false;
}

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
  if (animateAnt(3))
    return AntAnimNothing;

  return animation;
}

unsigned int ant_c::SFVictory(void) {
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

  if (!animateAnt(0)) {
    blockX = 200;
    blockY = 200;

    level->openExitDoor(false);
  }

  return animation;
}

unsigned int ant_c::SFGhost1(void) {
  if (animateAnt(0))
    animation = AntAnimLandDying;

  return animation;
}

unsigned int ant_c::SFGhost2(void) {

  if (animateAnt(2))
    animationImage = 12;

  return animation;
}

unsigned int ant_c::SFXXX5(void) {
  if (animateAnt(5) || level->pushDomino(blockX-1, blockY, -1)) {
    animationImage = 9;
    animation = AntAnimPushLeft;
  }

  return animation;
}

unsigned int ant_c::SFXXX6(void) {
  if (animateAnt(5) || level->pushDomino(blockX+1, blockY, -1)) {
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

unsigned int ant_c::SFPushLeft(void) {
  if (animationImage == 1) {
    if (level->pushDomino(blockX-1, blockY, -1)) {
      if (pushDelay == 0) {

        switch(level->getDominoType(blockX, blockY)) {
          case level_c::DominoTypeBlocker:
            pushDelay = 5;
            pushAnimation = AntAnimPushDelayLeft;
            break;

          case level_c::DominoTypeExploder:
            animationImage = 9;
            break;

          case level_c::DominoTypeDelay:
            pushDelay = 5;
            pushAnimation = AntAnimXXX5;
            break;

          case level_c::DominoTypeRiser:
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
    if (level->pushDomino(blockX+1, blockY, 1)) {
      if (pushDelay == 0) {

        switch(level->getDominoType(blockX+1, blockY)) {
          case level_c::DominoTypeBlocker:
            pushDelay = 5;
            pushAnimation = AntAnimPushDelayRight;
            break;

          case level_c::DominoTypeExploder:
            animationImage = 9;
            break;

          case level_c::DominoTypeDelay:
            pushDelay = 5;
            pushAnimation = AntAnimXXX6;
            break;

          case level_c::DominoTypeRiser:
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

  // TODO: this is really mysterious why that strange animation???
  // we her animation 36 here....
  screenBlock = subBlock + gr->getAntOffset(38, animationImage);

  return AntAnimNothing;
}

unsigned int ant_c::SFEnterDominosRight(void) {
  if (!animateAnt(0))
    return animation;

  animation = AntAnimPushRight;

  // TODO: this is really mysterious why that strange animation???
  // we her animation 37 here....
  screenBlock = subBlock + gr->getAntOffset(39, animationImage);

  return AntAnimNothing;
}

unsigned int ant_c::SFXXX7(void) {
  if (animateAnt(0))
    animation = AntAnimLoosingDominoRight;
  return animation;
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
    level->putDownDomino(blockX+1, blockY, carriedDomino);
    level->fallingDomino(blockX+1, blockY);
    carriedDomino = 0;
  } else if (animationImage == 6) {
    blockX++;
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
  } else if (animationImage == 6) {
    level->putDownDomino(blockX, blockY, carriedDomino);
    level->fallingDomino(blockX, blockY);
    carriedDomino = 0;
  }

  return animation;
}

unsigned int ant_c::SFLeaveLadderRight(void) {

  if (!animateAnt(0))
    return animation;

  animation = AntAnimCarryRight;
  return AntAnimNothing;

}

unsigned int ant_c::SFLeaveLadderLeft(void) {

  if (!animateAnt(0))
    return animation;

  animation = AntAnimCarryLeft;
  return AntAnimNothing;
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
  level->putDownDomino(blockX, blockY, carriedDomino);
  carriedDomino = 0;

  return AntAnimNothing;
}

unsigned int ant_c::SFPushInRight(void) {
  if (!animateAnt(0))
    return animation;

  blockX++;
  animation = AntAnimWalkRight;
  level->putDownDomino(blockX, blockY, carriedDomino);
  carriedDomino = 0;

  return AntAnimNothing;
}

unsigned int ant_c::SFPullOutLeft(void) {

  if (!animateAnt(0))
    return animation;

  animation = AntAnimCarryLeft;
  return AntAnimNothing;
}

unsigned int ant_c::SFPullOutRight(void) {

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
    screenBlock = subBlock = gr->halveBlockDisplace();
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
    screenBlock = subBlock = gr->halveBlockDisplace();
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
    screenBlock = subBlock = gr->halveBlockDisplace();
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
    screenBlock = subBlock = gr->halveBlockDisplace();
  } else {
    blockY++;
    screenBlock = subBlock = 0;
  }

  return AntAnimNothing;
}

unsigned int ant_c::SFLeaveDoor(void) {
  level->openEntryDoor(true);

  if (!level->isEntryDoorOpen())
    return state;

  if (animationImage == 0) {
    blockX = level->getEntryDoorPosX();
    blockY = level->getEntryDoorPosY();

    screenBlock = subBlock + gr->getAntOffset(animation, 0);
  }

  if (animateAnt(0)) {
    animation = AntAnimStepAsideAfterEnter;
  }

  return animation;
}

unsigned int ant_c::SFStepAside(void) {

  if (animateAnt(0)) {
    animation = AntAnimStop;
    level->openEntryDoor(false);
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
    blockY++;
    fallingHight++;
    return AntAnimNothing;
  }

  return animation;
}

unsigned int ant_c::SFLanding(void) {
  if (animateAnt(0)) {
    animation = AntAnimStop;
    return AntAnimNothing;
  }

  return animation;
}

unsigned int ant_c::SFNextAction(void) {

  if (keyMask) inactiveTimer = 0;

  if (!level->canStandThere(blockX, blockY, subBlock)) {
    if (animation != AntAnimFalling) {
      // TODO start falling here
    }
  } else if (animation == AntAnimFalling) {

    if (fallingHight < 3) {
      animation = AntAnimLanding;
    } else {
      animation = AntAnimLandDying;
    }

  } else if (keyMask & KEY_LEFT) {

    if (level->canStandThere(blockX-1, blockY, subBlock))
      animation = AntAnimWalkLeft;
    else if (subBlock && level->canStandThere(blockX-1, blockY, 0) ||
             !subBlock && level->canStandThere(blockX-1, blockY-1, 1))
      animation = AntAnimJunpUpLeft;
    else if (subBlock && level->canStandThere(blockX-1, blockY+1, 0) ||
             !subBlock && level->canStandThere(blockX-1, blockY, 1))
      animation = AntAnimJunpDownLeft;
    else {
      if (animation == AntAnimStruggingAgainsFallLeft) {

        fallingHight = 0;
        animation = AntAnimSuddenFallLeft;

      } else {

        animation = AntAnimStruggingAgainsFallLeft;

      }
    }

  } else if (keyMask & KEY_RIGHT) {

    if (level->canStandThere(blockX+1, blockY, subBlock))
      animation = AntAnimWalkRight;
    else if (subBlock && level->canStandThere(blockX+1, blockY, 0) ||
             !subBlock && level->canStandThere(blockX+1, blockY-1, 1))
      animation = AntAnimJunpUpRight;
    else if (subBlock && level->canStandThere(blockX+1, blockY+1, 0) ||
             !subBlock && level->canStandThere(blockX+1, blockY, 1))
      animation = AntAnimJunpDownRight;
    else {
      if (animation == AntAnimStruggingAgainsFallRight) {

        fallingHight = 0;
        animation = AntAnimSuddenFallRight;

      } else {

        animation = AntAnimStruggingAgainsFallRight;

      }
    }

  } else if (keyMask & KEY_UP) {
  } else if (keyMask & KEY_DOWN) {
  } else {
    if (level->getDominoType(blockX, blockY+1) == level_c::DominoTypeExploder) {
      if (animation != AntAnimInFrontOfExploderWait)
        animation = AntAnimInFrontOfExploder;
    } else {

      //// TODO tapping and yawning is still not verified....
      inactiveTimer++;

      if (inactiveTimer > 18*5) {

        animation = AntAnimYawning;
        inactiveTimer = 0;

      } else {

        animation = AntAnimStop;
      }
    }
  }

  return animation;
}

