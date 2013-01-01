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

#include "levelplayer.h"

#include "textsections.h"
#include "sha1.h"
#include "graphics.h"
#include "soundsys.h"
#include "screen.h"
#include "ant.h"

#include <stdio.h>

#include <sstream>
#include <iomanip>

void levelPlayer_c::load(const textsections_c & sections, const std::string & userString) {

  openDoorEntry = openDoorExit = false;
  resetTriggerFalln();
  finishCheckDone = false;

  levelDisplay_c::load(sections, userString);
}

void levelPlayer_c::performDoors(void) {

  if (openDoorEntry) {

    if (getEntryDoor() == FgElementDoor0)
      soundSystem_c::instance()->startSound(soundSystem_c::SE_DOOR_OPEN);

    if (getEntryDoor() < FgElementDoor3) {
      openEntryDoorStep();
      markDirtyBg(getEntryX(), getEntryY());
    }

  } else {

    if (getEntryDoor() == FgElementDoor3)
      soundSystem_c::instance()->startSound(soundSystem_c::SE_DOOR_CLOSE);

    if (getEntryDoor() > FgElementDoor0) {
      closeEntryDoorStep();
      markDirtyBg(getEntryX(), getEntryY());
    }
  }

  if (openDoorExit) {

    if (getExitDoor() == FgElementDoor0)
      soundSystem_c::instance()->startSound(soundSystem_c::SE_DOOR_OPEN);

    if (getExitDoor() < FgElementDoor3) {
      openExitDoorStep();
      markDirtyBg(getExitX(), getExitY());
    }
  } else {

    if (getExitDoor() == FgElementDoor3)
      soundSystem_c::instance()->startSound(soundSystem_c::SE_DOOR_CLOSE);

    if (getExitDoor() > FgElementDoor0) {
      closeExitDoorStep();
      markDirtyBg(getExitX(), getExitY());
    }
  }
}

int levelPlayer_c::pickUpDomino(int x, int y) {
  int dom = getDominoType(x, y);
  removeDomino(x, y);
  return dom;
}

void levelPlayer_c::putDownDomino(int x, int y, int domino, bool pushin) {

  if (getDominoType(x, y) != 0)
  { // there is a domino in the place where we want to put our domino
    if (pushin)
      DominoCrash(x, y, domino, 0);
    else
      DominoCrash(x, y, domino, 0x70);
  }
  else if (x > 0 && (getDominoType(x-1, y) != DominoTypeEmpty) && (getDominoState(x-1, y) >= 12))
  { // there is no domino in our place but the left neighbor is falling towards us
    DominoCrash(x, y, domino, 0);
  }
  else if (x < 19 && (getDominoType(x+1, y) != DominoTypeEmpty) && getDominoState(x+1, y) <= 4)
  { // there is no domino in our place but the right neighbor is falling towards us
    DominoCrash(x, y, domino, 0);
  }
  else
  { // we can place the domino
    setDominoType(x, y, domino);
    setDominoState(x, y, 8);
    setDominoDir(x, y, 0);
    setDominoYOffset(x, y, 0);
    setDominoExtra(x, y, 0);
  }
}

void levelPlayer_c::fallingDomino(int x, int y) {
  if (getDominoType(x, y) == DominoTypeAscender)
    setDominoExtra(x, y, 0x60);
  else
    setDominoExtra(x, y, 0x70);

  soundSystem_c::instance()->startSound(soundSystem_c::SE_ASCENDER);
}

bool levelPlayer_c::pushDomino(int x, int y, int dir) {

  bool retVal = true;

  if (getDominoExtra(x, y) == 0x70)
  {
    return true;
  }

  // first check for possible crashes
  switch (getDominoType(x, y)) {

    // if we want to push a splitter the stone must not be active right now
    case DominoTypeSplitter:
      if (getDominoDir(x, y) != 0)
      {
        DominoCrash(x, y, getDominoYOffset(x+dir, y), getDominoExtra(x+dir, y));
        return false;
      }
      break;

      // there is never a problem with those types
    case DominoTypeExploder:
    case DominoTypeDelay:
    case DominoTypeVanish:
    case DominoTypeAscender:
    case DominoTypeCrash0:
    case DominoTypeCrash1:
    case DominoTypeCrash2:
    case DominoTypeCrash3:
    case DominoTypeCrash4:
    case DominoTypeCrash5:
    case DominoTypeRiserCont:
      break;

      // the following stones must not fall against the push direction
    case DominoTypeTumbler:
    case DominoTypeBridger:
    case DominoTypeTrigger:
    case DominoTypeEmpty:
    case DominoTypeStandard:
    case DominoTypeStopper:
    case DominoTypeConnectedA:
    case DominoTypeConnectedB:
      if (getDominoDir(x, y) == -dir)
      {
        DominoCrash(x, y, getDominoYOffset(x+dir, y), getDominoExtra(x+dir, y));
        return false;
      }
      break;
  }

  // now push the dominos, we only push, if the domino is not already falling
  switch(getDominoType(x, y)) {

    // these are the default stones, they fall into the given direction
    // and immediately start falling
    case DominoTypeStandard:
    case DominoTypeTumbler:
    case DominoTypeBridger:
    case DominoTypeVanish:
    case DominoTypeTrigger:
      if (getDominoState(x, y) == 8) {
        soundSystem_c::instance()->startSound(getDominoType(x, y)-1);
        setDominoDir(x, y, dir);
        setDominoState(x, y, getDominoState(x, y)+dir);
      }
      break;

      // the splitter is special it only falls in the left direction
      // even though the pieces fall in both directions
    case DominoTypeSplitter:
      if (getDominoState(x, y) == 8) {
        soundSystem_c::instance()->startSound(soundSystem_c::SE_SPLITTER);
        setDominoDir(x, y, -1);
        setDominoState(x, y, getDominoState(x, y)-1);
      }
      break;

      // the exploder only explodes and does not fall in one direction
      // so dir is always set to -1
      // we also need to delay the domino falling against this domino, so
      // return false
    case DominoTypeExploder:
      if (getDominoState(x, y) == 8) {
        soundSystem_c::instance()->startSound(soundSystem_c::SE_EXPLODER);
        setDominoDir(x, y, -1);
        setDominoState(x, y, getDominoState(x, y)-1);
        retVal = false;
      }
      else if (getDominoDir(x, y))
      {
        retVal = false;
      }
      break;

      // for the delay we additionally check, if the delay timer is
      // already running, if it is, we don't push
      // otherwise we start the delay
    case DominoTypeDelay:
      if (getDominoState(x, y) == 8) {
        if (getDominoExtra(x, y) == 0) {
          soundSystem_c::instance()->startSound(soundSystem_c::SE_DELAY);
          setDominoDir(x, y, dir);
          setDominoExtra(x, y, 20);

        }
        retVal = false;
      }
      break;

      // we return false then pushing the riser, because it will rise
      // and the domino has to wait
    case DominoTypeAscender:
      if (getDominoState(x, y) == 16) {
        setDominoDir(x, y, dir);
      }
      else
      {
        if (getDominoState(x, y) == 8 && getDominoYOffset(x, y) > -6) {
          if (getDominoExtra(x, y) != 0x60) {
            soundSystem_c::instance()->startSound(soundSystem_c::SE_ASCENDER);
          }
          setDominoDir(x, y, dir);
          setDominoExtra(x, y, 0x60);
          retVal = false;
        }
      }
      break;

      // here we must go over the complete level and start all connector stones
    case DominoTypeConnectedA:
      {
        bool sound = false;
        for (int yp = 0; yp < 13; yp++)
          for (int xp = 0; xp < 20; xp++)
          {
            if (getDominoType(xp, yp) == DominoTypeConnectedA && getDominoState(xp, yp) == 8)
            {
              sound = true;
              setDominoDir(xp, yp, dir);
              setDominoState(xp, yp, getDominoState(xp, yp)+dir);
            }
            if (getDominoType(xp, yp) == DominoTypeConnectedB && getDominoState(xp, yp) == 8)
            {
              sound = true;
              setDominoDir(xp, yp, -dir);
              setDominoState(xp, yp, getDominoState(xp, yp)-dir);
            }
          }
        if (sound)
        {
          soundSystem_c::instance()->startSound(getDominoType(x, y)-1);
        }
      }
      break;
    case DominoTypeConnectedB:
      {
        bool sound = false;
        for (int yp = 0; yp < 13; yp++)
          for (int xp = 0; xp < 20; xp++)
          {
            if (getDominoType(xp, yp) == DominoTypeConnectedB && getDominoState(xp, yp) == 8)
            {
              sound = true;
              setDominoDir(xp, yp, dir);
              setDominoState(xp, yp, getDominoState(xp, yp)+dir);
            }
            if (getDominoType(xp, yp) == DominoTypeConnectedA && getDominoState(xp, yp) == 8)
            {
              sound = true;
              setDominoDir(xp, yp, -dir);
              setDominoState(xp, yp, getDominoState(xp, yp)-dir);
            }
          }
        if (sound)
        {
          soundSystem_c::instance()->startSound(getDominoType(x, y)-1);
        }
      }
      break;

    case DominoTypeCounter1:
    case DominoTypeCounter2:
    case DominoTypeCounter3:
      {
        if (getDominoState(x, y) == 8)
        {
          setDominoDir(x, y, dir);
          if (CounterStopper(getDominoType(x, y)))
          {
            retVal = false;
          }
          else
          {
            setDominoState(x, y, getDominoState(x, y)+dir);
            soundSystem_c::instance()->startSound(getDominoType(x, y)-1);
          }
        }
      }
      break;

      // for this types we always return false to stop dominos
      // falling against this block
    case DominoTypeStopper:
    case DominoTypeCrash0:
    case DominoTypeCrash1:
    case DominoTypeCrash2:
    case DominoTypeCrash3:
    case DominoTypeCrash4:
    case DominoTypeCrash5:
      retVal = false;
      break;
  }

  return retVal;
}


// the 2 trigger fallen functions. They just call the
// normal domino fallen function and additionally set the trigger bit
void levelPlayer_c::DTA_9(int x, int y) {
  DTA_1(x, y);

  setTriggerFalln();
}
void levelPlayer_c::DTA_N(int x, int y) {
  DTA_K(x, y);

  setTriggerFalln();
}

// this is for the stopper, splitter and exploder dominos, when they
// are falling after being lost when going over the edge
// we check, if we are still falling and only handle the falling case
void levelPlayer_c::DTA_F(int x, int y) {
  if (getDominoExtra(x, y) == 0x70)
    DTA_E(x, y);
}

// this is for the delay domino. We check, if we are falling down when
// falling over the edge or when the delay time is up
// if the delay time is still running, decrement and wait
void levelPlayer_c::DTA_G(int x, int y) {
  if (getDominoExtra(x, y) <= 1 || getDominoExtra(x, y) == 0x70)
    DTA_E(x, y);
  else
    setDominoExtra(x, y, getDominoExtra(x, y)-1);
}

// crash case with dust clouds, we need to call DTA_E because
// dominos might crash in the air and the rubble needs to fall down
void levelPlayer_c::DTA_B(int x, int y) {
  DTA_E(x, y);

  markDirty(x-1, y);
  markDirty(x-1, y-1);
  markDirty(x+1, y);
  markDirty(x+1, y-1);
  markDirty(x, y-1);
  markDirty(x, y-2);
}

// splitter opening simply call the normal domino falling
// function and mark some more blocks because we split left and
// right and the normal function will only mark one side
void levelPlayer_c::DTA_D(int x, int y) {

  markDirty(x+1, y-1);
  markDirty(x+1, y);

  DTA_4(x, y);

  // mark the splitting stone, that now vanishes as dirty
  // the lower 2 lines are for splitting dust clouds
  if (getDominoState(x, y) == 5)
  {
    markDirty(x, y-2);
    markDirty(x-1, y-2);
    markDirty(x+1, y-2);
  }
}

// the final vanisher state, remove the vanisher
// from the level and mark things dirty
void levelPlayer_c::DTA_8(int x, int y) {
  markDirty(x, y);
  markDirty(x+getDominoDir(x, y), y);
  markDirty(x, y-1);
  markDirty(x+getDominoDir(x, y), y-1);

  removeDomino(x, y);
}

// this is the nearly fallen down left case
void levelPlayer_c::DTA_2(int x, int y) {

  if (getDominoExtra(x, y) == 0x40 || getDominoExtra(x, y) == 0x70)
  {
    DTA_4(x, y);
    return;
  }

  // if we move in the other direction (e.g. tumbler) simply continue
  if (getDominoDir(x, y) == 1)
  {
    DTA_4(x, y);
    return;
  }

  // if there is a domino left of us, we can't fall down further
  if (getDominoType(x-1, y) != 0)
    return;

  // if here is a domino 2 to the left of us and that domino has fallen down
  // far enough to the right, we can't fall farther
  if (x >= 2 && getDominoType(x-2, y) != 0 && getDominoState(x-2, y) > 13)
    return;

  // if the domino is a splitter, we need to check, if it is in one of the
  // states where the left halve is far enough down
  // if this is not the case or it is no splitter fall further
  if (x < 2 ||
      getDominoType(x-2, y) != DominoTypeSplitter ||
      (getDominoState(x-2, y) != 1 &&
       getDominoState(x-2, y) != 10 &&
       getDominoState(x-2, y) != 12 &&
       getDominoState(x-2, y) != 13))
  {
    DTA_4(x, y);
  }
}

// nearly fallen down right
void levelPlayer_c::DTA_J(int x, int y) {

  if (getDominoExtra(x, y) == 0x40 || getDominoExtra(x, y) == 0x70)
  {
    DTA_4(x, y);
    return;
  }

  // if we fall in the other direction (tumbler) we simply continue
  if (getDominoDir(x, y) == -1)
  {
    DTA_4(x, y);
    return;
  }

  // if our right neighbor if not empty, we stop
  if (getDominoType(x+1, y) != 0)
    return;

  // if the 2nd next right neighbor is not empty and far enough fallen to the left
  // we stop
  if (x <= 17 && getDominoType(x+2, y) != 0 && getDominoState(x+2, y) < 3)
    return;

  // if the domino is a splitter, we need to check, if it is in one of the
  // states where the right halve is far enough down
  // if this is not the case or it is no splitter fall further
  if (x > 17 ||
      getDominoType(x+2, y) != DominoTypeSplitter ||
      (getDominoState(x+2, y) != 1 &&
       getDominoState(x+2, y) != 9 &&
       getDominoState(x+2, y) != 11 &&
       getDominoState(x+2, y) != 14))
  {
    DTA_4(x, y);
  }
}

// normal falling case
void levelPlayer_c::DTA_4(int x, int y) {
  if (getDominoExtra(x, y) == 0x40 || getDominoExtra(x, y) == 0x70)
  {
    setDominoYOffset(x, y, getDominoYOffset(x, y)+2);
    setDominoState(x, y, getDominoState(x, y)+getDominoDir(x, y));
  }
  else if (getDominoExtra(x, y) == 0x60)
  {
    setDominoYOffset(x, y, getDominoYOffset(x, y)-2);
    setDominoState(x, y, getDominoState(x, y)+getDominoDir(x, y));
  }

  // update state
  setDominoState(x, y, getDominoState(x, y)+getDominoDir(x, y));

  if (getDominoType(x, y) == DominoTypeAscender &&
      getDominoState(x, y) == 8 &&
      getDominoYOffset(x, y) == -10)
  {
    setDominoState(x, y, 16);
  }

  markDirty(x, y);
  markDirty(x, y-1);

  // add some dirty blocks depending on the direction we have fallen
  if (getDominoState(x, y) > 8)
  {
    markDirty(x+1, y);
    markDirty(x+1, y-1);
  }

  if (getDominoState(x, y) < 8)
  {
    markDirty(x-1, y);
    markDirty(x-1, y-1);
  }

  if (getDominoYOffset(x, y) == 8)
  {
    markDirty(x+getDominoDir(x, y), y);
    markDirty(x+getDominoDir(x, y), y-1);
  }

  if (getDominoType(x, y) == DominoTypeAscender)
  {
    markDirty(x+getDominoDir(x, y), y-2);
    markDirty(x-getDominoDir(x, y), y-2);
    markDirty(x, y-2);
  }
}

// exploder making its hole
void levelPlayer_c::DTA_5(int x, int y) {

  removeDomino(x, y);
  setFg(x, y, FgElementEmpty);

  if (getFg(x+1, y) == FgElementPlatformMiddle ||
      getFg(x+1, y) == FgElementPlatformLadderDown ||
      getFg(x+1, y) == FgElementPlatformLadderUp)
  {
    setFg(x+1, y, FgElementPlatformStart);
  }
  else if (getFg(x+1, y) == FgElementPlatformEnd)
  {
    setFg(x+1, y, FgElementPlatformStrip);
  }

  if (getFg(x-1, y) == FgElementPlatformMiddle ||
      getFg(x-1, y) == FgElementPlatformLadderDown ||
      getFg(x-1, y) == FgElementPlatformLadderUp)
  {
    setFg(x-1, y, FgElementPlatformEnd);
  }
  else if (getFg(x-1, y) == FgElementPlatformStart)
  {
    setFg(x-1, y, FgElementPlatformStrip);
  }

  markDirtyBg(x-1, y);
  markDirtyBg(x, y);
  markDirtyBg(x+1, y);
}

// hitting next domino to the left
void levelPlayer_c::DTA_3(int x, int y) {

  // if we hit a step, stop falling
  if (x > 0 && getFg(x-1, y) == FgElementPlatformStep4)
    return;

  if (getDominoYOffset(x, y) == 8 && x > 0 && getFg(x-1, y) == FgElementPlatformStep1)
    return;

  // if the next domino is empty, continue falling
  if (getDominoType(x-1, y) == DominoTypeEmpty) {
    DTA_4(x, y);
    return;
  }

  // if the next domino is not a stopper and not a delay, we
  // simply push that domino and continue falling
  if (getDominoType(x-1, y) != DominoTypeStopper &&
      getDominoType(x-1, y) != DominoTypeDelay &&
      getDominoType(x-1, y) != DominoTypeCounter1 &&
      getDominoType(x-1, y) != DominoTypeCounter2 &&
      getDominoType(x-1, y) != DominoTypeCounter2
      )
  {
    if (pushDomino(x-1, y, -1))
      DTA_4(x, y);
    return;
  }

  // if the next neighbor is a delay we only continue falling, if
  // it has started falling as well
  if (getDominoType(x-1, y) == DominoTypeDelay)
  {
    if (getDominoState(x-1, y) != 8) {
      DTA_4(x, y);
      return;
    }
  }

  if (getDominoType(x-1, y) == DominoTypeCounter1 ||
      getDominoType(x-1, y) == DominoTypeCounter2 ||
      getDominoType(x-1, y) == DominoTypeCounter3
     )
  {
    if (!CounterStopper(getDominoType(x-1, y)))
    {
      if (pushDomino(x-1, y, -1))
      {
        DTA_4(x, y);
        return;
      }
    }
  }

  // if the right neighbor is a splitter and the splitter is already fallen
  // we try to continue falling
  if (getDominoType(x+1, y) == DominoTypeSplitter && getDominoState(x+1, y) != 8)
  {
    if (pushDomino(x-1, y, -1))
      DTA_4(x, y);
    return;
  }

  // if there is a domino to the right of us that has fallen we try to continue
  if (getDominoType(x+1, y) != DominoTypeEmpty && getDominoState(x+1, y) < 8)
  {
    if (pushDomino(x-1, y, -1))
      DTA_4(x, y);
    return;
  }

  // now the only case left is to reverse direction but still push the domino to our left
  setDominoDir(x, y, 1);
  pushDomino(x-1, y, -1);
  soundSystem_c::instance()->startSound(soundSystem_c::SE_STOPPER);
  DTA_4(x, y);
}

// same as DTA_3 but for the right direction
void levelPlayer_c::DTA_I(int x, int y) {

  if (x < 19 && getFg(x+1, y) == FgElementPlatformStep7)
    return;

  if (getDominoYOffset(x, y) == 8 && x < 19 && getFg(x-1, y) == FgElementPlatformStep6)
    return;

  if (getDominoType(x+1, y) == DominoTypeEmpty) {
    DTA_4(x, y);
    return;
  }

  if (getDominoType(x+1, y) != DominoTypeStopper &&
      getDominoType(x+1, y) != DominoTypeDelay &&
      getDominoType(x+1, y) != DominoTypeCounter1 &&
      getDominoType(x+1, y) != DominoTypeCounter2 &&
      getDominoType(x+1, y) != DominoTypeCounter3
     )
  {
    if (pushDomino(x+1, y, 1))
      DTA_4(x, y);
    return;
  }

  if (getDominoType(x+1, y) == DominoTypeDelay)
  {
    if (getDominoState(x+1, y) != 8) {
      DTA_4(x, y);
      return;
    }
  }

  if (getDominoType(x+1, y) == DominoTypeCounter1 ||
      getDominoType(x+1, y) == DominoTypeCounter2 ||
      getDominoType(x+1, y) == DominoTypeCounter3
     )
  {
    if (!CounterStopper(getDominoType(x+1, y)))
      if (pushDomino(x+1, y, 1))
      {
        DTA_4(x, y);
        return;
      }
  }

  if (getDominoType(x-1, y) == DominoTypeSplitter && getDominoState(x-1, y) != 8)
  {
    if (pushDomino(x+1, y, 1))
      DTA_4(x, y);
    return;
  }

  if (getDominoType(x-1, y) != DominoTypeEmpty && getDominoState(x-1, y) > 8)
  {
    if (pushDomino(x+1, y, 1))
      DTA_4(x, y);
    return;
  }

  setDominoDir(x, y, -1);
  pushDomino(x+1, y, 1);
  soundSystem_c::instance()->startSound(soundSystem_c::SE_STOPPER);
  DTA_4(x, y);
}

// handle dominos crashing into something
void levelPlayer_c::DominoCrash(int x, int y, int type, int extra) {

  // what do we crash into?
  int next = getDominoType(x, y);

  // depending on what crashed we get a new pile

  // standard + standard     -> DominoCarsh3 little yellow pile
  // standard + DominoCrash3 -> DominoCarsh0 big yellow pile
  // DominoCrash3 + other    -> DominoCrash1 big mixed pile
  // blocker + blocker       -> DominoCrash5 little red pile
  // blocker + DominoCrash5  -> DominoCrash2 big red pile
  // ...
  //



  // for all combinations for participants do what's shown in the table above

  if (next == DominoTypeStandard)
  {
         if (type == DominoTypeStandard)                                                         next = DominoTypeCrash3;
    else if (type == DominoTypeCrash0 || type == DominoTypeCrash3)                               next = DominoTypeCrash0;
    else if (type < DominoTypeCrash0)                                                            next = DominoTypeCrash4;
    else                                                                                         next = DominoTypeCrash1;
  }
  else if (next == DominoTypeCrash3 || next == DominoTypeCrash0)
  {
         if (type == DominoTypeStandard || type == DominoTypeCrash0 || type == DominoTypeCrash3) next = DominoTypeCrash0;
    else                                                                                         next = DominoTypeCrash1;
  }
  else if (next == DominoTypeStopper)
  {
         if (type == DominoTypeStopper)                                                          next = DominoTypeCrash5;
    else if (type == DominoTypeCrash2 || type == DominoTypeCrash5)                               next = DominoTypeCrash2;
    else if (type < DominoTypeCrash0)                                                            next = DominoTypeCrash4;
    else                                                                                         next = DominoTypeCrash1;
  }
  else if (next == DominoTypeCrash5 || next == DominoTypeCrash2)
  {
         if (type == DominoTypeStopper || type == DominoTypeCrash2 || type == DominoTypeCrash5)  next = DominoTypeCrash2;
    else                                                                                         next = DominoTypeCrash1;
  }
  else if (next < DominoTypeCrash0)
  {
         if (type < DominoTypeCrash0)                                                            next = DominoTypeCrash4;
    else                                                                                         next = DominoTypeCrash1;
  }
  else
  {
    next = DominoTypeCrash1;
  }

  // set the resulting domino animation
  setDominoType(x, y, next);
  setDominoState(x, y, 1);
  setDominoDir(x, y, 1);
  if (getDominoExtra(x, y) == 0x70 || extra == 0x70 || getDominoExtra(x, y) == 0x60 || extra == 0x60)
  {
    setDominoExtra(x, y, 0x70);
    setDominoYOffset(x, y, getDominoYOffset(x, y) & 0xFC);
  }
  else
  {
    setDominoExtra(x, y, 0);
  }

  markDirty(x-1, y);
  markDirty(x, y);
  markDirty(x+1, y);

  soundSystem_c::instance()->startSound(soundSystem_c::SE_EXPLODER);
}

// vertical stone
void levelPlayer_c::DTA_E(int x, int y) {

  if (getDominoExtra(x, y) == 0x40)
  {
    setDominoExtra(x, y, 0);
  }

  // start falling to the side, once we hit the ground and the
  // domino had been given a direction
  if (getDominoExtra(x, y) != 0x70)
  {
    if (getDominoDir(x, y) != 0)
    {
      DTA_4(x, y);
    }
    return;
  }

  // continue falling down
  if (getDominoYOffset(x, y) != 0)
  {
    // if we have not yet reached the next level of the level data
    if (getDominoYOffset(x, y) != 12) {
      setDominoYOffset(x, y, getDominoYOffset(x, y)+4);
      markDirty(x, y-1);
      markDirty(x, y);
      return;
    }

    // if we have not yet fallen out of the level put
    // the domino into the next block below
    if (y < 12)
    {
      if (getDominoType(x, y+1) == DominoTypeEmpty)
      {
        setDominoType(x, y+1, getDominoType(x, y));
        setDominoState(x, y+1, getDominoState(x, y));
        setDominoDir(x, y+1, getDominoDir(x, y));
        setDominoYOffset(x, y+1, 0);
        setDominoExtra(x, y+1, 0x70);
      }
      else
      {
        DominoCrash(x, y+1, getDominoType(x, y), getDominoExtra(x, y));
        setDominoYOffset(x, y+1, 0);
        markDirty(x, y);
        markDirty(x, y+1);
      }
    }

    // remove the old domino
    removeDomino(x, y);

    markDirty(x, y+1);
    markDirty(x, y);
    markDirty(x, y-1);
    return;
  }

  // we have reached a special position where we need to check
  // for the ground again
  if (isTherePlatform(x, y))
  {
    // we still crash if there is a domino below us
    if (getDominoType(x, y+1) != DominoTypeEmpty &&
        getDominoType(x, y+1) != DominoTypeAscender
        )
    {
      DominoCrash(x, y+1, getDominoType(x, y), getDominoExtra(x, y));
    }

    if (getDominoType(x, y+1) == DominoTypeAscender)
    {
      if (getDominoState(x, y+1) == 1)
      {
        setDominoState(x-1, y+2, 14);
        setDominoType(x-1, y+2, DominoTypeAscender);
        setDominoDir(x-1, y+2, -1);
        setDominoExtra(x-1, y+2, 0x00);
        setDominoYOffset(x-1, y+2, getDominoYOffset(x, y+1)-16);
        removeDomino(x, y+1);
      }
      if (getDominoState(x, y+1) == 15)
      {
        setDominoState(x+1, y+2, 2);
        setDominoType(x+1, y+2, DominoTypeAscender);
        setDominoDir(x+1, y+2, 1);
        setDominoExtra(x+1, y+2, 0x00);
        setDominoYOffset(x+1, y+2, getDominoYOffset(x, y+1)-16);
        removeDomino(x, y+1);
      }
    }

    // no more falling
    setDominoExtra(x, y, 0);

    // we need to set the yOffset properly for the halve steps
    if (getFg(x, y) == 8 || getFg(x, y) == 11)
    {
      setDominoYOffset(x, y, 8);
    }

    markDirty(x, y);
    markDirty(x, y+1);
    return;
  }

  // no ground below us, continue falling

  // we can continue, if there is either no domino or no more
  // level below us
  if (y >= 12 || getDominoType(x, y+1) == DominoTypeEmpty || getDominoYOffset(x, y+1) != 0)
  {
    setDominoYOffset(x, y, getDominoYOffset(x, y)+4);
    markDirty(x, y);
    markDirty(x, y+1);
    return;
  }

  // if there is no splitter below us, we crash
  if (getDominoType(x, y+1) != DominoTypeSplitter)
  {
    DominoCrash(x, y+1, getDominoType(x, y), getDominoExtra(x, y));

    removeDomino(x, y);

    markDirty(x, y);
    markDirty(x, y+1);

    return;
  }

  // there is a splitter below us, so start that splitter
  // and we vanish
  pushDomino(x, y+1, -1);

  setDominoExtra(x, y+1, getDominoType(x, y));
  removeDomino(x, y);

  markDirty(x, y);
  markDirty(x, y+1);
}

// splitter parts falling further
void levelPlayer_c::DTA_C(int x, int y) {

  // this table contains the positions of the 2 splitter halves
  // for each splitter state, 8 = vertical, 1 = horizontal
  static const int SplitterLookup[] = {
    0, 0,
    1, 1,
    2, 2,
    3, 3,
    4, 4,
    5, 5,
    6, 6,
    7, 7,
    8, 8,
    2, 1,
    1, 2,
    3, 1,
    1, 3,
    2, 3,
    3, 2
  };

  // first find the positions of the 2 halves for the
  // current state
  int a = SplitterLookup[getDominoState(x, y)*2+1];
  int b = SplitterLookup[getDominoState(x, y)*2];

  // calculate the new position of the left halve
  if (a == 3)
  {
    // left halve is at a pushing place, check if we hit the wall
    if (x > 0 && getFg(x-1, y) != 0xA)
    {
      // no wall, check, if we hit a domino and if so try to push it
      if (getDominoType(x-1, y) == DominoTypeEmpty)
      {
        a--;
      }
      else if (pushDomino(x-1, y, -1))
      {
        a--;
      }
    }
  }
  else if (a == 2 && getDominoType(x-1, y) == 0)
  {
    a--;
  }

  // same as above but for right halve
  if (b == 3)
  {
    if (x < 19)
    {
      if (getDominoType(x+1, y) == DominoTypeEmpty)
      {
        b--;
      }
      else if (pushDomino(x+1, y, 1))
      {
        b--;
      }
    }
  }
  else if (b == 2 && getDominoType(x+1, y) == 0)
  {
    b--;
  }

  // now find the new state of the splitter domino
  for (int i = 0; i < 15; i++)
  {
    if (SplitterLookup[2*i+1] == a && SplitterLookup[2*i] == b)
    {
      if (getDominoState(x, y) != i)
      {
        setDominoState(x, y, i);
        markDirty(x, y);
        markDirty(x+1, y);
        markDirty(x-1, y);
        markDirty(x, y-1);
        markDirty(x-1, y-1);
        markDirty(x+1, y-1);
      }
      return;
    }
  }

  // this must not happen...
  printf("oops missing splitter image");
}

// bridger left this is mainly a lot of ifs to
// find out the new level elements that need to
// be placed
void levelPlayer_c::DTA_7(int x, int y) {

  int fg2;

  if (x >= 2)
  {
    fg2 = getFg(x-2, y);
  }
  else
  {
    fg2 = 0;
  }

  int fg1;

  if (x >= 1)
  {
    fg1 = getFg(x-1, y);
  }
  else
  {
    fg1 = 0;
  }

  int fg = getFg(x, y);

  if (fg != FgElementPlatformStart && fg != FgElementPlatformStrip)
  {
    DTA_1(x, y);
    return;
  }

  int doit = 0;

  if (fg1 == FgElementPlatformEnd)
  {
    fg1 = FgElementPlatformMiddle;
    if (fg == FgElementPlatformStart)
    {
      fg = FgElementPlatformMiddle;
    }
    else
    {
      fg = FgElementPlatformEnd;
    }
    doit = 1;
  }
  else if (fg1 != FgElementEmpty || fg2 != FgElementPlatformEnd)
  {
    if (fg1 == FgElementPlatformStrip)
    {
      fg1 = FgElementPlatformStart;
      if (fg == FgElementPlatformStart)
      {
        fg = FgElementPlatformMiddle;
      }
      else
      {
        fg = FgElementPlatformEnd;
      }
      doit = 1;
    }
    else
    {
      if (fg1 == FgElementEmpty && fg2 == FgElementPlatformStrip)
      {
        fg2 = FgElementPlatformStart;
        fg1 = FgElementPlatformMiddle;
        if (fg == FgElementPlatformStart)
        {
          fg = FgElementPlatformMiddle;
        }
        else
        {
          fg = FgElementPlatformEnd;
        }
        doit = 1;
      }
    }
  }
  else
  {
    fg2 = FgElementPlatformMiddle;
    fg1 = FgElementPlatformMiddle;
    if (fg == FgElementPlatformStart)
    {
      fg = FgElementPlatformMiddle;
    }
    else
    {
      fg = FgElementPlatformEnd;
    }
    doit = 1;
  }

  // if we can't build a bridge, we continue falling, maybe topple over and
  // continue downwards
  if (doit == 0)
  {
    DTA_1(x, y);
    return;
  }

  if (x >= 2)
  {
    setFg(x-2, y, fg2);
    markDirty(x-2, y);
  }

  if (x >= 1)
  {
    setFg(x-1, y, fg1);
  }

  setFg(x, y, fg);
  removeDomino(x, y);
  markDirty(x, y-1);
  markDirty(x-1, y-1);

  markDirtyBg(x-2, y);
  markDirtyBg(x-1, y);
  markDirtyBg(x  , y);
}

// Bridger right same as DTA_7 but for other direction
void levelPlayer_c::DTA_M(int x, int y) {

  int fg2;

  if (x < 18)
  {
    fg2 = getFg(x+2, y);
  }
  else
  {
    fg2 = 0;
  }

  int fg1;

  if (x < 19)
  {
    fg1 = getFg(x+1, y);
  }
  else
  {
    fg1 = 0;
  }

  int fg = getFg(x, y);

  if (fg != FgElementPlatformEnd && fg != FgElementPlatformStrip)
  {
    DTA_K(x, y);
    return;
  }

  int doit = 0;

  if (fg1 == FgElementPlatformStart)
  {
    if (fg == FgElementPlatformEnd)
    {
      fg = FgElementPlatformMiddle;
    }
    else
    {
      fg = FgElementPlatformStart;
    }
    fg1 = FgElementPlatformMiddle;
    doit = 1;
  }
  else if (fg1 != FgElementEmpty || fg2 != FgElementPlatformStart)
  {
    if (fg1 == FgElementPlatformStrip)
    {
      fg1 = FgElementPlatformStart;
      if (fg == FgElementPlatformEnd)
      {
        fg = FgElementPlatformMiddle;
      }
      else
      {
        fg = FgElementPlatformStart;
      }
      doit = 1;
    }
    else
    {
      if (fg1 == FgElementEmpty && fg2 == FgElementPlatformStrip)
      {
        fg2 = FgElementPlatformEnd;
        fg1 = FgElementPlatformMiddle;
        if (fg == FgElementPlatformEnd)
        {
          fg = FgElementPlatformMiddle;
        }
        else
        {
          fg = FgElementPlatformStart;
        }
        doit = 1;
      }
    }
  }
  else
  {
    fg2 = FgElementPlatformMiddle;
    fg1 = FgElementPlatformMiddle;
    if (fg == FgElementPlatformEnd)
    {
      fg = FgElementPlatformMiddle;
    }
    else
    {
      fg = FgElementPlatformStart;
    }
    doit = 1;
  }

  if (doit == 0)
  {
    DTA_K(x, y);
    return;
  }

  if (x < 18)
  {
    setFg(x+2, y, fg2);
    markDirty(x+2, y);
  }

  if (x < 19)
  {
    setFg(x+1, y, fg1);
  }

  setFg(x, y, fg);
  removeDomino(x, y);

  markDirty(x, y-1);
  markDirty(x+1, y-1);

  markDirtyBg(x  , y);
  markDirtyBg(x+1, y);
  markDirtyBg(x+2, y);
}


// riser
void levelPlayer_c::DTA_A(int x, int y) {

  int a;

  if (getDominoExtra(x, y) == 0x50)
    a = 1;
  else
    a = 2;

  int b;
  if (x > 0)
    b = getFg(x-1, y-a);
  else
    b = FgElementEmpty;

  int c = getFg(x, y-a);

  if ((c == FgElementPlatformStart || c == FgElementPlatformStrip) &&
      b == FgElementEmpty)
  {
    if (a == 1)
    {
      if (getDominoType(x-1, y) == DominoTypeEmpty)
      {
        setDominoExtra(x-1, y, 0x60);
        setDominoType(x-1, y, DominoTypeAscender);
        setDominoState(x-1, y, 14);
        setDominoDir(x-1, y, -1);
        setDominoYOffset(x-1, y, getDominoYOffset(x, y)-2);
      }
      else
      {
        DominoCrash(x-1, y, getDominoType(x, y), getDominoExtra(x, y));

        markDirty(x-2, y-2);
        markDirty(x-1, y-2);
        markDirty(x-2, y-1);
        markDirty(x-1, y-1);
      }
    }
    else
    {
      if (y > 0)
      {
        if (getDominoType(x-1, y-1) == DominoTypeEmpty)
        {
          setDominoExtra(x-1, y-1, 0x60);
          setDominoType(x-1, y-1, DominoTypeAscender);
          setDominoState(x-1, y-1, 14);
          setDominoDir(x-1, y-1, -1);
          setDominoYOffset(x-1, y-1, getDominoYOffset(x, y)+14);
        }
        else
        {
          DominoCrash(x-1, y-1, getDominoType(x, y), getDominoExtra(x, y));

          markDirty(x-2, y-3);
          markDirty(x-1, y-3);
          markDirty(x-2, y-2);
          markDirty(x-1, y-2);
        }
      }
    }

    removeDomino(x, y);

    markDirty(x, y-a);
    markDirty(x-1, y-a);
    markDirty(x, y-a+1);
    markDirty(x-1, y-a+1);

    return;
  }

  if (c == 0)
  {
    removeDomino(x, y);

    if (getDominoType(x, y+1-a) == DominoTypeEmpty)
    {
      setDominoExtra(x, y+1-a, 0x60);
      setDominoType(x, y+1-a, DominoTypeAscender);
      setDominoState(x, y+1-a, 8);
      setDominoDir(x, y+1-a, -1);
      setDominoYOffset(x, y+1-a, 0);

      markDirty(x, y-a);
      markDirty(x-1, y-a);
      markDirty(x, y-a+1);
      markDirty(x-1, y-a+1);
    }
    else
    {
      DominoCrash(x, y+1-a, getDominoType(x, y), getDominoExtra(x, y));

      markDirty(x-1, y-1-a);
      markDirty(x, y-1-a);
      markDirty(x-1, y-a);
      markDirty(x, y-a);
    }

    return;
  }

  if (getDominoExtra(x, y) != 0x50)
  {
    if (getDominoType(x, y-1) == DominoTypeEmpty)
    {
      setDominoType(x, y-1, getDominoType(x, y));
      setDominoState(x, y-1, getDominoState(x, y));
      setDominoDir(x, y-1, getDominoDir(x, y));
      setDominoYOffset(x, y-1, getDominoYOffset(x, y)+16);
      setDominoExtra(x, y-1, 0x50);
    }
    else
    {
      DominoCrash(x, y-1, getDominoType(x, y), getDominoExtra(x, y));

      markDirty(x-1, y-3);
      markDirty(x, y-3);
      markDirty(x-1, y-2);
      markDirty(x, y-2);
    }

    removeDomino(x, y);
  }
}


// Riser
void levelPlayer_c::DTA_O(int x, int y) {

  int a;

  if (getDominoExtra(x, y) == 0x50)
    a = 1;
  else
    a = 2;

  int b;
  if (x < 19)
    b = getFg(x+1, y-a);
  else
    b = FgElementEmpty;

  int c = getFg(x, y-a);

  if ((c == FgElementPlatformEnd || c == FgElementPlatformStrip) &&
      b == FgElementEmpty)
  {
    if (a == 1)
    {
      if (getDominoType(x+1, y) == DominoTypeEmpty)
      {
        setDominoExtra(x+1, y, 0x60);
        setDominoType(x+1, y, DominoTypeAscender);
        setDominoState(x+1, y, 2);
        setDominoDir(x+1, y, 1);
        setDominoYOffset(x+1, y, getDominoYOffset(x, y)-2);
      }
      else
      {
        DominoCrash(x+1, y, getDominoType(x, y), getDominoExtra(x, y));

        markDirty(x, y-2);
        markDirty(x+1, y-2);
        markDirty(x, y-1);
        markDirty(x+1, y-1);
      }
    }
    else
    {
      if (y > 0)
      {
        if (getDominoType(x+1, y-1) == DominoTypeEmpty)
        {
          setDominoExtra(x+1, y-1, 0x60);
          setDominoType(x+1, y-1, DominoTypeAscender);
          setDominoState(x+1, y-1, 2);
          setDominoDir(x+1, y-1, 1);
          setDominoYOffset(x+1, y-1, getDominoYOffset(x, y)+14);
        }
        else
        {
          DominoCrash(x+1, y-1, getDominoType(x, y), getDominoExtra(x, y));

          markDirty(x, y-3);
          markDirty(x+1, y-3);
          markDirty(x, y-2);
          markDirty(x+1, y-2);
        }
      }
    }

    removeDomino(x, y);

    markDirty(x, y-a);
    markDirty(x+1, y-a);
    markDirty(x, y-a+1);
    markDirty(x+1, y-a+1);

    return;
  }

  if (c == 0)
  {
    removeDomino(x, y);

    if (getDominoType(x, y+1-a) == DominoTypeEmpty)
    {
      setDominoExtra(x, y+1-a, 0x60);
      setDominoType(x, y+1-a, DominoTypeAscender);
      setDominoState(x, y+1-a, 8);
      setDominoDir(x, y+1-a, 1);
      setDominoYOffset(x, y+1-a, 0);
    }
    else
    {
      DominoCrash(x, y+1-a, getDominoType(x, y), getDominoExtra(x, y));

      markDirty(x-1, y-1-a);
      markDirty(x, y-1-a);
      markDirty(x-1, y-a);
      markDirty(x, y-a);
    }

    markDirty(x, y-a);
    markDirty(x+1, y-a);
    markDirty(x, y-a+1);
    markDirty(x+1, y-a+1);

    return;
  }

  if (getDominoExtra(x, y) != 0x50)
  {
    if (getDominoType(x, y-1) == DominoTypeEmpty)
    {
      setDominoType(x, y-1, getDominoType(x, y));
      setDominoState(x, y-1, getDominoState(x, y));
      setDominoDir(x, y-1, getDominoDir(x, y));
      setDominoYOffset(x, y-1, getDominoYOffset(x, y)+16);
      setDominoExtra(x, y-1, 0x50);
    }
    else
    {
      DominoCrash(x, y-1, getDominoType(x, y), getDominoExtra(x, y));

      markDirty(x-1, y-3);
      markDirty(x, y-3);
      markDirty(x-1, y-2);
      markDirty(x, y-2);
    }

    removeDomino(x, y);
  }
}

// riser rising vertically
void levelPlayer_c::DTA_H(int x, int y) {

  int riserDir = getDominoDir(x, y);

  // raiser is raising
  if (getDominoExtra(x, y) == 0x60)
  {
    if (getDominoYOffset(x, y) == 4 && y > 1)
    {
      if (isTherePlatform(x, y-2))
      {
        if (getFg(x, y-1) == 0xA || getFg(x, y-1) == 0xD)
        {
          setDominoState(x, y, 16);
          setDominoExtra(x, y, 0x50);

          markDirty(x, y);
          markDirty(x, y-2);
          markDirty(x, y-1);
          return;
        }
      }
    }
    if (getDominoYOffset(x, y) == -6 && y > 1)
    {
      if (isTherePlatform(x, y-2))
      {
        if (getFg(x, y-1) == 9 || getFg(x, y-1) == 0xE)
        {
          setDominoState(x, y, 16);
          setDominoExtra(x, y, 0x50);
          markDirty(x, y);
          markDirty(x, y-2);
          markDirty(x, y-1);
          return;
        }
      }
    }
    if (getDominoYOffset(x, y) == -10)
    {
      if (y > 1)
      {
        if (isTherePlatform(x, y-2))
        {
          setDominoState(x, y, 16);
          setDominoExtra(x, y, 0);

          markDirty(x, y);
          markDirty(x, y-2);
          markDirty(x, y-1);
          return;
        }
      }

      if (y > 0)
      {
        if (getDominoType(x, y-1) == DominoTypeEmpty)
        {
          setDominoExtra(x, y-1, 0x60);
          setDominoYOffset(x, y-1, 4);
          setDominoDir(x, y-1, getDominoDir(x, y));
          setDominoState(x, y-1, 8);
          setDominoType(x, y-1, DominoTypeAscender);
        }
        else
        {
          DominoCrash(x, y-1, getDominoType(x, y), getDominoExtra(x, y));
          markDirty(x, y-3);
        }
      }

      removeDomino(x, y);

      markDirty(x, y);
      markDirty(x, y-2);
      markDirty(x, y-1);
      return;
    }

    if (getDominoYOffset(x, y) == -8 && y > 1)
    {
      if (isTherePlatform(x, y-2))
      {
        setDominoState(x, y, 16);
      }
    }

    if (getDominoYOffset(x, y) == -6 && y > 1)
    {
      if (isTherePlatform(x, y-2))
      {
        setDominoState(x, y, 17);
      }
    }

    // normally raise the domino by 2 units
    setDominoYOffset(x, y, getDominoYOffset(x, y)-2);

    markDirty(x, y);
    markDirty(x, y-2);
    markDirty(x, y-1);
    return;
  }

  // if the raiser had been pushed, set it loose
  if (riserDir != 0)
  {
    // first reset to middle position
    if (getDominoState(x, y) == 16)
      setDominoState(x, y, 8);

    // then move it
    DTA_4(x, y);
  }
}

// Stone completely fallen down right used for
// standard, Trigger, Delay, Bridger
void levelPlayer_c::DTA_K(int x, int y) {
  int fg;

  if (x < 19)
    fg = getFg(x+1, y);
  else
    fg = 0;

  if (fg == FgElementLadder)
    fg = 0;

  if ((getFg(x, y) == FgElementPlatformEnd ||
        getFg(x, y) == FgElementPlatformStrip)
      &&
      fg == FgElementEmpty
     )
  {

    if (getDominoType(x+1, y+1) != DominoTypeEmpty)
    {
      DominoCrash(x+1, y+1, getDominoType(x, y), getDominoExtra(x, y));
      removeDomino(x, y);

      markDirty(x, y-1);
      markDirty(x+1, y-1);
      markDirty(x, y);
      markDirty(x+1, y);
    }
    else
    {
      setDominoType(x+1, y, getDominoType(x, y));
      setDominoState(x+1, y, 2);
      setDominoDir(x+1, y, getDominoDir(x, y));
      setDominoYOffset(x+1, y, 2);
      setDominoExtra(x+1, y, 0x70);

      soundSystem_c::instance()->startSound(soundSystem_c::SE_ASCENDER);

      removeDomino(x, y);

      markDirty(x, y-1);
      markDirty(x+1, y-1);
      markDirty(x, y);
      markDirty(x+1, y);
    }
  }
  else
  {
    if (x < 19 && getFg(x, y) == FgElementPlatformStep1 && fg == FgElementPlatformStep2)
    {
      if (getDominoType(x+1, y) != DominoTypeEmpty)
      {
        DominoCrash(x+1, y, getDominoType(x, y), getDominoExtra(x, y));
      }
      else
      {
        setDominoType(x+1, y, getDominoType(x, y));
        setDominoState(x+1, y, 2);
        setDominoDir(x+1, y, getDominoDir(x, y));
        setDominoYOffset(x+1, y, 2);
        setDominoExtra(x+1, y, 0x40);
      }

      removeDomino(x, y);

      markDirty(x, y-1);
      markDirty(x+1, y-1);
      markDirty(x, y);
      markDirty(x+1, y);

      return;
    }

    if (x < 19 && y < 11 && getFg(x, y) == FgElementPlatformStep2)
    {
      if (getDominoType(x+1, y+1) != 0)
      {
        DominoCrash(x+1, y+1, getDominoType(x, y), getDominoExtra(x, y));
      }
      else
      {
        setDominoType(x+1, y+1, getDominoType(x, y));
        setDominoState(x+1, y+1, 2);
        setDominoDir(x+1, y+1, getDominoDir(x, y));
        setDominoYOffset(x+1, y+1, -6);
        setDominoExtra(x+1, y+1, 0x40);
      }

      removeDomino(x, y);

      markDirty(x, y-1);
      markDirty(x+1, y-1);
      markDirty(x, y);
      markDirty(x+1, y);

      return;
    }

    setDominoDir(x, y, 0);
    return;
  }

}

// Stone completely fallen down left used for
// standard, Trigger, Delay, Bridger
void levelPlayer_c::DTA_1(int x, int y) {

  int fg;

  if (x > 0)
    fg = getFg(x-1, y);
  else
    fg = 0;

  if (fg == FgElementLadder)
    fg = 0;

  if ((getFg(x, y) == FgElementPlatformStart ||
        getFg(x, y) == FgElementPlatformStrip)
      &&
      fg == FgElementEmpty
     )
  {

    if (getDominoType(x-1, y+1) != DominoTypeEmpty)
    {
      DominoCrash(x-1, y+1, getDominoType(x, y), getDominoExtra(x, y));
      removeDomino(x, y);

      markDirty(x, y-1);
      markDirty(x-1, y-1);
      markDirty(x, y);
      markDirty(x-1, y);

      return;
    }
    else
    {
      setDominoType(x-1, y, getDominoType(x, y));
      setDominoState(x-1, y, 0xE);
      setDominoDir(x-1, y, getDominoDir(x, y));
      setDominoYOffset(x-1, y, 2);
      setDominoExtra(x-1, y, 0x70);

      soundSystem_c::instance()->startSound(soundSystem_c::SE_ASCENDER);

      removeDomino(x, y);

      markDirty(x, y-1);
      markDirty(x-1, y-1);
      markDirty(x, y);
      markDirty(x-1, y);
    }
  }
  else
  {
    if (x > 0 && getFg(x, y) == FgElementPlatformStep6 && fg == FgElementPlatformStep5)
    {
      if (getDominoType(x-1, y) != DominoTypeEmpty)
      {
        DominoCrash(x-1, y, getDominoType(x, y), getDominoExtra(x, y));
      }
      else
      {
        setDominoType(x-1, y, getDominoType(x, y));
        setDominoState(x-1, y, 0xE);
        setDominoDir(x-1, y, getDominoDir(x, y));
        setDominoYOffset(x-1, y, 2);
        setDominoExtra(x-1, y, 0x40);
      }

      removeDomino(x, y);

      markDirty(x, y-1);
      markDirty(x-1, y-1);
      markDirty(x, y);
      markDirty(x-1, y);

      return;
    }

    if (x > 0 && y < 11 && getFg(x, y) == FgElementPlatformStep5)
    {
      if (getDominoType(x-1, y+1) != 0)
      {
        DominoCrash(x-1, y+1, getDominoType(x, y), getDominoExtra(x, y));
      }
      else
      {
        setDominoType(x-1, y+1, getDominoType(x, y));
        setDominoState(x-1, y+1, 0xE);
        setDominoDir(x-1, y+1, getDominoDir(x, y));
        setDominoYOffset(x-1, y+1, -6);
        setDominoExtra(x-1, y+1, 0x40);
      }

      removeDomino(x, y);

      markDirty(x, y-1);
      markDirty(x-1, y-1);
      markDirty(x, y);
      markDirty(x-1, y);

      return;
    }

    setDominoDir(x, y, 0);
    return;
  }

}

// Tumbler fallen down left
void levelPlayer_c::DTA_6(int x, int y) {

  int fg;

  if (x > 0)
    fg = getFg(x-1, y);
  else
    fg = 0;

  if (fg == FgElementLadder)
    fg = 0;

  if ((getFg(x, y) == FgElementPlatformStart || getFg(x, y) == FgElementPlatformStrip) && fg == 0)
  {
    if (getDominoType(x-1, y+1) != 0)
    {
      DominoCrash(x-1, y+1, getDominoType(x, y), getDominoExtra(x, y));
    }
    else
    {
      setDominoType(x-1, y, getDominoType(x, y));
      setDominoState(x-1, y, 14);
      setDominoDir(x-1, y, getDominoDir(x, y));
      setDominoYOffset(x-1, y, 2);
      setDominoExtra(x-1, y, 0x70);

      soundSystem_c::instance()->startSound(soundSystem_c::SE_ASCENDER);
    }

    removeDomino(x, y);

    markDirty(x, y);
    markDirty(x-1, y);
    markDirty(x, y-1);
    markDirty(x-1, y-1);

    return;
  }

  if (x > 0 && getFg(x, y) == FgElementPlatformStep6 && fg == FgElementPlatformStep5)
  {
    if (getDominoType(x-1, y) != 0)
    {
      DominoCrash(x-1, y, getDominoType(x, y), getDominoExtra(x, y));
    }
    else
    {
      setDominoType(x-1, y, getDominoType(x, y));
      setDominoState(x-1, y, 14);
      setDominoDir(x-1, y, getDominoDir(x, y));
      setDominoYOffset(x-1, y, 2);
      setDominoExtra(x-1, y, 0x40);
    }

    removeDomino(x, y);

    markDirty(x, y);
    markDirty(x-1, y);
    markDirty(x, y-1);
    markDirty(x-1, y-1);

    return;
  }

  if (x > 0 && y < 11 && getFg(x, y) == FgElementPlatformStep5)
  {
    if (getDominoType(x-1, y+1) != 0)
    {
      DominoCrash(x-1, y+1, getDominoType(x, y), getDominoExtra(x, y));
    }
    else
    {
      setDominoType(x-1, y+1, getDominoType(x, y));
      setDominoState(x-1, y+1, 14);
      setDominoDir(x-1, y+1, getDominoDir(x, y));
      setDominoYOffset(x-1, y+1, -6);
      setDominoExtra(x-1, y+1, 0x40);
    }

    removeDomino(x, y);

    markDirty(x, y);
    markDirty(x-1, y);
    markDirty(x, y-1);
    markDirty(x-1, y-1);

    return;
  }

  if (getDominoType(x-1, y) != 0)
  {
    DominoCrash(x-1, y, getDominoType(x, y), getDominoExtra(x, y));
  }
  else
  {
    setDominoType(x-1, y, DominoTypeTumbler);
    setDominoState(x-1, y, 14);
    setDominoDir(x-1, y, getDominoDir(x, y));
    setDominoYOffset(x-1, y, 0);
  }

  removeDomino(x, y);

  markDirty(x, y);
  markDirty(x-1, y);
  markDirty(x, y-1);
  markDirty(x-1, y-1);
}

// Tumbler fallen down right
void levelPlayer_c::DTA_L(int x, int y) {

  int fg;

  if (x < 19)
    fg = getFg(x+1, y);
  else
    fg = 0;

  if (fg == FgElementLadder)
    fg = 0;

  if ((getFg(x, y) == FgElementPlatformEnd || getFg(x, y) == FgElementPlatformStrip) && fg == 0)
  {
    if (getDominoType(x+1, y+1) != 0)
    {
      DominoCrash(x+1, y+1, getDominoType(x, y), getDominoExtra(x, y));
    }
    else
    {
      setDominoType(x+1, y, getDominoType(x, y));
      setDominoState(x+1, y, 2);
      setDominoDir(x+1, y, getDominoDir(x, y));
      setDominoYOffset(x+1, y, 2);
      setDominoExtra(x+1, y, 0x70);

      soundSystem_c::instance()->startSound(soundSystem_c::SE_ASCENDER);
    }

    removeDomino(x, y);

    markDirty(x, y);
    markDirty(x+1, y);
    markDirty(x, y-1);
    markDirty(x+1, y-1);

    return;
  }

  if (x < 19 && getFg(x, y) == FgElementPlatformStep1 && fg == FgElementPlatformStep2)
  {
    if (getDominoType(x+1, y) != 0)
    {
      DominoCrash(x+1, y, getDominoType(x, y), getDominoExtra(x, y));
    }
    else
    {
      setDominoType(x+1, y, getDominoType(x, y));
      setDominoState(x+1, y, 2);
      setDominoDir(x+1, y, getDominoDir(x, y));
      setDominoYOffset(x+1, y, 2);
      setDominoExtra(x+1, y, 0x40);
    }

    removeDomino(x, y);

    markDirty(x, y);
    markDirty(x+1, y);
    markDirty(x, y-1);
    markDirty(x+1, y-1);

    return;
  }

  if (x < 19 && y < 11 && getFg(x, y) == FgElementPlatformStep2)
  {
    if (getDominoType(x+1, y+1) != 0)
    {
      DominoCrash(x+1, y+1, getDominoType(x, y), getDominoExtra(x, y));
    }
    else
    {
      setDominoType(x+1, y+1, getDominoType(x, y));
      setDominoState(x+1, y+1, 2);
      setDominoDir(x+1, y+1, getDominoDir(x, y));
      setDominoYOffset(x+1, y+1, -6);
      setDominoExtra(x+1, y+1, 0x40);
    }

    removeDomino(x, y);

    markDirty(x, y);
    markDirty(x+1, y);
    markDirty(x, y-1);
    markDirty(x+1, y-1);

    return;
  }

  if (getDominoType(x+1, y) != 0)
  {
    DominoCrash(x+1, y, getDominoType(x, y), getDominoExtra(x, y));
  }
  else
  {
    setDominoType(x+1, y, DominoTypeTumbler);
    setDominoState(x+1, y, 2);
    setDominoDir(x+1, y, getDominoDir(x, y));
  }

  removeDomino(x, y);

  markDirty(x, y);
  markDirty(x+1, y);
  markDirty(x, y-1);
  markDirty(x+1, y-1);
}


bool levelPlayer_c::CounterStopper(int num) {
  for (int yp = 0; yp < 13; yp++)
    for (int xp = 0; xp < 20; xp++)
      if (  (getDominoType(xp, yp) > num)
          &&(getDominoType(xp, yp) <= DominoTypeCounter3)
          &&(getDominoState(xp, yp) == 8)
          &&(getDominoDir(xp, yp) == 0)
         )
      {
        return true;
      }

  return false;
}


// counter stone
void levelPlayer_c::DTA_P(int x, int y) {

  int dir = getDominoDir(x, y);

  if (getDominoDir(x, y) != 0)
  {
    // The domino has been pushed, check if there is an other one with a higher priority, if so
    // remove the direction temorarily so that it keeps standing

    if (CounterStopper(getDominoType(x, y)))
    {
      setDominoDir(x, y, 0);
    }
  }

  DTA_E(x, y);

  setDominoDir(x, y, dir);

}

void levelPlayer_c::callStateFunction(int type, int state, int x, int y) {

  switch ((type-1)*17+state-1) {

    // DominoTypeStandard
    case 17*(DominoTypeStandard-1) +   0: DTA_1(x, y); break;
    case 17*(DominoTypeStandard-1) +   1: DTA_2(x, y); break;
    case 17*(DominoTypeStandard-1) +   2: DTA_3(x, y); break;
    case 17*(DominoTypeStandard-1) +   3: DTA_4(x, y); break;
    case 17*(DominoTypeStandard-1) +   4: DTA_4(x, y); break;
    case 17*(DominoTypeStandard-1) +   5: DTA_4(x, y); break;
    case 17*(DominoTypeStandard-1) +   6: DTA_4(x, y); break;
    case 17*(DominoTypeStandard-1) +   7: DTA_E(x, y); break;
    case 17*(DominoTypeStandard-1) +   8: DTA_4(x, y); break;
    case 17*(DominoTypeStandard-1) +   9: DTA_4(x, y); break;
    case 17*(DominoTypeStandard-1) +  10: DTA_4(x, y); break;
    case 17*(DominoTypeStandard-1) +  11: DTA_4(x, y); break;
    case 17*(DominoTypeStandard-1) +  12: DTA_I(x, y); break;
    case 17*(DominoTypeStandard-1) +  13: DTA_J(x, y); break;
    case 17*(DominoTypeStandard-1) +  14: DTA_K(x, y); break;

              // DominoTypeStopper
    case 17*(DominoTypeStopper -1) +   7: DTA_F(x, y); break;

              // DominoTypeSplitter
    case 17*(DominoTypeSplitter-1) +   1: DTA_C(x, y); break;
    case 17*(DominoTypeSplitter-1) +   2: DTA_C(x, y); break;
    case 17*(DominoTypeSplitter-1) +   3: DTA_D(x, y); break;
    case 17*(DominoTypeSplitter-1) +   4: DTA_D(x, y); break;
    case 17*(DominoTypeSplitter-1) +   5: DTA_D(x, y); break;
    case 17*(DominoTypeSplitter-1) +   6: DTA_D(x, y); break;
    case 17*(DominoTypeSplitter-1) +   7: DTA_F(x, y); break;
    case 17*(DominoTypeSplitter-1) +   8: DTA_C(x, y); break;
    case 17*(DominoTypeSplitter-1) +   9: DTA_C(x, y); break;
    case 17*(DominoTypeSplitter-1) +  10: DTA_C(x, y); break;
    case 17*(DominoTypeSplitter-1) +  11: DTA_C(x, y); break;
    case 17*(DominoTypeSplitter-1) +  12: DTA_C(x, y); break;
    case 17*(DominoTypeSplitter-1) +  13: DTA_C(x, y); break;

              // DominoTypeExploder
    case 17*(DominoTypeExploder-1) +   0: DTA_5(x, y); break;
    case 17*(DominoTypeExploder-1) +   1: DTA_4(x, y); break;
    case 17*(DominoTypeExploder-1) +   2: DTA_4(x, y); break;
    case 17*(DominoTypeExploder-1) +   3: DTA_4(x, y); break;
    case 17*(DominoTypeExploder-1) +   4: DTA_4(x, y); break;
    case 17*(DominoTypeExploder-1) +   5: DTA_4(x, y); break;
    case 17*(DominoTypeExploder-1) +   6: DTA_4(x, y); break;
    case 17*(DominoTypeExploder-1) +   7: DTA_F(x, y); break;

              // DominoTypeDelay
    case 17*(DominoTypeDelay-1) +   0: DTA_1(x, y); break;
    case 17*(DominoTypeDelay-1) +   1: DTA_2(x, y); break;
    case 17*(DominoTypeDelay-1) +   2: DTA_3(x, y); break;
    case 17*(DominoTypeDelay-1) +   3: DTA_4(x, y); break;
    case 17*(DominoTypeDelay-1) +   4: DTA_4(x, y); break;
    case 17*(DominoTypeDelay-1) +   5: DTA_4(x, y); break;
    case 17*(DominoTypeDelay-1) +   6: DTA_4(x, y); break;
    case 17*(DominoTypeDelay-1) +   7: DTA_G(x, y); break;
    case 17*(DominoTypeDelay-1) +   8: DTA_4(x, y); break;
    case 17*(DominoTypeDelay-1) +   9: DTA_4(x, y); break;
    case 17*(DominoTypeDelay-1) +  10: DTA_4(x, y); break;
    case 17*(DominoTypeDelay-1) +  11: DTA_4(x, y); break;
    case 17*(DominoTypeDelay-1) +  12: DTA_I(x, y); break;
    case 17*(DominoTypeDelay-1) +  13: DTA_J(x, y); break;
    case 17*(DominoTypeDelay-1) +  14: DTA_K(x, y); break;

              // DominoTypeTumbler
    case 17*(DominoTypeTumbler-1) +   0: DTA_6(x, y); break;
    case 17*(DominoTypeTumbler-1) +   1: DTA_2(x, y); break;
    case 17*(DominoTypeTumbler-1) +   2: DTA_3(x, y); break;
    case 17*(DominoTypeTumbler-1) +   3: DTA_4(x, y); break;
    case 17*(DominoTypeTumbler-1) +   4: DTA_4(x, y); break;
    case 17*(DominoTypeTumbler-1) +   5: DTA_4(x, y); break;
    case 17*(DominoTypeTumbler-1) +   6: DTA_4(x, y); break;
    case 17*(DominoTypeTumbler-1) +   7: DTA_E(x, y); break;
    case 17*(DominoTypeTumbler-1) +   8: DTA_4(x, y); break;
    case 17*(DominoTypeTumbler-1) +   9: DTA_4(x, y); break;
    case 17*(DominoTypeTumbler-1) +  10: DTA_4(x, y); break;
    case 17*(DominoTypeTumbler-1) +  11: DTA_4(x, y); break;
    case 17*(DominoTypeTumbler-1) +  12: DTA_I(x, y); break;
    case 17*(DominoTypeTumbler-1) +  13: DTA_J(x, y); break;
    case 17*(DominoTypeTumbler-1) +  14: DTA_L(x, y); break;

              // DominoTypeBridger
    case 17*(DominoTypeBridger-1) +   0: DTA_7(x, y); break;
    case 17*(DominoTypeBridger-1) +   1: DTA_2(x, y); break;
    case 17*(DominoTypeBridger-1) +   2: DTA_3(x, y); break;
    case 17*(DominoTypeBridger-1) +   3: DTA_4(x, y); break;
    case 17*(DominoTypeBridger-1) +   4: DTA_4(x, y); break;
    case 17*(DominoTypeBridger-1) +   5: DTA_4(x, y); break;
    case 17*(DominoTypeBridger-1) +   6: DTA_4(x, y); break;
    case 17*(DominoTypeBridger-1) +   7: DTA_E(x, y); break;
    case 17*(DominoTypeBridger-1) +   8: DTA_4(x, y); break;
    case 17*(DominoTypeBridger-1) +   9: DTA_4(x, y); break;
    case 17*(DominoTypeBridger-1) +  10: DTA_4(x, y); break;
    case 17*(DominoTypeBridger-1) +  11: DTA_4(x, y); break;
    case 17*(DominoTypeBridger-1) +  12: DTA_I(x, y); break;
    case 17*(DominoTypeBridger-1) +  13: DTA_J(x, y); break;
    case 17*(DominoTypeBridger-1) +  14: DTA_M(x, y); break;

              // DominoTypeVanish
    case 17*(DominoTypeVanish-1) +   0: DTA_8(x, y); break;
    case 17*(DominoTypeVanish-1) +   1: DTA_4(x, y); break;
    case 17*(DominoTypeVanish-1) +   2: DTA_3(x, y); break;
    case 17*(DominoTypeVanish-1) +   3: DTA_4(x, y); break;
    case 17*(DominoTypeVanish-1) +   4: DTA_4(x, y); break;
    case 17*(DominoTypeVanish-1) +   5: DTA_4(x, y); break;
    case 17*(DominoTypeVanish-1) +   6: DTA_4(x, y); break;
    case 17*(DominoTypeVanish-1) +   7: DTA_E(x, y); break;
    case 17*(DominoTypeVanish-1) +   8: DTA_4(x, y); break;
    case 17*(DominoTypeVanish-1) +   9: DTA_4(x, y); break;
    case 17*(DominoTypeVanish-1) +  10: DTA_4(x, y); break;
    case 17*(DominoTypeVanish-1) +  11: DTA_4(x, y); break;
    case 17*(DominoTypeVanish-1) +  12: DTA_I(x, y); break;
    case 17*(DominoTypeVanish-1) +  13: DTA_4(x, y); break;
    case 17*(DominoTypeVanish-1) +  14: DTA_8(x, y); break;

              // DominoTypeTrigger
    case 17*(DominoTypeTrigger-1) +   0: DTA_9(x, y); break;
    case 17*(DominoTypeTrigger-1) +   1: DTA_2(x, y); break;
    case 17*(DominoTypeTrigger-1) +   2: DTA_3(x, y); break;
    case 17*(DominoTypeTrigger-1) +   3: DTA_4(x, y); break;
    case 17*(DominoTypeTrigger-1) +   4: DTA_4(x, y); break;
    case 17*(DominoTypeTrigger-1) +   5: DTA_4(x, y); break;
    case 17*(DominoTypeTrigger-1) +   6: DTA_4(x, y); break;
    case 17*(DominoTypeTrigger-1) +   7: DTA_E(x, y); break;
    case 17*(DominoTypeTrigger-1) +   8: DTA_4(x, y); break;
    case 17*(DominoTypeTrigger-1) +   9: DTA_4(x, y); break;
    case 17*(DominoTypeTrigger-1) +  10: DTA_4(x, y); break;
    case 17*(DominoTypeTrigger-1) +  11: DTA_4(x, y); break;
    case 17*(DominoTypeTrigger-1) +  12: DTA_I(x, y); break;
    case 17*(DominoTypeTrigger-1) +  13: DTA_J(x, y); break;
    case 17*(DominoTypeTrigger-1) +  14: DTA_N(x, y); break;

              // DominoTypeAscender
    case 17*(DominoTypeAscender-1) +   0: DTA_A(x, y); break;
    case 17*(DominoTypeAscender-1) +   1: DTA_4(x, y); break;
    case 17*(DominoTypeAscender-1) +   2: DTA_3(x, y); break;
    case 17*(DominoTypeAscender-1) +   3: DTA_4(x, y); break;
    case 17*(DominoTypeAscender-1) +   4: DTA_4(x, y); break;
    case 17*(DominoTypeAscender-1) +   5: DTA_4(x, y); break;
    case 17*(DominoTypeAscender-1) +   6: DTA_4(x, y); break;
    case 17*(DominoTypeAscender-1) +   7: DTA_H(x, y); break;
    case 17*(DominoTypeAscender-1) +   8: DTA_4(x, y); break;
    case 17*(DominoTypeAscender-1) +   9: DTA_4(x, y); break;
    case 17*(DominoTypeAscender-1) +  10: DTA_4(x, y); break;
    case 17*(DominoTypeAscender-1) +  11: DTA_4(x, y); break;
    case 17*(DominoTypeAscender-1) +  12: DTA_I(x, y); break;
    case 17*(DominoTypeAscender-1) +  13: DTA_4(x, y); break;
    case 17*(DominoTypeAscender-1) +  14: DTA_O(x, y); break;
    case 17*(DominoTypeAscender-1) +  15: DTA_H(x, y); break;
    case 17*(DominoTypeAscender-1) +  16: DTA_H(x, y); break;

              // DominoTypeConnectedA
    case 17*(DominoTypeConnectedA-1) +   0: DTA_1(x, y); break;
    case 17*(DominoTypeConnectedA-1) +   1: DTA_2(x, y); break;
    case 17*(DominoTypeConnectedA-1) +   2: DTA_3(x, y); break;
    case 17*(DominoTypeConnectedA-1) +   3: DTA_4(x, y); break;
    case 17*(DominoTypeConnectedA-1) +   4: DTA_4(x, y); break;
    case 17*(DominoTypeConnectedA-1) +   5: DTA_4(x, y); break;
    case 17*(DominoTypeConnectedA-1) +   6: DTA_4(x, y); break;
    case 17*(DominoTypeConnectedA-1) +   7: DTA_E(x, y); break;
    case 17*(DominoTypeConnectedA-1) +   8: DTA_4(x, y); break;
    case 17*(DominoTypeConnectedA-1) +   9: DTA_4(x, y); break;
    case 17*(DominoTypeConnectedA-1) +  10: DTA_4(x, y); break;
    case 17*(DominoTypeConnectedA-1) +  11: DTA_4(x, y); break;
    case 17*(DominoTypeConnectedA-1) +  12: DTA_I(x, y); break;
    case 17*(DominoTypeConnectedA-1) +  13: DTA_J(x, y); break;
    case 17*(DominoTypeConnectedA-1) +  14: DTA_K(x, y); break;

              // DominoTypeConnectedA
    case 17*(DominoTypeConnectedB-1) +   0: DTA_1(x, y); break;
    case 17*(DominoTypeConnectedB-1) +   1: DTA_2(x, y); break;
    case 17*(DominoTypeConnectedB-1) +   2: DTA_3(x, y); break;
    case 17*(DominoTypeConnectedB-1) +   3: DTA_4(x, y); break;
    case 17*(DominoTypeConnectedB-1) +   4: DTA_4(x, y); break;
    case 17*(DominoTypeConnectedB-1) +   5: DTA_4(x, y); break;
    case 17*(DominoTypeConnectedB-1) +   6: DTA_4(x, y); break;
    case 17*(DominoTypeConnectedB-1) +   7: DTA_E(x, y); break;
    case 17*(DominoTypeConnectedB-1) +   8: DTA_4(x, y); break;
    case 17*(DominoTypeConnectedB-1) +   9: DTA_4(x, y); break;
    case 17*(DominoTypeConnectedB-1) +  10: DTA_4(x, y); break;
    case 17*(DominoTypeConnectedB-1) +  11: DTA_4(x, y); break;
    case 17*(DominoTypeConnectedB-1) +  12: DTA_I(x, y); break;
    case 17*(DominoTypeConnectedB-1) +  13: DTA_J(x, y); break;
    case 17*(DominoTypeConnectedB-1) +  14: DTA_K(x, y); break;

              // DominoTypeCounter1
    case 17*(DominoTypeCounter1-1) +   0: DTA_1(x, y); break;
    case 17*(DominoTypeCounter1-1) +   1: DTA_2(x, y); break;
    case 17*(DominoTypeCounter1-1) +   2: DTA_3(x, y); break;
    case 17*(DominoTypeCounter1-1) +   3: DTA_4(x, y); break;
    case 17*(DominoTypeCounter1-1) +   4: DTA_4(x, y); break;
    case 17*(DominoTypeCounter1-1) +   5: DTA_4(x, y); break;
    case 17*(DominoTypeCounter1-1) +   6: DTA_4(x, y); break;
    case 17*(DominoTypeCounter1-1) +   7: DTA_P(x, y); break;
    case 17*(DominoTypeCounter1-1) +   8: DTA_4(x, y); break;
    case 17*(DominoTypeCounter1-1) +   9: DTA_4(x, y); break;
    case 17*(DominoTypeCounter1-1) +  10: DTA_4(x, y); break;
    case 17*(DominoTypeCounter1-1) +  11: DTA_4(x, y); break;
    case 17*(DominoTypeCounter1-1) +  12: DTA_I(x, y); break;
    case 17*(DominoTypeCounter1-1) +  13: DTA_J(x, y); break;
    case 17*(DominoTypeCounter1-1) +  14: DTA_K(x, y); break;

              // DominoTypeCounter2
    case 17*(DominoTypeCounter2-1) +   0: DTA_1(x, y); break;
    case 17*(DominoTypeCounter2-1) +   1: DTA_2(x, y); break;
    case 17*(DominoTypeCounter2-1) +   2: DTA_3(x, y); break;
    case 17*(DominoTypeCounter2-1) +   3: DTA_4(x, y); break;
    case 17*(DominoTypeCounter2-1) +   4: DTA_4(x, y); break;
    case 17*(DominoTypeCounter2-1) +   5: DTA_4(x, y); break;
    case 17*(DominoTypeCounter2-1) +   6: DTA_4(x, y); break;
    case 17*(DominoTypeCounter2-1) +   7: DTA_P(x, y); break;
    case 17*(DominoTypeCounter2-1) +   8: DTA_4(x, y); break;
    case 17*(DominoTypeCounter2-1) +   9: DTA_4(x, y); break;
    case 17*(DominoTypeCounter2-1) +  10: DTA_4(x, y); break;
    case 17*(DominoTypeCounter2-1) +  11: DTA_4(x, y); break;
    case 17*(DominoTypeCounter2-1) +  12: DTA_I(x, y); break;
    case 17*(DominoTypeCounter2-1) +  13: DTA_J(x, y); break;
    case 17*(DominoTypeCounter2-1) +  14: DTA_K(x, y); break;

              // DominoTypeCounter3
    case 17*(DominoTypeCounter3-1) +   0: DTA_1(x, y); break;
    case 17*(DominoTypeCounter3-1) +   1: DTA_2(x, y); break;
    case 17*(DominoTypeCounter3-1) +   2: DTA_3(x, y); break;
    case 17*(DominoTypeCounter3-1) +   3: DTA_4(x, y); break;
    case 17*(DominoTypeCounter3-1) +   4: DTA_4(x, y); break;
    case 17*(DominoTypeCounter3-1) +   5: DTA_4(x, y); break;
    case 17*(DominoTypeCounter3-1) +   6: DTA_4(x, y); break;
    case 17*(DominoTypeCounter3-1) +   7: DTA_P(x, y); break;
    case 17*(DominoTypeCounter3-1) +   8: DTA_4(x, y); break;
    case 17*(DominoTypeCounter3-1) +   9: DTA_4(x, y); break;
    case 17*(DominoTypeCounter3-1) +  10: DTA_4(x, y); break;
    case 17*(DominoTypeCounter3-1) +  11: DTA_4(x, y); break;
    case 17*(DominoTypeCounter3-1) +  12: DTA_I(x, y); break;
    case 17*(DominoTypeCounter3-1) +  13: DTA_J(x, y); break;
    case 17*(DominoTypeCounter3-1) +  14: DTA_K(x, y); break;

              // DominoTypeCrash0
    case 17*(DominoTypeCrash0-1) +   0: DTA_B(x, y); break;
    case 17*(DominoTypeCrash0-1) +   1: DTA_B(x, y); break;
    case 17*(DominoTypeCrash0-1) +   2: DTA_B(x, y); break;
    case 17*(DominoTypeCrash0-1) +   3: DTA_B(x, y); break;
    case 17*(DominoTypeCrash0-1) +   4: DTA_B(x, y); break;

              // DominoTypeCrash1
    case 17*(DominoTypeCrash1-1) +   0: DTA_B(x, y); break;
    case 17*(DominoTypeCrash1-1) +   1: DTA_B(x, y); break;
    case 17*(DominoTypeCrash1-1) +   2: DTA_B(x, y); break;
    case 17*(DominoTypeCrash1-1) +   3: DTA_B(x, y); break;
    case 17*(DominoTypeCrash1-1) +   4: DTA_B(x, y); break;

              // DominoTypeCrash2
    case 17*(DominoTypeCrash2-1) +   0: DTA_B(x, y); break;
    case 17*(DominoTypeCrash2-1) +   1: DTA_B(x, y); break;
    case 17*(DominoTypeCrash2-1) +   2: DTA_B(x, y); break;
    case 17*(DominoTypeCrash2-1) +   3: DTA_B(x, y); break;
    case 17*(DominoTypeCrash2-1) +   4: DTA_B(x, y); break;

              // DominoTypeCrash3
    case 17*(DominoTypeCrash3-1) +   0: DTA_B(x, y); break;
    case 17*(DominoTypeCrash3-1) +   1: DTA_B(x, y); break;
    case 17*(DominoTypeCrash3-1) +   2: DTA_B(x, y); break;
    case 17*(DominoTypeCrash3-1) +   3: DTA_B(x, y); break;
    case 17*(DominoTypeCrash3-1) +   4: DTA_B(x, y); break;

              // DominoTypeCrash4
    case 17*(DominoTypeCrash4-1) +   0: DTA_B(x, y); break;
    case 17*(DominoTypeCrash4-1) +   1: DTA_B(x, y); break;
    case 17*(DominoTypeCrash4-1) +   2: DTA_B(x, y); break;
    case 17*(DominoTypeCrash4-1) +   3: DTA_B(x, y); break;
    case 17*(DominoTypeCrash4-1) +   4: DTA_B(x, y); break;

              // DominoTypeCrash5
    case 17*(DominoTypeCrash5-1) +   0: DTA_B(x, y); break;
    case 17*(DominoTypeCrash5-1) +   1: DTA_B(x, y); break;
    case 17*(DominoTypeCrash5-1) +   2: DTA_B(x, y); break;
    case 17*(DominoTypeCrash5-1) +   3: DTA_B(x, y); break;
    case 17*(DominoTypeCrash5-1) +   4: DTA_B(x, y); break;
              // DominoTypeRiserCont
              // DominoTypeQuaver
  }
}

int levelPlayer_c::performDominos(ant_c & a) {

  a.performAnimation();

  inactive++;

  for (int y = 0; y < 13; y++)
    for (int x = 0; x < 20; x++)
      if (getDominoType(x, y) != DominoTypeEmpty &&
          getDominoState(x, y) != 0) {

        int oldState = getDominoState(x, y);
        int oldExtra = getDominoExtra(x, y);
        int oldYpos = getDominoYOffset(x, y);

        callStateFunction(getDominoType(x, y), getDominoState(x, y), x, y);

        if (oldState != getDominoState(x, y) || oldExtra != getDominoExtra(x, y) || oldYpos != getDominoYOffset(x, y))
          if (getDominoType(x, y) != DominoTypeTumbler)
            inactive = 0;

        if (getDominoType(x, y) == DominoTypeAscender)
        {
          if (isDirty(x-1, y)) markDirty(x-1, y-1);
          if (isDirty(x  , y)) markDirty(x  , y-1);
          if (isDirty(x+1, y)) markDirty(x+1, y-1);
        }
      }

  timeTick();

  int reason = 0;

  if (finishCheckDone && !levelCompleted(reason))
  {
    // we failed after all
    openExitDoor(false);

    return reason;
  }

  if (triggerIsFalln() && !finishCheckDone)
  {
    finishCheckDone = true;

    if (levelCompleted(reason) && !a.carrySomething() && a.isLiving())
    {
      a.success();
    }
    else
    {
      a.fail();

      if (reason)
        return reason;
      else if (a.carrySomething())
        return 4;
      else
        return 5;
    }
  }

  // if level is inactive for a longer time and no pushed are left
  if (a.getPushsLeft() == 0 && inactive > 36) {
    // search for a trigger

    for (int y = 0; y < 13; y++)
      for (int x = 0; x < 20; x++)
        if (getDominoType(x, y) == DominoTypeTrigger) {

          // if the trigger is not lying completely flat
          if (getDominoState(x, y) != 8 && getDominoState(x, y) != 1 && getDominoState(x, y) != 15)
            return 7;

          x = 20;
          y = 13;
        }
  }

  return 0;
}


