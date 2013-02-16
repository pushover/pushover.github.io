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

#include "leveldata.h"
#include "textsections.h"
#include "sha1.h"
#include "graphics.h"
#include "soundsys.h"
#include "screen.h"
#include "ant.h"

#include <stdio.h>

#include <sstream>
#include <iomanip>

#include <assert.h>

void levelPlayer_c::load(const textsections_c & sections, const std::string & userString) {

  openDoorEntry = openDoorExit = false;

  levelData_c::load(sections, userString);
}

void levelPlayer_c::performDoors(void) {

  if (openDoorEntry) {

    if (isEntryDoorClosed())
      soundSystem_c::instance()->startSound(soundSystem_c::SE_DOOR_OPEN);

    if (!isEntryDoorOpen()) {
      openEntryDoorStep();
    }

  } else {

    if (isEntryDoorOpen())
      soundSystem_c::instance()->startSound(soundSystem_c::SE_DOOR_CLOSE);

    if (!isEntryDoorClosed()) {
      closeEntryDoorStep();
    }
  }

  if (openDoorExit) {

    if (isExitDoorClosed())
      soundSystem_c::instance()->startSound(soundSystem_c::SE_DOOR_OPEN);

    if (!isExitDoorOpen()) {
      openExitDoorStep();
    }
  } else {

    if (isExitDoorOpen())
      soundSystem_c::instance()->startSound(soundSystem_c::SE_DOOR_CLOSE);

    if (!isExitDoorClosed()) {
      closeExitDoorStep();
    }
  }
}

DominoType levelPlayer_c::pickUpDomino(int x, int y) {
  DominoType dom = getDominoType(x, y);
  removeDomino(x, y);
  return dom;
}

void levelPlayer_c::putDownDomino(int x, int y, DominoType domino, bool pushin) {

  if (getDominoType(x, y) != DominoTypeEmpty)
  { // there is a domino in the place where we want to put our domino
    if (pushin)
      DominoCrash(x, y, domino, 0);
    else
      DominoCrash(x, y, domino, 0x70);
  }
  else if ((getDominoType(x-1, y) != DominoTypeEmpty) && (getDominoState(x-1, y) >= DO_ST_UPRIGHT+4) && (getDominoState(x-1, y) <= DO_ST_RIGHT))
  { // there is no domino in our place but the left neighbor is falling towards us
    DominoCrash(x, y, domino, 0);
  }
  else if ((getDominoType(x+1, y) != DominoTypeEmpty) && getDominoState(x+1, y) <= DO_ST_UPRIGHT-4)
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
  {
    setDominoExtra(x, y, 0x60);
    setDominoState(x, y, DO_ST_DOWNRIGHT);
  }
  else
  {
    setDominoExtra(x, y, 0x70);
  }

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

    default:
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
      if (getDominoState(x, y) == DO_ST_UPRIGHT) {
        soundSystem_c::instance()->startSound(getDominoType(x, y)-1);
        setDominoDir(x, y, dir);
        setDominoState(x, y, getDominoState(x, y)+dir);
      }
      break;

      // the splitter is special it only falls in the left direction
      // even though the pieces fall in both directions
    case DominoTypeSplitter:
      if (getDominoState(x, y) == DO_ST_UPRIGHT) {
        soundSystem_c::instance()->startSound(soundSystem_c::SE_SPLITTER);
        setDominoDir(x, y, -1);
        setDominoState(x, y, DO_ST_SPLIT);
      }
      break;

      // the exploder only explodes and does not fall in one direction
      // so dir is always set to -1
      // we also need to delay the domino falling against this domino, so
      // return false
    case DominoTypeExploder:
      if (getDominoState(x, y) == DO_ST_UPRIGHT) {
        soundSystem_c::instance()->startSound(soundSystem_c::SE_EXPLODER);
        setDominoDir(x, y, -1);
        setDominoState(x, y, DO_ST_EXPLODE_E);
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
      if (getDominoState(x, y) == DO_ST_UPRIGHT) {
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
      if (getDominoState(x, y) == DO_ST_DOWNRIGHT) {
        setDominoDir(x, y, dir);
      }
      else
      {
        if (getDominoState(x, y) == DO_ST_UPRIGHT && getDominoYOffset(x, y) > -6) {
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
        for (size_t yp = 0; yp < levelY(); yp++)
          for (size_t xp = 0; xp < levelX(); xp++)
          {
            if (getDominoType(xp, yp) == DominoTypeConnectedA && getDominoState(xp, yp) == DO_ST_UPRIGHT)
            {
              sound = true;
              setDominoDir(xp, yp, dir);
              setDominoState(xp, yp, getDominoState(xp, yp)+dir);
            }
            if (getDominoType(xp, yp) == DominoTypeConnectedB && getDominoState(xp, yp) == DO_ST_UPRIGHT)
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
        for (size_t yp = 0; yp < levelY(); yp++)
          for (size_t xp = 0; xp < levelX(); xp++)
          {
            if (getDominoType(xp, yp) == DominoTypeConnectedB && getDominoState(xp, yp) == DO_ST_UPRIGHT)
            {
              sound = true;
              setDominoDir(xp, yp, dir);
              setDominoState(xp, yp, getDominoState(xp, yp)+dir);
            }
            if (getDominoType(xp, yp) == DominoTypeConnectedA && getDominoState(xp, yp) == DO_ST_UPRIGHT)
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
        if (getDominoState(x, y) == DO_ST_UPRIGHT)
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

    default:
      break;
  }

  return retVal;
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

// the final vanisher state, remove the vanisher
// from the level and mark things dirty
void levelPlayer_c::DTA_8(int x, int y) {
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
  if (getDominoType(x-1, y) != DominoTypeEmpty)
    return;

  // if here is a domino 2 to the left of us and that domino has fallen down
  // far enough to the right, we can't fall farther
  if (getDominoType(x-2, y) != DominoTypeEmpty &&
      ((getDominoState(x-2, y) > DO_ST_UPRIGHT+5 && getDominoState(x-2, y) <= DO_ST_RIGHT) ||
       (getDominoState(x-2, y) > DO_ST_DOWNRIGHT+5 && getDominoState(x-2, y) <= DO_ST_RIGHT_AS))
     )
    return;

  // if the domino is a splitter, we need to check, if it is in one of the
  // states where the left halve is far enough down
  // if this is not the case or it is no splitter fall further
  if (x < 2 ||
      getDominoType(x-2, y) != DominoTypeSplitter ||
      (getDominoState(x-2, y) != DO_ST_SPLIT+5 &&
       getDominoState(x-2, y) != DO_ST_SPLIT+6 &&
       getDominoState(x-2, y) != DO_ST_SPLIT+7 &&
       getDominoState(x-2, y) != DO_ST_SPLIT+8 &&
       getDominoState(x-2, y) != DO_ST_SPLIT+10 &&
       getDominoState(x-2, y) != DO_ST_SPLIT+11))
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
  if (getDominoType(x+1, y) != DominoTypeEmpty)
    return;

  // if the 2nd next right neighbor is not empty and far enough fallen to the left
  // we stop
  if ((size_t)(x+2) < levelX() &&
        (   (getDominoType(x+2, y) != DominoTypeEmpty && getDominoState(x+2, y) < DO_ST_UPRIGHT-5)
            // TODO remove, this special case has to be here because the old
            // exploder used to "fall left" when regarded by the states
            // this could be removed at the cost of a recording
         || (getDominoType(x+2, y) == DominoTypeExploder && (getDominoState(x+2, y) == DO_ST_EXPLODE || getDominoState(x+2, y) == DO_ST_EXPLODE))
        )
      )
    return;

  // if the domino is a splitter, we need to check, if it is in one of the
  // states where the right halve is far enough down
  // if this is not the case or it is no splitter fall further
  if ((size_t)(x+2) >= levelX() ||
      getDominoType(x+2, y) != DominoTypeSplitter ||
      (getDominoState(x+2, y) != DO_ST_SPLIT+5 &&
       getDominoState(x+2, y) != DO_ST_SPLIT+6 &&
       getDominoState(x+2, y) != DO_ST_SPLIT+7 &&
       getDominoState(x+2, y) != DO_ST_SPLIT+8 &&
       getDominoState(x+2, y) != DO_ST_SPLIT+9 &&
       getDominoState(x+2, y) != DO_ST_SPLIT+12))
  {
    DTA_4(x, y);
  }
}

// normal falling case
void levelPlayer_c::DTA_4(int x, int y) {

  // TODO cruft, see comment below
  if (getDominoExtra(x, y) == 0x41 || getDominoExtra(x, y) == 0x71)
  {
    setDominoExtra(x, y, getDominoExtra(x, y)-1);
    return;
  }

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
      getDominoState(x, y) == DO_ST_FALLING+7 &&
      getDominoYOffset(x, y) == -10)
  {
    setDominoState(x, y, DO_ST_DOWNRIGHT);
  }

  if (getDominoState(x, y) == DO_ST_UPRIGHT && !getPlatform(x, y+1) && (getDominoExtra(x, y) == 0x40 || getDominoExtra(x, y) == 0x70))
  {
    if ((size_t)(y+1) < levelY())
    {
      setDominoState(x, y+1, 8);
      setDominoType(x, y+1, getDominoType(x, y));

      // TODO cruft: this makes the domino fall a bit slower... this is because
      // in the original player the domino was not put down onto the
      // next level until a bit later... but we do whick maked it
      // make another animation step, but we want to prevent that so
      // we add 1 here and subtract it on the next call see start of this function
      setDominoExtra(x, y+1, getDominoExtra(x, y)+1);
      setDominoDir(x, y+1, getDominoDir(x, y));
      setDominoYOffset(x, y+1, 0);
    }

    removeDomino(x, y);
  }
}

// exploder making its hole
void levelPlayer_c::DTA_5(int x, int y) {

  removeDomino(x, y);

  setPlatform(x, y+1, false);
  setLadder(x, y, false);
  setLadder(x, y+1, false);
}

// hitting next domino to the left
void levelPlayer_c::DTA_3(int x, int y) {

  // if we hit a step, stop falling
  if (getPlatform(x-1, y))
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
    if (getDominoState(x-1, y) != DO_ST_UPRIGHT) {
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
  if (getDominoType(x+1, y) == DominoTypeSplitter && getDominoState(x+1, y) != DO_ST_UPRIGHT)
  {
    if (pushDomino(x-1, y, -1))
      DTA_4(x, y);
    return;
  }

  // if there is a domino to the right of us that has fallen we try to continue
  if (getDominoType(x+1, y) != DominoTypeEmpty && getDominoState(x+1, y) < DO_ST_UPRIGHT)
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

  if (getPlatform(x+1, y))
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
    if (getDominoState(x+1, y) != DO_ST_UPRIGHT) {
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

  if (getDominoType(x-1, y) == DominoTypeSplitter && getDominoState(x-1, y) != DO_ST_UPRIGHT)
  {
    if (pushDomino(x+1, y, 1))
      DTA_4(x, y);
    return;
  }

  if (getDominoType(x-1, y) != DominoTypeEmpty && (getDominoState(x-1, y) > DO_ST_UPRIGHT && getDominoState(x-1, y) <= DO_ST_RIGHT))
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
  DominoType next = getDominoType(x, y);

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
  setDominoState(x, y, DO_ST_CRASH);
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
    if (getDominoYOffset(x, y) != 4) {
      setDominoYOffset(x, y, getDominoYOffset(x, y)+4);
      return;
    }

    // if we have not yet fallen out of the level put
    // the domino into the next block below
    if ((size_t)(y+1) < levelY())
    {
      if (getDominoType(x, y+1) == DominoTypeEmpty)
      {
        setDominoType(x, y+1, getDominoType(x, y));
        setDominoState(x, y+1, getDominoState(x, y));
        setDominoDir(x, y+1, getDominoDir(x, y));
        if (((y % 2) == 0) && ((size_t)(y+2) >= levelY() || !getPlatform(x, y+2)))
          setDominoYOffset(x, y+1, -4);
        else
          setDominoYOffset(x, y+1, 0);

        setDominoExtra(x, y+1, 0x70);
      }
      else
      {
        DominoCrash(x, y+1, getDominoType(x, y), getDominoExtra(x, y));
        setDominoYOffset(x, y+1, 0);
      }
    }

    // remove the old domino
    removeDomino(x, y);

    return;
  }

  // we have reached a special position where we need to check
  // for the ground again
  if (getPlatform(x, y+1))
  {
    // we still crash if there is a domino below us
    if (getDominoType(x, y+2) != DominoTypeEmpty &&
        getDominoType(x, y+2) != DominoTypeAscender
        )
    {
      DominoCrash(x, y+2, getDominoType(x, y), getDominoExtra(x, y));
    }

    // this sets the Ascender turning over again
    if (getDominoType(x, y+2) == DominoTypeAscender)
    {
      if (getDominoState(x, y+2) == DO_ST_LEFT_AS)
      {
        setDominoState(x-1, y+4, DO_ST_RIGHT_AS-1);
        setDominoType(x-1, y+4, DominoTypeAscender);
        setDominoDir(x-1, y+4, -1);
        setDominoExtra(x-1, y+4, 0x00);
        setDominoYOffset(x-1, y+4, getDominoYOffset(x, y+2)-16);
        removeDomino(x, y+2);
      }
      if (getDominoState(x, y+2) == DO_ST_RIGHT_AS)
      {
        setDominoState(x+1, y+4, DO_ST_LEFT_AS+1);
        setDominoType(x+1, y+4, DominoTypeAscender);
        setDominoDir(x+1, y+4, 1);
        setDominoExtra(x+1, y+4, 0x00);
        setDominoYOffset(x+1, y+4, getDominoYOffset(x, y+2)-16);
        removeDomino(x, y+2);
      }
    }

    // no more falling
    setDominoExtra(x, y, 0);

    return;
  }

  // no ground below us, continue falling

  // we can continue, if there is either no domino or no more
  // level below us
  if (getDominoType(x, y+2) == DominoTypeEmpty || getDominoYOffset(x, y+2) != 0)
  {
    setDominoYOffset(x, y, getDominoYOffset(x, y)+4);
    return;
  }

  // if there is no splitter below us, we crash
  if (getDominoType(x, y+2) != DominoTypeSplitter)
  {
    DominoCrash(x, y+2, getDominoType(x, y), getDominoExtra(x, y));

    removeDomino(x, y);

    return;
  }

  // there is a splitter below us, so start that splitter
  // and we vanish
  pushDomino(x, y+2, -1);

  setDominoExtra(x, y+2, getDominoType(x, y));
  removeDomino(x, y);
}

// splitter parts falling further
void levelPlayer_c::DTA_C(int x, int y) {

  // this table contains the positions of the 2 splitter halves
  // for each splitter state, 8 = vertical, 1 = horizontal
  static const int SplitterLookup[] = {
    7, 7,
    6, 6,
    5, 5,
    4, 4,
    3, 3,
    2, 2,
    1, 1,
    2, 1,
    1, 2,
    3, 1,
    1, 3,
    2, 3,
    3, 2
  };

  // first find the positions of the 2 halves for the
  // current state

  int i = 2*(getDominoState(x, y)-DO_ST_SPLIT);

  int a = SplitterLookup[i+1];
  int b = SplitterLookup[i];

  // calculate the new position of the left halve
  if (a == 3)
  {
    // left halve is at a pushing place, check if we hit the wall
    if (!getPlatform(x-1, y))
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
  else if (a == 2 && getDominoType(x-1, y) == DominoTypeEmpty)
  {
    a--;
  }
  else if (a > 3)
  {
    a--;
  }

  // same as above but for right halve
  if (b == 3)
  {
    if (!getPlatform(x+1, y))
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
  else if (b == 2 && getDominoType(x+1, y) == DominoTypeEmpty)
  {
    b--;
  }
  else if (b > 3)
  {
    b--;
  }

  // now find the new state of the splitter domino
  for (int i = 0; i < 13; i++)
  {
    if (SplitterLookup[2*i+1] == a && SplitterLookup[2*i] == b)
    {
      if (getDominoState(x, y) != i)
      {
        setDominoState(x, y, i+DO_ST_SPLIT);
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
void levelPlayer_c::DTA_7(int x, int y)
{
  if (getPlatform(x-2, y+1) && !getPlatform(x-1, y+1))
  {
    setPlatform(x-1, y+1, true);
    removeDomino(x, y);
  }
  else
  {
    DTA_1(x,y);
  }
}

// Bridger right same as DTA_7 but for other direction
void levelPlayer_c::DTA_M(int x, int y)
{
  if (getPlatform(x+2, y+1) && !getPlatform(x+1, y+1))
  {
    setPlatform(x+1, y+1, true);
    removeDomino(x, y);
  }
  else
  {
    DTA_K(x,y);
  }
}


// riser
void levelPlayer_c::DTA_A(int x, int y) {

  int a = (getDominoExtra(x, y) == 0x50) ? 2 : 4;

  bool leftAboveEmpty = true;
  bool aboveEmpty = true;

  if (getPlatform(x-1, y-a+1)) leftAboveEmpty = false;
  if (getPlatform(x-1, y-a-1)) aboveEmpty = false;

  if (aboveEmpty && leftAboveEmpty)
  {
    if (getDominoExtra(x, y) == 0x50)
    {
      if (getDominoType(x-1, y) == DominoTypeEmpty)
      {
        setDominoExtra(x-1, y, 0x60);
        setDominoType(x-1, y, DominoTypeAscender);
        setDominoState(x-1, y, DO_ST_RIGHT_AS-1);
        setDominoDir(x-1, y, -1);
        setDominoYOffset(x-1, y, getDominoYOffset(x, y)-2);
      }
      else
      {
        DominoCrash(x-1, y, getDominoType(x, y), getDominoExtra(x, y));
      }
    }
    else
    {
      if (y > 0)
      {
        if (getDominoType(x-1, y-2) == DominoTypeEmpty)
        {
          setDominoExtra(x-1, y-2, 0x60);
          setDominoType(x-1, y-2, DominoTypeAscender);
          setDominoState(x-1, y-2, DO_ST_RIGHT_AS-1);
          setDominoDir(x-1, y-2, -1);
          setDominoYOffset(x-1, y-2, getDominoYOffset(x, y)+14);
        }
        else
        {
          DominoCrash(x-1, y-2, getDominoType(x, y), getDominoExtra(x, y));
        }
      }
    }

    removeDomino(x, y);

    return;
  }

  if (!getPlatform(x, y-a+1))
  {
    removeDomino(x, y);

    if (getDominoType(x, y+2-a) == DominoTypeEmpty)
    {
      setDominoExtra(x, y+2-a, 0x60);
      setDominoType(x, y+2-a, DominoTypeAscender);
      setDominoState(x, y+2-a, DO_ST_DOWNRIGHT);
      setDominoDir(x, y+2-a, -1);
      setDominoYOffset(x, y+2-a, 0);
    }
    else
    {
      DominoCrash(x, y+2-a, getDominoType(x, y), getDominoExtra(x, y));
    }

    return;
  }

  if (getDominoExtra(x, y) != 0x50)
  {
    if (getDominoType(x, y-2) == DominoTypeEmpty)
    {
      setDominoType(x, y-2, getDominoType(x, y));
      setDominoState(x, y-2, getDominoState(x, y));
      setDominoDir(x, y-2, getDominoDir(x, y));
      setDominoYOffset(x, y-2, getDominoYOffset(x, y)+16);
      setDominoExtra(x, y-2, 0x50);
    }
    else
    {
      DominoCrash(x, y-2, getDominoType(x, y), getDominoExtra(x, y));
    }

    removeDomino(x, y);
  }
}


// Riser
void levelPlayer_c::DTA_O(int x, int y) {

  int a = (getDominoExtra(x, y) == 0x50) ? 2 : 4;

  bool rightAboveEmpty = true;
  bool aboveEmpty = true;

  if (getPlatform(x+1, y-a+1)) rightAboveEmpty = false;
  if (getPlatform(x+1, y-a-1)) aboveEmpty = false;

  if (aboveEmpty && rightAboveEmpty)
  {
    if (getDominoExtra(x, y) == 0x50)
    {
      if (getDominoType(x+1, y) == DominoTypeEmpty)
      {
        setDominoExtra(x+1, y, 0x60);
        setDominoType(x+1, y, DominoTypeAscender);
        setDominoState(x+1, y, DO_ST_LEFT_AS+1);
        setDominoDir(x+1, y, 1);
        setDominoYOffset(x+1, y, getDominoYOffset(x, y)-2);
      }
      else
      {
        DominoCrash(x+1, y, getDominoType(x, y), getDominoExtra(x, y));
      }
    }
    else
    {
      if (y > 0)
      {
        if (getDominoType(x+1, y-2) == DominoTypeEmpty)
        {
          setDominoExtra(x+1, y-2, 0x60);
          setDominoType(x+1, y-2, DominoTypeAscender);
          setDominoState(x+1, y-2, DO_ST_LEFT_AS+1);
          setDominoDir(x+1, y-2, 1);
          setDominoYOffset(x+1, y-2, getDominoYOffset(x, y)+14);
        }
        else
        {
          DominoCrash(x+1, y-2, getDominoType(x, y), getDominoExtra(x, y));
        }
      }
    }

    removeDomino(x, y);

    return;
  }

  if (!getPlatform(x, y-a+1))
  {
    removeDomino(x, y);

    if (getDominoType(x, y+2-a) == DominoTypeEmpty)
    {
      setDominoExtra(x, y+2-a, 0x60);
      setDominoType(x, y+2-a, DominoTypeAscender);
      setDominoState(x, y+2-a, DO_ST_DOWNRIGHT);
      setDominoDir(x, y+2-a, 1);
      setDominoYOffset(x, y+2-a, 0);
    }
    else
    {
      DominoCrash(x, y+2-a, getDominoType(x, y), getDominoExtra(x, y));
    }

    return;
  }

  if (getDominoExtra(x, y) != 0x50)
  {
    if (getDominoType(x, y-2) == DominoTypeEmpty)
    {
      setDominoType(x, y-2, getDominoType(x, y));
      setDominoState(x, y-2, getDominoState(x, y));
      setDominoDir(x, y-2, getDominoDir(x, y));
      setDominoYOffset(x, y-2, getDominoYOffset(x, y)+16);
      setDominoExtra(x, y-2, 0x50);
    }
    else
    {
      DominoCrash(x, y-2, getDominoType(x, y), getDominoExtra(x, y));
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
    if (getDominoYOffset(x, y) == 4 && y > 2)
    {
      if (getPlatform(x, y-3) && getPlatform(x, y-1))
      {
        setDominoExtra(x, y, 0x50);

        return;
      }
    }
    if (getDominoYOffset(x, y) == -6 && y > 2)
    {
      if (getPlatform(x, y-3) && getPlatform(x, y-2))
      {
        setDominoExtra(x, y, 0x50);
        return;
      }
    }
    if (getDominoYOffset(x, y) == -10)
    {
      if (y > 2)
      {
        if (getPlatform(x, y-3))
        {
          setDominoExtra(x, y, 0);

          return;
        }
      }

      if (y > 1)
      {
        if (getDominoType(x, y-1) == DominoTypeEmpty)
        {
          setDominoExtra(x, y-1, 0x60);
          setDominoYOffset(x, y-1, -4);
          setDominoDir(x, y-1, getDominoDir(x, y));
          setDominoState(x, y-1, DO_ST_DOWNRIGHT);
          setDominoType(x, y-1, DominoTypeAscender);
        }
        else
        {
          DominoCrash(x, y-1, getDominoType(x, y), getDominoExtra(x, y));
        }
      }

      removeDomino(x, y);

      return;
    }

    // normally raise the domino by 2 units
    setDominoYOffset(x, y, getDominoYOffset(x, y)-2);

    return;
  }

  // if the raiser had been pushed, set it loose
  if (riserDir != 0)
  {
    setDominoState(x, y, DO_ST_DOWNRIGHT);
    // then move it
    DTA_4(x, y);
  }
}

// Stone completely fallen down right used for
// standard, Trigger, Delay, Bridger
void levelPlayer_c::DTA_K(int x, int y)
{
  if (!getPlatform(x+1, y+1))
  {
    if (getDominoType(x+1, y+1) != DominoTypeEmpty)
    {
      DominoCrash(x+1, y+1, getDominoType(x, y), getDominoExtra(x, y));
      removeDomino(x, y);
    }
    else
    {
      if ((size_t)x+1 < levelX())
      {
        setDominoType(x+1, y, getDominoType(x, y));
        setDominoState(x+1, y, 2);
        setDominoDir(x+1, y, getDominoDir(x, y));
        setDominoYOffset(x+1, y, 2);

        if (getPlatform(x+1, y+2))
          setDominoExtra(x+1, y, 0x40);
        else
          setDominoExtra(x+1, y, 0x70);

        soundSystem_c::instance()->startSound(soundSystem_c::SE_ASCENDER);
      }

      removeDomino(x, y);
    }
    return;
  }

  setDominoDir(x, y, 0);
}

// Stone completely fallen down left used for
// standard, Trigger, Delay, Bridger
void levelPlayer_c::DTA_1(int x, int y)
{
  if (!getPlatform(x-1, y+1))
  {
    if (getDominoType(x-1, y+1) != DominoTypeEmpty)
    {
      DominoCrash(x-1, y+2, getDominoType(x, y), getDominoExtra(x, y));
      removeDomino(x, y);
    }
    else
    {
      if (x > 0)
      {
        setDominoType(x-1, y, getDominoType(x, y));
        setDominoState(x-1, y, 0xE);
        setDominoDir(x-1, y, getDominoDir(x, y));
        setDominoYOffset(x-1, y, 2);

        if (getPlatform(x-1, y+2))
          setDominoExtra(x-1, y, 0x40);
        else
          setDominoExtra(x-1, y, 0x70);

        soundSystem_c::instance()->startSound(soundSystem_c::SE_ASCENDER);
      }

      removeDomino(x, y);
    }
    return;
  }
}

// Tumbler fallen down left
void levelPlayer_c::DTA_6(int x, int y)
{
  if (!getPlatform(x-1, y+1))
  {
    if (getDominoType(x-1, y+1) != DominoTypeEmpty)
    {
      DominoCrash(x-1, y+2, getDominoType(x, y), getDominoExtra(x, y));
      removeDomino(x, y);
    }
    else
    {
      if (x > 0)
      {
        setDominoType(x-1, y, getDominoType(x, y));
        setDominoState(x-1, y, 0xE);
        setDominoDir(x-1, y, getDominoDir(x, y));
        setDominoYOffset(x-1, y, 2);

        if (getPlatform(x-1, y+2))
          setDominoExtra(x-1, y, 0x40);
        else
          setDominoExtra(x-1, y, 0x70);

        // TODO remainder of the old 2 block level code: when the tubler fell down a tile
        // it was called again in the next row making one more step, we need to emulate that
        // here
        if (y % 2)
          DTA_J(x-1, y);
      }

      soundSystem_c::instance()->startSound(soundSystem_c::SE_ASCENDER);

      removeDomino(x, y);

    }
    return;
  }

  if (x > 0)
  {
    if (getDominoType(x-1, y) != DominoTypeEmpty)
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
  }

  removeDomino(x, y);
}

// Tumbler fallen down right
void levelPlayer_c::DTA_L(int x, int y) {

  if (!getPlatform(x+1, y+1))
  {
    if (getDominoType(x+1, y+1) != DominoTypeEmpty)
    {
      DominoCrash(x+1, y+1, getDominoType(x, y), getDominoExtra(x, y));
      removeDomino(x, y);
    }
    else
    {
      if ((size_t)(x+1) < levelX())
      {
        setDominoType(x+1, y, getDominoType(x, y));
        setDominoState(x+1, y, 2);
        setDominoDir(x+1, y, getDominoDir(x, y));
        setDominoYOffset(x+1, y, 2);

        if (getPlatform(x+1, y+2))
          setDominoExtra(x+1, y, 0x40);
        else
          setDominoExtra(x+1, y, 0x70);
      }

      soundSystem_c::instance()->startSound(soundSystem_c::SE_ASCENDER);

      removeDomino(x, y);
    }
    return;
  }

  if (getDominoType(x+1, y) != DominoTypeEmpty)
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
}


bool levelPlayer_c::CounterStopper(DominoType num) {
  for (size_t yp = 0; yp < levelY(); yp++)
    for (size_t xp = 0; xp < levelX(); xp++)
      if (  (getDominoType(xp, yp) > num)
          &&(getDominoType(xp, yp) <= DominoTypeCounter3)
          &&(getDominoState(xp, yp) == DO_ST_UPRIGHT)
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

void levelPlayer_c::DTA(int num, int x, int y)
{
  switch(num)
  {
    case  0:              break;
    case  1: DTA_1(x, y); break;
    case  2: DTA_2(x, y); break;
    case  3: DTA_3(x, y); break;
    case  4: DTA_4(x, y); break;
    case  5: DTA_5(x, y); break;
    case  6: DTA_6(x, y); break;
    case  7: DTA_7(x, y); break;
    case  8: DTA_8(x, y); break;
    case 10: DTA_A(x, y); break;
    case 12: DTA_C(x, y); break;
    case 14: DTA_E(x, y); break;
    case 15: DTA_F(x, y); break;
    case 16: DTA_G(x, y); break;
    case 17: DTA_H(x, y); break;
    case 18: DTA_I(x, y); break;
    case 19: DTA_J(x, y); break;
    case 20: DTA_K(x, y); break;
    case 21: DTA_L(x, y); break;
    case 22: DTA_M(x, y); break;
    case 24: DTA_O(x, y); break;
    case 25: DTA_P(x, y); break;

    default:
      assert(0);
  }
}

void levelPlayer_c::callStateFunction(int type, int state, int x, int y)
{
  // This table contains the action functions to call for each domino type and each
  // possible state of that domino
  // each column stands for one domino
  // the rows are the possible state
  // 0 action means don't do anything
  // 99 achtion means invalid state
  static int action[DO_ST_NUM][DominoNumber] = {
  //  DominoTypeEmpty,        DominoTypeTumbler,      DominoTypeConnectedB,   DominoTypeCrash2,
  //      DominoTypeStandard,     DominoTypeBridger,      DominoTypeCounter1,     DominoTypeCrash3,
  //          DominoTypeStopper,      DominoTypeVanish,       DominoTypeCounter2,     DominoTypeCrash4,
  //              DominoTypeSplitter,     DominoTypeTrigger,      DominoTypeCounter3,     DominoTypeCrash5,
  //                  DominoTypeExploder,     DominoTypeAscender,     DominoTypeCrash0,
  //                      DominoTypeDelay,        DominoTypeConnectedA,   DominoTypeCrash1,
    { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // invalid state 0
    { 99,  1, 99, 99, 99,  1,  6,  7,  8,  1, 99,  1,  1,  1,  1,  1, 99, 99, 99, 99, 99, 99 },
    { 99,  2, 99, 99, 99,  2,  2,  2,  4,  2, 99,  2,  2,  2,  2,  2, 99, 99, 99, 99, 99, 99 },
    { 99,  3, 99, 99, 99,  3,  3,  3,  3,  3, 99,  3,  3,  3,  3,  3, 99, 99, 99, 99, 99, 99 },
    { 99,  4, 99, 99, 99,  4,  4,  4,  4,  4, 99,  4,  4,  4,  4,  4, 99, 99, 99, 99, 99, 99 }, // domino falling left
    { 99,  4, 99, 99, 99,  4,  4,  4,  4,  4, 99,  4,  4,  4,  4,  4, 99, 99, 99, 99, 99, 99 },
    { 99,  4, 99, 99, 99,  4,  4,  4,  4,  4, 99,  4,  4,  4,  4,  4, 99, 99, 99, 99, 99, 99 },
    { 99,  4, 99, 99, 99,  4,  4,  4,  4,  4, 99,  4,  4,  4,  4,  4, 99, 99, 99, 99, 99, 99 },
    { 99, 14, 15, 15, 15, 16, 14, 14, 14, 14, 17, 14, 14, 25, 25, 25, 99, 99, 99, 99, 99, 99 }, // domino standing vertically
    { 99,  4, 99, 99, 99,  4,  4,  4,  4,  4, 99,  4,  4,  4,  4,  4, 99, 99, 99, 99, 99, 99 },
    { 99,  4, 99, 99, 99,  4,  4,  4,  4,  4, 99,  4,  4,  4,  4,  4, 99, 99, 99, 99, 99, 99 },
    { 99,  4, 99, 99, 99,  4,  4,  4,  4,  4, 99,  4,  4,  4,  4,  4, 99, 99, 99, 99, 99, 99 },
    { 99,  4, 99, 99, 99,  4,  4,  4,  4,  4, 99,  4,  4,  4,  4,  4, 99, 99, 99, 99, 99, 99 }, // domino falling right
    { 99, 18, 99, 99, 99, 18, 18, 18, 18, 18, 99, 18, 18, 18, 18, 18, 99, 99, 99, 99, 99, 99 },
    { 99, 19, 99, 99, 99, 19, 19, 19,  4, 19, 99, 19, 19, 19, 19, 19, 99, 99, 99, 99, 99, 99 },
    { 99, 20, 99, 99, 99, 20, 21, 22,  8, 20, 99, 20, 20, 20, 20, 20, 99, 99, 99, 99, 99, 99 },
    { 99, 99, 99, 99,  5, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // explosion, explosion ended
    { 99, 99, 99, 99,  4, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    { 99, 99, 99, 99,  4, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    { 99, 99, 99, 99,  4, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    { 99, 99, 99, 99,  4, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    { 99, 99, 99, 99,  4, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 },
    { 99, 99, 99, 99,  4, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // explostion, explostion started
    { 99, 99, 99, 12, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // Splitter states...
    { 99, 99, 99, 12, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // Splitter states...
    { 99, 99, 99, 12, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // Splitter states...
    { 99, 99, 99, 12, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // Splitter states...
    { 99, 99, 99, 12, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // Splitter states...
    { 99, 99, 99, 12, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // Splitter states...
    { 99, 99, 99, 12, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // Splitter states...
    { 99, 99, 99, 12, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // Splitter states...
    { 99, 99, 99, 12, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // Splitter states...
    { 99, 99, 99, 12, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // Splitter states...
    { 99, 99, 99, 12, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // Splitter states...
    { 99, 99, 99, 12, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // Splitter states...
    { 99, 99, 99, 12, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // Splitter states...
    { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 10, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // ascender special: ceiling flip
    { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,  4, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // ascender special: ceiling flip
    { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,  3, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // ascender special: ceiling flip
    { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,  4, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // ascender special: ceiling flip
    { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,  4, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // ascender special: ceiling flip
    { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,  4, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // ascender special: ceiling flip
    { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,  4, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // ascender special: ceiling flip
    { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 17, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // ascender special: ceiling flip
    { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,  4, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // ascender special: ceiling flip
    { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,  4, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // ascender special: ceiling flip
    { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,  4, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // ascender special: ceiling flip
    { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,  4, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // ascender special: ceiling flip
    { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 18, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // ascender special: ceiling flip
    { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,  4, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // ascender special: ceiling flip
    { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 24, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 }, // ascender special: ceiling flip
    { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 14, 14, 14, 14, 14, 14 }, // Crash
    { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 14, 14, 14, 14, 14, 14 }, // Crash
    { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 14, 14, 14, 14, 14, 14 }, // Crash
    { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 14, 14, 14, 14, 14, 14 }, // Crash
    { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 14, 14, 14, 14, 14, 14 }, // Crash
    { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,  0,  0,  0,  0,  0,  0 }, // Crash
  };

  assert(type < DominoNumber);
  assert(state < DO_ST_NUM);
  assert(action[state][type] != 99);

  DTA(action[state][type], x, y);
}

void levelPlayer_c::performDominos(void) {

  inactive++;

  for (size_t y = 0; y < levelY(); y++)
    for (size_t x = 0; x < levelX(); x++)
      if (getDominoType(x, y) != DominoTypeEmpty &&
          getDominoState(x, y) != DO_ST_INVALID) {

        int oldState = getDominoState(x, y);
        int oldExtra = getDominoExtra(x, y);
        int oldYpos = getDominoYOffset(x, y);

        callStateFunction(getDominoType(x, y), getDominoState(x, y), x, y);

        if (oldState != getDominoState(x, y) || oldExtra != getDominoExtra(x, y) || oldYpos != getDominoYOffset(x, y))
          if (getDominoType(x, y) != DominoTypeTumbler)
            inactive = 0;
      }

  timeTick();
}

bool levelPlayer_c::triggerNotFlat(void) const {

  for (size_t y = 0; y < levelY(); y++)
    for (size_t x = 0; x < levelX(); x++)
      if (getDominoType(x, y) == DominoTypeTrigger)
      {
        // if the trigger is not lying completely flat
        if (getDominoState(x, y) != DO_ST_UPRIGHT && getDominoState(x, y) != DO_ST_LEFT && getDominoState(x, y) != DO_ST_RIGHT)
          return true;

        // there can be multiple triggers, so go on
      }

  return false;
}

bool levelPlayer_c::triggerIsFalln(void) const {

  for (size_t y = 0; y < levelY(); y++)
    for (size_t x = 0; x < levelX(); x++)
      if (getDominoType(x, y) == DominoTypeTrigger)
      {
        // if the trigger is not lying flat... then it is not falln
        if (    (getDominoState(x, y) !=  DO_ST_LEFT)
             && (getDominoState(x, y) !=  DO_ST_RIGHT)
            )
          return false;

        // there can be multiple triggers, so go on
      }

  return true;
}

bool levelPlayer_c::rubblePile(void) const
{
  for (size_t y = 0; y < levelY(); y++)
    for (size_t x = 0; x < levelX(); x++)
      if (getDominoType(x, y) >= DominoTypeCrash0 &&
          getDominoType(x, y) <= DominoTypeCrash5)
      {
        return true;
      }
  return false;
}

bool levelPlayer_c::dominosFalln(void) const {

  for (size_t y = 0; y < levelY(); y++)
    for (size_t x = 0; x < levelX(); x++) {

      if (getDominoType(x, y) != DominoTypeEmpty && getDominoType(x, y) != DominoTypeStopper)
      {
        if (getDominoType(x, y) == DominoTypeSplitter)
        { // for splitters is must have started to fall
          if (getDominoState(x, y) > DO_ST_LEFT+1 && getDominoState(x, y) <= DO_ST_UPRIGHT)
          {
            return false;
          }
        }
        else if (getDominoType(x, y) == DominoTypeTrigger)
        { // triggers must as least have started to fall
          if (getDominoState(x, y) == DO_ST_UPRIGHT && !triggerIsFalln())
          {
            return false;
          }
        }
        else if (getDominoType(x, y) == DominoTypeTumbler)
        { // tumbler must lie on something or lean against a wall
          // not fallen far enough
          if (getDominoState(x, y) >= DO_ST_LEFT+2 && getDominoState(x, y) <= DO_ST_RIGHT-2) {
            return false;
          }

          // fallen far enough but neighbor empty
          if (   getDominoState(x, y) <= DO_ST_LEFT+1
              && (getDominoType(x-1, y) == DominoTypeEmpty)
              && (getDominoType(x-2, y) == DominoTypeEmpty || getDominoState(x-2, y) < DO_ST_RIGHT-1)
             )
          {
            return false;
          }
          if (   getDominoState(x, y) >= DO_ST_RIGHT-1
              && (getDominoType(x+1, y) == DominoTypeEmpty)
              && (getDominoType(x+2, y) == DominoTypeEmpty || getDominoState(x+2, y) > DO_ST_LEFT+1)
             )
          {
            return false;
          }

        }
        else
        {
          if (getDominoState(x, y) == DO_ST_UPRIGHT)
          {
            return false;
          }
          if (getDominoState(x, y) > DO_ST_LEFT+2 && getDominoState(x, y) < DO_ST_RIGHT-2)
          {
            // here we certainly fail
            return false;
          }
          // in this case we might still succeed, when we lean against a block
          if (   getDominoState(x, y) == DO_ST_LEFT+3
              && (   getDominoType(x-1, y) != DominoTypeStopper
                  || getDominoType(x+1, y) == DominoTypeEmpty
                  || (getDominoDir(x+1, y) != -1 && getDominoType(x+1, y) != DominoTypeSplitter))
             )
          {
            // here we lean against a of a blocker and can't go back
            return false;
          }

          if (   getDominoState(x, y) == DO_ST_RIGHT-3
              && (   getDominoType(x+1, y) != DominoTypeStopper
                  || getDominoType(x-1, y) == DominoTypeEmpty
                  || (getDominoDir(x-1, y) != 1 && getDominoType(x-1, y) != DominoTypeSplitter))
             )
          {
            // here we lean against a step
            return false;
          }
        }
      }
    }

  return true;
}


bool levelPlayer_c::dominosStanding(void) const
{
  for (size_t y = 0; y < levelY(); y++)
    for (size_t x = 0; x < levelX(); x++)
      if (getDominoType(x, y) != DominoTypeEmpty && getDominoType(x, y) != DominoTypeStopper)
        if (getDominoState(x, y) == 8)
          return true;

  return false;
}

