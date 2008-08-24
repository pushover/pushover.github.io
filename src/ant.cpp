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

    case AntAnimWalkLeft:                  return SFWalkLeft();
    case AntAnimWalkRight:                 return SFWalkRight();
    case AntAnimJunpUpLeft:                return SFJumpUpLeft();
    case AntAnimJunpUpRight:               return SFJumpUpRight();
    case AntAnimJunpDownLeft:              return SFJumpDownLeft();
    case AntAnimJunpDownRight:             return SFJumpDownRight();
    case AntAnimLadder1:
    case AntAnimLadder2:
    case AntAnimLadder3:
    case AntAnimLadder4:                             return AntAnimNothing; /////////////
    case AntAnimCarryLeft:                 return SFWalkLeft();
    case AntAnimCarryRight:                return SFWalkRight();
    case AntAnimCarryUpLeft:               return SFJumpUpLeft();
    case AntAnimCarryUpRight:              return SFJumpUpRight();
    case AntAnimCarryDownLeft:             return SFJumpDownLeft();
    case AntAnimCarryDownRight:            return SFJumpDownRight();
    case AntAnimCarryLadder1:
    case AntAnimCarryLadder2:
    case AntAnimCarryLadder3:
    case AntAnimCarryLadder4:
    case AntAnimCarryStopLeft:
    case AntAnimCarryStopRight:
    case AntAnimPullOutLeft:
    case AntAnimPullOutRight:
    case AntAnimPushInLeft:
    case AntAnimPushInRight:
    case AntAnimXXX1:
    case AntAnimXXX2:
    case AntAnimXXX3:
    case AntAnimXXX4:
    case AntAnimLoosingDominoRight:
    case AntAnimLoosingDominoLeft:
    case AntAnimXXX7:                                return AntAnimNothing;  /////////
    case AntAnimStop:                      return SFInactive();
    case AntAnimTapping:                   return SFLazying();
    case AntAnimYawning:                   return SFLazying();
    case AntAnimEnterLeft:
    case AntAnimEnterRight:
    case AntAnimPushLeft:
    case AntAnimPushRight:
    case AntAnimPushDelayLeft:
    case AntAnimPushDelayRight:
    case AntAnimPushRiserLeft:
    case AntAnimPushRiserRight:
    case AntAnimXXX5:
    case AntAnimXXX6:                                return AntAnimNothing;  /////////
    case AntAnimSuddenFallRight:           return SFStartFallingRight();
    case AntAnimSuddenFallLeft:            return SFStartFallingLeft();
    case AntAnimFalling:                   return SFFalling();
    case AntAnimInFrontOfExploder:         return SFInFrontOfExploder();
    case AntAnimInFrontOfExploderWait:     return SFInactive();
    case AntAnimLanding:                   return SFLanding();
    case AntAnimGhost1:
    case AntAnimGhost2:                              return AntAnimNothing;  /////////
    case AntAnimLeaveDoorEnterLevel:       return SFLeaveDoor();
    case AntAnimStepAsideAfterEnter:       return SFStepAside();
    case AntAnimEnterDoor:
    case AntAnimXXX9:
    case AntAnimStruggingAgainsFallLeft:   return SFFlailing();
    case AntAnimStruggingAgainsFallRight:  return SFFlailing();
    case AntAnimVictory:
    case AntAnimShrugging:
    case AntAnimNoNo:
    case AntAnimXXXA:
    case AntAnimDominoDying:                         return AntAnimNothing;  /////////
    case AntAnimLandDying:                 return SFLandDying();
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

unsigned int ant_c::SFLandDying(void) {
  if (animateAnt(2)) {
    animationImage = 12;
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

