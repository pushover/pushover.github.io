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

    case AntAnimWalkLeft:      return SFWalkLeft();
    case AntAnimWalkRight:     return SFWalkRight();
    case AntAnimJunpUpLeft:    return SFJumpUpLeft();
    case AntAnimJunpUpRight:   return SFJumpUpRight();
    case AntAnimJunpDownLeft:  return SFJumpDownLeft();
    case AntAnimJunpDownRight: return SFJumpDownRight();
    case AntAnimLadder1:
    case AntAnimLadder2:
    case AntAnimLadder3:
    case AntAnimLadder4:                   return AntAnimNothing; /////////////
    case AntAnimCarryLeft:     return SFWalkLeft();
    case AntAnimCarryRight:    return SFWalkRight();
    case AntAnimCarryUpLeft:   return SFJumpUpLeft();
    case AntAnimCarryUpRight:  return SFJumpUpRight();
    case AntAnimCarryDownLeft: return SFJumpDownLeft();
    case AntAnimCarryDownRight:return SFJumpDownRight();
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
    case AntAnimXXX7:
    case AntAnimStop:
    case AntAnimTapping:
    case AntAnimYawning:
    case AntAnimEnterLeft:
    case AntAnimEnterRight:
    case AntAnimPushLeft:
    case AntAnimPushRight:
    case AntAnimPushDelayLeft:
    case AntAnimPushDelayRight:
    case AntAnimPushRiserLeft:
    case AntAnimPushRiserRight:
    case AntAnimXXX5:
    case AntAnimXXX6:
    case AntAnimSuddenFallRight:
    case AntAnimSuddenFallLeft:
    case AntAnimFalling:                             return AntAnimNothing;  /////////
    case AntAnimInFrontOfExploder:         return SFInFrontOfExploder();
    case AntAnimInFrontOfExploderWait:     return SFInactive();
    case AntAnimLanding:
    case AntAnimGhost1:
    case AntAnimGhost2:                              return AntAnimNothing;  /////////
    case AntAnimLeaveDoorEnterLevel:       return SFLeaveDoor();
    case AntAnimStepAsideAfterEnter:       return SFStepAside();
    case AntAnimEnterDoor:
    case AntAnimXXX9:
    case AntAnimStruggingAgainsFallLeft:
    case AntAnimStruggingAgainsFallRight:
    case AntAnimVictory:
    case AntAnimShrugging:
    case AntAnimNoNo:
    case AntAnimXXXA:
    case AntAnimDominoDying:
    case AntAnimLandDying:                           return AntAnimNothing;  /////////
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

unsigned int ant_c::SFInactive(void) {
  return AntAnimNothing;
}

unsigned int ant_c::SFNextAction(void) {

  if (keyMask & KEY_LEFT) {
    if (level->canStandThere(blockX-1, blockY, subBlock))
      animation = AntAnimWalkLeft;
    else if (subBlock && level->canStandThere(blockX-1, blockY, 0) ||
             !subBlock && level->canStandThere(blockX-1, blockY-1, 8))
      animation = AntAnimJunpUpLeft;
    else if (subBlock && level->canStandThere(blockX-1, blockY+1, 0) ||
             !subBlock && level->canStandThere(blockX-1, blockY, 8))
      animation = AntAnimJunpDownLeft;
    else
      animation = AntAnimStop;

  } else if (keyMask & KEY_RIGHT) {
    if (level->canStandThere(blockX+1, blockY, subBlock))
      animation = AntAnimWalkRight;
    else if (subBlock && level->canStandThere(blockX+1, blockY, 0) ||
             !subBlock && level->canStandThere(blockX+1, blockY-1, 8))
      animation = AntAnimJunpUpRight;
    else if (subBlock && level->canStandThere(blockX+1, blockY+1, 0) ||
             !subBlock && level->canStandThere(blockX+1, blockY, 8))
      animation = AntAnimJunpDownRight;
    else
      animation = AntAnimStop;

  } else if (keyMask & KEY_UP) {
  } else if (keyMask & KEY_DOWN) {
  } else {
    if (level->getDominoType(blockX, blockY+1) == level_c::DominoTypeExploder) {
      if (animation != AntAnimInFrontOfExploderWait)
        animation = AntAnimInFrontOfExploder;
    } else {
      animation = AntAnimStop;
    }
  }

  return animation;
}

