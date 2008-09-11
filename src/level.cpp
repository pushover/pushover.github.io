#include "level.h"

#include "decompress.h"
#include "graphics.h"

#include <stdio.h>

level_c::level_c(void) {
  SDL_Surface * vid = SDL_GetVideoSurface();
  background = SDL_CreateRGBSurface(0, vid->w, vid->h, 32,
      vid->format->Rmask, vid->format->Gmask, vid->format->Bmask, 0);
}

level_c::~level_c(void) {
  SDL_FreeSurface(background);
}

void level_c::load(const char * name) {

  char fname[200];

  snprintf(fname, 200, "./screens/%s.SCR", name);

  unsigned char * dat = decompress(fname, 0);

  /* copy level data */
  for (int i = 0; i < 13*20; i++) {

    level[i/20][i%20].bg = ((unsigned short)dat[i*6] << 8) + dat[i*6+1];
    level[i/20][i%20].fg = dat[i*6+2];

    level[i/20][i%20].dominoType = dat[i*6+3];

    level[i/20][i%20].dominoState = 8;
    level[i/20][i%20].dominoDir = 0;
    level[i/20][i%20].dominoYOffset = 0;
    level[i/20][i%20].dominoExtra = 0;
  }

  /* copy theme */
  for (int i = 0; i < 10; i++)
    theme[i] = dat[260*6+i];

  /* copy the door positions */
  doorEntryX = dat[1580];
  doorEntryY = dat[1581];
  doorExitX  = dat[1582];
  doorExitY  = dat[1583];


  level[doorEntryY][doorEntryX].fg = FgElementDoor0;
  openDoorEntry = openDoorExit = false;

  delete [] dat;

  for (unsigned int i = 0; i < 13; i++)
    staticDirty[i] = dynamicDirty[i] = 0xFFFFFFFF;
}

void level_c::updateBackground(graphics_c * gr) {
  for (unsigned int y = 0; y < 13; y++)
    for (unsigned int x = 0; x < 20; x++) {
      if ((staticDirty[y] >> x) & 1) {

        SDL_Rect dst;
        dst.x = x*gr->blockX();
        dst.y = y*gr->blockY();
        dst.w = gr->blockX();
        dst.h = gr->blockY();
        SDL_BlitSurface(gr->getBgTile(getBg(x, y)), 0, background, &dst);
        SDL_BlitSurface(gr->getFgTile(getFg(x, y)), 0, background, &dst);

        /* apply gradient effect */
        for (unsigned int i = 0; i < gr->blockY() && y*gr->blockY()+i < (unsigned int)background->h; i++)
          for (unsigned int j = 0; j < gr->blockX(); j++) {

            uint32_t col = *((uint32_t*)(((uint8_t*)background->pixels) + (y*gr->blockY()+i) * background->pitch +
                  background->format->BytesPerPixel*(x*gr->blockX()+j)));

            Uint8 r, g, b;

            SDL_GetRGB(col, background->format, &r, &g, &b);

            double val = (2.0-((1.0*x*gr->blockX()+j)/background->w + (1.0*y*gr->blockY()+i)/background->h));
            val += (1.0*rand()/RAND_MAX)/20 - 1.0/40;
            if (val < 0) val = 0;
            if (val > 2) val = 2;

            r = (Uint8)(((255.0-r)*val+r)*r/255);
            g = (Uint8)(((255.0-g)*val+g)*g/255);
            b = (Uint8)(((255.0-b)*val+b)*b/255);

            col = SDL_MapRGB(background->format, r, g, b);

            *((uint32_t*)(((uint8_t*)background->pixels) + (y*gr->blockY()+i) * background->pitch +
                  background->format->BytesPerPixel*(x*gr->blockX()+j))) = col;
          }

        /* remove dirty bit */
        staticDirty[y] &= ~(1 << x);
      }
    }
}

void level_c::drawDominos(SDL_Surface * target, graphics_c * gr, bool debug) {
  for (unsigned int y = 0; y < 13; y++)
    for (unsigned int x = 0; x < 20; x++) {
      if ((dynamicDirty[y] >> x) & 1 || true) {

        /* copy background from background surface */
        {
          SDL_Rect src, dst;
          dst.x = x*gr->blockX();
          dst.y = y*gr->blockY();
          dst.w = gr->blockX();
          dst.h = gr->blockY();
          src.x = x*gr->blockX();
          src.y = y*gr->blockY();
          src.w = gr->blockX();
          src.h = gr->blockY();
          SDL_BlitSurface(background, &src, target, &dst);
        }
      }
    }
  for (unsigned int y = 0; y < 13; y++)
    for (unsigned int x = 0; x < 20; x++) {
      if ((dynamicDirty[y] >> x) & 1 || true) {
        /* paint the foreground */

        // TODO: for splitters it is necessary to pain the splitting domino
        if (getDominoType(x, y) > 0) {
          SDL_Surface * v = gr->getDomino(getDominoType(x, y)-1, getDominoState(x, y)-1);

          if (v) {

            SDL_Rect dst;
            dst.x = (x-2)*gr->blockX();
            dst.y = y*gr->blockY()-v->h + gr->dominoDisplace() + level[y][x].dominoYOffset*2;
            dst.w = v->w;
            dst.h = v->h;
            SDL_BlitSurface(v, 0, target, &dst);
          }
        }
      }
    }

  // repaint the ladders in front of dominos
  for (unsigned int y = 0; y < 13; y++)
    for (unsigned int x = 0; x < 20; x++) {
      if ((dynamicDirty[y] >> x) & 1 || true) {
        if (getFg(x, y) == FgElementPlatformLadderDown || getFg(x, y) == FgElementLadder) {
          SDL_Rect dst;
          dst.x = x*gr->blockX();
          dst.y = y*gr->blockY();
          dst.w = gr->blockX();
          dst.h = gr->blockY();
          SDL_BlitSurface(gr->getFgTile(FgElementLadder2), 0, target, &dst);
	}
        else if (getFg(x, y) == FgElementPlatformLadderUp)
        {
          SDL_Rect dst;
          dst.x = x*gr->blockX();
          dst.y = y*gr->blockY();
          dst.w = gr->blockX();
          dst.h = gr->blockY();
          SDL_BlitSurface(gr->getFgTile(FgElementLadderMiddle), 0, target, &dst);
	}
      }
    }

  if (debug)
  {
    for (unsigned int y = 0; y < 13; y++)
      for (unsigned int x = 0; x < 20; x++) {

        if ((dynamicDirty[y] >> x) & 1)
        {
          for (int h = 0; h < gr->blockY(); h+=2)
          {
            SDL_Rect dst;
            dst.w = gr->blockX();
            dst.h = 1;
            dst.x = x*gr->blockX();
            dst.y = y*gr->blockY()+h;
            Uint32 color = SDL_MapRGB(target->format, 0, 0, 0);
            SDL_FillRect(target, &dst, color);
          }
        }
      }
  }

  for (unsigned int y = 0; y < 13; y++) {
    dynamicDirty[y] = 0;
  }

}

void level_c::performDoors(void) {

  if (openDoorEntry) {
    if (getFg(doorEntryX, doorEntryY) < FgElementDoor3) {
      level[doorEntryY][doorEntryX].fg++;
      staticDirty[doorEntryY] |= 1 << doorEntryX;
      dynamicDirty[doorEntryY] |= 1 << doorEntryX;
    }

  } else {
    if (getFg(doorEntryX, doorEntryY) > FgElementDoor0) {
      level[doorEntryY][doorEntryX].fg--;
      staticDirty[doorEntryY] |= 1 << doorEntryX;
      dynamicDirty[doorEntryY] |= 1 << doorEntryX;
    }
  }

  if (openDoorExit) {
    if (getFg(doorExitX, doorExitY) < FgElementDoor3) {
      level[doorExitY][doorExitX].fg++;
      staticDirty[doorExitY] |= 1 << doorExitX;
      dynamicDirty[doorExitY] |= 1 << doorExitX;
    }
  } else {
    if (getFg(doorExitX, doorExitY) > FgElementDoor0) {
      level[doorExitY][doorExitX].fg--;
      staticDirty[doorExitY] |= 1 << doorExitX;
      dynamicDirty[doorExitY] |= 1 << doorExitX;
    }
  }
}

bool level_c::containsPlank(int x, int y) {
  if (x < 0 || x >= 20 || y < 0 || y >= 13) return false;

  unsigned int fg = getFg(x, y);

  return (fg == 1 || fg == 2 | fg == 3 || fg == 4 || fg == 6 || fg == 7 || fg == 10 || fg == 15);
}

bool level_c::noGround(int x, int y, bool onLadder) {

  if (y >= 12) return true;

  if (getFg(x, y) == FgElementEmpty) return true;

  if (getFg(x, y) >= FgElementDoor0) return true;

  if (getFg(x, y) != FgElementLadder) return false;

  if (onLadder) return false;

  return true;
}

int level_c::pickUpDomino(int x, int y) {
  int dom = level[y][x].dominoType;
  level[y][x].dominoType = DominoTypeEmpty;
  level[y][x].dominoState = 8;
  level[y][x].dominoDir = 0;
  level[y][x].dominoYOffset = 0;
  level[y][x].dominoExtra = 0;

  return dom;
}

void level_c::putDownDomino(int x, int y, int domino, bool pushin) {

  if (level[y][x].dominoType != 0)
  { // there is a domino in the place where we want to put our domino
    if (pushin)
      DominoCrash(x, y, domino, 0);
    else
      DominoCrash(x, y, domino, 0x70);
  }
  else if (x > 0 && level[y][x-1].dominoType && level[y][x-1].dominoState >= 12)
  { // there is no domino in our place but the left neighbor is falling towards us
    DominoCrash(x, y, domino, 0);
  }
  else if (x < 19 && level[y][x+1].dominoType && level[y][x-1].dominoState <= 4)
  { // there is no domino in our place but the right neighbor is falling towards us
    DominoCrash(x, y, domino, 0);
  }
  else
  { // we can place the domino
    level[y][x].dominoType = domino;
    level[y][x].dominoState = 8;
    level[y][x].dominoDir = 0;
    level[y][x].dominoYOffset = 0;
    level[y][x].dominoExtra = 0;
  }
}

void level_c::fallingDomino(int x, int y) {
  if (level[y][x].dominoType == DominoTypeRiser)
    level[y][x].dominoExtra = 0x60;
  else
    level[y][x].dominoExtra = 0x70;
}

bool level_c::pushDomino(int x, int y, int dir) {

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
        DominoCrash(x, y, level[y][x+dir].dominoYOffset, level[y][x+dir].dominoExtra);
        return false;
      }
      break;

    // there is never a problem with those types
    case DominoTypeExploder:
    case DominoTypeDelay:
    case DominoTypeVanisher:
    case DominoTypeRiser:
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
    case DominoTypeBlocker:
       if (getDominoDir(x, y) == -dir)
       {
         DominoCrash(x, y, level[y][x+dir].dominoYOffset, level[y][x+dir].dominoExtra);
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
    case DominoTypeVanisher:
    case DominoTypeTrigger:
      if (getDominoState(x, y) == 8) {
        // play sound
        level[y][x].dominoDir = dir;
        level[y][x].dominoState += dir;
      }
      break;

    // the splitter is special it only falls in the left direction
    // even though the pieces fall in both directions
    case DominoTypeSplitter:
      if (getDominoState(x, y) == 8) {
        // play dound
        level[y][x].dominoDir = -1;
        level[y][x].dominoState --;
      }
      break;

    // the exploder only explosed and does not fall in one direction
    // so dir is always set to -1
    // we also need to delay the domino falling against this domino, so
    // return false
    case DominoTypeExploder:
      if (getDominoState(x, y) == 8) {
        // play dound
        level[y][x].dominoDir = -1;
        level[y][x].dominoState --;
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
          // play sound
          level[y][x].dominoDir = dir;
          level[y][x].dominoExtra = 20;

        }
        retVal = false;
      }
      break;

    // we return false then pushing the riser, because it will rise
    // and the domino has to wait
    case DominoTypeRiser:
      if (getDominoState(x, y) == 16) {
        level[y][x].dominoDir = dir;
      }
      else
      {
        if (getDominoState(x, y) == 8 && level[y][x].dominoYOffset > -6) {
          if (getDominoExtra(x, y) != 0x60) {
            // play sound
          }
          level[y][x].dominoDir = dir;
          level[y][x].dominoExtra = 0x60;
          retVal = false;
        }
      }
      break;

    // for this types we always return false to stop dominos
    // falling against this block
    case DominoTypeBlocker:
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


// the 2 tigger falln functions. They just call the
// normal domino falln function and additionally set the trigger bit
void level_c::DTA_9(int x, int y, int x2, int y2) {
  DTA_1(x, y, x2, y2);

  triggerFalln = true;
}
void level_c::DTA_N(int x, int y, int x2, int y2) {
  DTA_K(x, y, x2, y2);

  triggerFalln = true;
}

// this is for the blocker splitter and exploder dominos, when they
// are falling after beeing lost when going over the edge
// we check, if we are still falling and only handle the falling case
void level_c::DTA_F(int x, int y, int x2, int y2) {
  if (getDominoExtra(x, y) == 0x70)
    DTA_E(x, y, x2, y2);
}

// this is for the delay domino. We check, if we are falling down when
// falling over the edge or when the delay time is up
// if the delay time is still running, decrement and wait
void level_c::DTA_G(int x, int y, int x2, int y2) {
  if (getDominoExtra(x, y) <= 1 || getDominoExtra(x, y) == 0x70)
    DTA_E(x, y, x2, y2);
  else
    level[y][x].dominoExtra--;
}

// crash case with dust clouds, we need to call DTA_E because
// dominos might crash in the air and the rubble needs to fall down
void level_c::DTA_B(int x, int y, int x2, int y2) {
  DTA_E(x, y, x2, y2);

  markDirty(x2+1, y2);
  markDirty(x2+1, y2-1);
}

// splitter opening simply call the normal domino falling
// function and mark some more blocks because we split left and
// right and the normal function will only mark one side
void level_c::DTA_D(int x, int y, int x2, int y2) {
  markDirty(x2+1, y2-1);
  markDirty(x2+1, y2);

  DTA_4(x, y, x2, y2);

  if (getDominoState(x, y) == 5)
    markDirty(x2, y2-2);
}

// the final vansher state, remove the vanisher
// from the level and mark things dirty
void level_c::DTA_8(int x, int y, int x2, int y2) {
  markDirty(x2, y2);
  markDirty(x2+getDominoDir(x, y), y2);
  markDirty(x2, y2-1);
  markDirty(x2+getDominoDir(x, y), y2-1);

  level[y][x].dominoType = DominoTypeEmpty;
  level[y][x].dominoDir = 0;
  level[y][x].dominoState = 0;
}

// this is the nearly falln down left case
void level_c::DTA_2(int x, int y, int x2, int y2) {

  if (getDominoExtra(x, y) == 0x40 || getDominoExtra(x, y) == 0x70)
  {
    DTA_4(x, y, x2, y2);
    return;
  }

  // if we move in the other direction (e.g. tumbler) simply continue
  if (getDominoDir(x, y) == 1)
  {
    DTA_4(x, y, x2, y2);
    return;
  }

  // if there is a domino left of us, we can not fall down further
  if (getDominoType(x-1, y) != 0)
    return;

  // if here is a domino 2 to the left of us and that domino has falln down
  // far enough to the right, we can not fall farther
  if (x >= 2 && getDominoType(x-2, y) != 0 && getDominoState(x-2, y) > 13)
    return;

  // if the domino is a splitter, we need to check, if it is in one of the
  // states where the left halve is far enough down
  // if this is not the case or it is no splitter fall further
  if (getDominoType(x-2, y) != DominoTypeSplitter ||
      getDominoState(x-2, y) != 1 &&
      getDominoState(x-2, y) != 10 &&
      getDominoState(x-2, y) != 12 &&
      getDominoState(x-2, y) != 13)
  {
    DTA_4(x, y, x2, y2);
  }
}

// nearly falln down right
void level_c::DTA_J(int x, int y, int x2, int y2) {

  if (getDominoExtra(x, y) == 0x40 || getDominoExtra(x, y) == 0x70)
  {
    DTA_4(x, y, x2, y2);
    return;
  }

  // if we fall in the other direction (tumbler) we simply contnue
  if (getDominoDir(x, y) == -1)
  {
    DTA_4(x, y, x2, y2);
    return;
  }

  // if our right neighbor if not empty, we stop
  if (getDominoType(x+1, y) != 0)
    return;

  // if the 2nd next right neighbor is not empty and far enough falln to the left
  // we stop
  if (x <= 17 && getDominoType(x+2, y) != 0 && getDominoState(x+2, y) < 3)
    return;

  // if the domino is a splitter, we need to check, if it is in one of the
  // states where the right halve is far enough down
  // if this is not the case or it is no splitter fall further
  if (getDominoType(x+2, y) != DominoTypeSplitter ||
      getDominoState(x+2, y) != 1 &&
      getDominoState(x+2, y) != 9 &&
      getDominoState(x+2, y) != 11 &&
      getDominoState(x+2, y) != 14)
  {
    DTA_4(x, y, x2, y2);
  }
}

// normal falling case
void level_c::DTA_4(int x, int y, int x2, int y2) {
  if (getDominoExtra(x, y) == 0x40 || getDominoExtra(x, y) == 0x70)
  {
    level[y][x].dominoYOffset += 2;
    level[y][x].dominoState += getDominoDir(x, y);
  }
  else if (getDominoExtra(x, y) == 0x60)
  {
    level[y][x].dominoYOffset -= 2;
    level[y][x].dominoState += getDominoDir(x, y);
  }

  // update state
  level[y][x].dominoState += getDominoDir(x, y);

  if (getDominoType(x, y) == DominoTypeRiser &&
      getDominoState(x, y) == 8 &&
      level[y][x].dominoYOffset == -10)
  {
    level[y][x].dominoState = 16;
  }

  markDirty(x, y);

  // add some dirty blocks depending on the direction we have falln
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

  if (level[y][x].dominoYOffset == 8)
  {
    markDirty(x+getDominoDir(x, y), y);
    markDirty(x+getDominoDir(x, y), y-1);
  }
}

// exploder making its hole
void level_c::DTA_5(int x, int y, int x2, int y2) {

  level[y][x].dominoType = 0;
  level[y][x].dominoState = 0;
  level[y][x].fg = FgElementEmpty;

  if (level[y][x+1].fg == FgElementPlatformMiddle ||
      level[y][x+1].fg == FgElementPlatformLadderDown ||
      level[y][x+1].fg == FgElementPlatformLadderUp)
  {
    level[y][x+1].fg = FgElementPlatformStart;
  }
  else if (level[y][x+1].fg == FgElementPlatformEnd)
  {
    level[y][x+1].fg = FgElementPlatformStrip;
  }

  if (level[y][x-1].fg == FgElementPlatformMiddle ||
      level[y][x-1].fg == FgElementPlatformLadderDown ||
      level[y][x-1].fg == FgElementPlatformLadderUp)
  {
    level[y][x-1].fg = FgElementPlatformEnd;
  }
  else if (level[y][x-1].fg == FgElementPlatformStart)
  {
    level[y][x-1].fg = FgElementPlatformStrip;
  }

  markDirty(x, y);
  markDirty(x-1, y);
  markDirty(x+1, y);

  staticDirty[y] |= 1 << x;
  staticDirty[y] |= 1 << (x+1);
  staticDirty[y] |= 1 << (x-1);
}

// hitting next domino to the left
void level_c::DTA_3(int x, int y, int x2, int y2) {

  // if we hit a step, stop falling
  if (x2 > 0 && getFg(x2-1, y2) == FgElementPlatformStep4)
    return;

  if (level[y][x].dominoYOffset == 8 && x2 > 0 && getFg(x2-1, y2) == FgElementPlatformStep1)
    return;

  // if the next somino is empty, continue falling
  if (getDominoType(x-1, y) == DominoTypeEmpty) {
    DTA_4(x, y, x2, y2);
    return;
  }

  // if the next domino is not a blocker and not a delay, we
  // simply push that domino and continue falling
  if (getDominoType(x2-1, y2) != DominoTypeBlocker &&
      getDominoType(x2-1, y2) != DominoTypeDelay)
  {
    if (pushDomino(x2-1, y2, -1))
      DTA_4(x, y, x2, y2);
    return;
  }

  // if the next neibor is a delay we only continue falling, if
  // it has started falling as well
  if (getDominoType(x-1, y) == DominoTypeDelay)
  {
    if (getDominoState(x-1, y) != 8) {
      DTA_4(x, y, x2, y2);
      return;
    }
  }

  // if the right neighbor is a splitter and the splitter is already falln
  // we try to continue falling
  if (getDominoType(x+1, y) == DominoTypeSplitter && getDominoState(x+1, y) != 8)
  {
    if (pushDomino(x2-1, y2, -1))
      DTA_4(x, y, x2, y2);
    return;
  }

  // if there is a domino to the right of us that has falln we try to continue
  if (getDominoType(x+1, y) != DominoTypeEmpty && getDominoState(x+1, y) < 8)
  {
    if (pushDomino(x2-1, y2, -1))
      DTA_4(x, y, x2, y2);
    return;
  }

  // now the only case left is to reverse direction but still push the domino to our left
  level[y][x].dominoDir = 1;
  pushDomino(x2-1, y2, -1);
  DTA_4(x, y, x2, y2);
}

// same as DTA_3 but for the right direction
void level_c::DTA_I(int x, int y, int x2, int y2) {

  if (x2 < 19 && getFg(x2+1, y2) == FgElementPlatformStep7)
    return;

  if (level[y][x].dominoYOffset == 8 && x2 < 19 && getFg(x2-1, y2) == FgElementPlatformStep6)
    return;

  if (getDominoType(x+1, y) == DominoTypeEmpty) {
    DTA_4(x, y, x2, y2);
    return;
  }

  if (getDominoType(x2+1, y2) != DominoTypeBlocker &&
      getDominoType(x2+1, y2) != DominoTypeDelay)
  {
    if (pushDomino(x2+1, y2, 1))
      DTA_4(x, y, x2, y2);
    return;
  }

  if (getDominoType(x+1, y) == DominoTypeDelay)
  {
    if (getDominoState(x+1, y) != 8) {
      DTA_4(x, y, x2, y2);
      return;
    }
  }

  if (getDominoType(x-1, y) == DominoTypeSplitter && getDominoState(x-1, y) != 8)
  {
    if (pushDomino(x2+1, y2, 1))
      DTA_4(x, y, x2, y2);
    return;
  }

  if (getDominoType(x-1, y) != DominoTypeEmpty && getDominoState(x-1, y) > 8)
  {
    if (pushDomino(x2+1, y2, 1))
      DTA_4(x, y, x2, y2);
    return;
  }

  level[y][x].dominoDir = -1;
  pushDomino(x2+1, y2, 1);
  DTA_4(x, y, x2, y2);
}

// return true, if there is a platform at the given position
bool level_c::isTherePlatform(int x, int y) {

  switch (level[y][x].fg)
  {
    case FgElementEmpty:              return false;
    case FgElementPlatformStart:      return true;
    case FgElementPlatformMiddle:     return true;
    case FgElementPlatformEnd:        return true;
    case FgElementPlatformLadderDown: return true;
    case FgElementLadder:             return false;
    case FgElementPlatformLadderUp:   return true;
    case FgElementPlatformStep1:      return true;
    case FgElementPlatformStep2:      return true;
    case FgElementPlatformStep3:      return false;
    case FgElementPlatformStep4:      return false;
    case FgElementPlatformStep5:      return true;
    case FgElementPlatformStep6:      return true;
    case FgElementPlatformStep7:      return false;
    case FgElementPlatformStep8:      return false;
    case FgElementPlatformWrongDoor:  return false;
    case FgElementPlatformStack:      return true;
    case FgElementLadderMiddle:       return false;
    case FgElementPlatformStrip:      return true;
    case FgElementLadder2:            return false;
    case FgElementDoor0:              return false;
    case FgElementDoor1:              return false;
    case FgElementDoor2:              return false;
    case FgElementDoor3:              return false;
    default:                          return false;
  }
}

// handle dominos crashing into something
void level_c::DominoCrash(int x, int y, int type, int extra) {

    // what do we crash into?
    int next = level[y][x].dominoType;

    // if all participating parts are completely yellow
    // the result is a yellow pile
    if ((next == DominoTypeStandard || next == DominoTypeCrash4) &&
        (type == DominoTypeStandard || type == DominoTypeCrash4))
    {
        next = DominoTypeCrash4;
    }
    // if all parts are red, the pile is red
    else if ((next == DominoTypeBlocker || next == DominoTypeCrash3) &&
             (type == DominoTypeBlocker || type == DominoTypeCrash3))
    {
        next = DominoTypeCrash3;
    }
    // otherwise it is mixed color
    else
    {
        next = DominoTypeCrash2;
    }

    // set the resulting domino animation
    level[y][x].dominoType = next;
    level[y][x].dominoState = 1;
    level[y][x].dominoDir = 1;
    if (level[y][x].dominoExtra == 0x70 || extra == 0x70 || level[y][x].dominoExtra == 0x60 || extra == 0x60)
    {
        level[y][x].dominoExtra = 0x70;
        level[y][x].dominoYOffset &= 0xFC;
    }
    else
    {
        level[y][x].dominoExtra = 0;
    }

    markDirty(x-1, y);
    markDirty(x, y);
    markDirty(x+1, y);

// 		mov	ax, 4
// 		push	ax
// 		call	SomethingWithSound
}

// vertial stone
void level_c::DTA_E(int x, int y, int x2, int y2) {

  if (level[y][x].dominoExtra == 0x40)
  {
    level[y][x].dominoExtra = 0;
  }

  // start falling to the side, once we hit the ground and the
  // domino had been given a direction
  if (level[y][x].dominoExtra != 0x70)
  {
    if (level[y][x].dominoDir != 0)
    {
      DTA_4(x, y, x2, y2);
    }
    return;
  }

  // continue falling down
  if (level[y][x].dominoYOffset != 0)
  {
    // if we have not yet reached the next level of the level data
    if (level[y][x].dominoYOffset != 12) {
      level[y][x].dominoYOffset += 4;
      markDirty(x, y+1);
      markDirty(x, y);
      return;
    }

    // if we have not yet fallen out of the level put
    // the domino into the next block below
    if (y < 12)
    {
      level[y+1][x].dominoType = level[y][x].dominoType;
      level[y+1][x].dominoState = level[y][x].dominoState;
      level[y+1][x].dominoDir = level[y][x].dominoDir;
      level[y+1][x].dominoYOffset = 0;
      level[y+1][x].dominoExtra = 0x70;
    }

    // remove the old domino
    level[y][x].dominoType = DominoTypeEmpty;
    level[y][x].dominoState = 0;
    level[y][x].dominoState = 0;
    level[y][x].dominoDir = 0;
    level[y][x].dominoYOffset = 0;
    level[y][x].dominoExtra = 0;

    markDirty(x, y+1);
    markDirty(x, y);
    return;
  }

  // we have reached a special position where we need to check
  // for the ground again
  if (isTherePlatform(x, y))
  {
    // we still crash if there is a domino below us
    if (getDominoType(x, y+1) != DominoTypeEmpty)
    {
        DominoCrash(x, y+1, level[y][x].dominoType, level[y][x].dominoExtra);
    }

    // no more falling
    level[y][x].dominoExtra = 0;

    // we need to set the yOffset properly for the halve steps
    if (level[y][x].fg == 8 || level[y][x].fg == 11)
    {
      level[y][x].dominoYOffset = 8;
    }

    markDirty(x, y);
    markDirty(x, y+1);
    return;
  }

  // no ground below us, continue falling

  // we can continue, if there is either no domino or no more
  // level below us
  if (y >= 12 || level[y+1][x].dominoType == DominoTypeEmpty)
  {
    level[y][x].dominoYOffset += 4;
    markDirty(x, y);
    markDirty(x, y+1);
    return;
  }

  // if there is no splitter below us, we crash
  if (level[y+1][x].dominoType != DominoTypeSplitter)
  {
    DominoCrash(x, y+1, level[y][x].dominoType, level[y][x].dominoExtra);

    level[y][x].dominoType = DominoTypeEmpty;
    level[y][x].dominoState = 0;
    level[y][x].dominoDir = 0;
    level[y][x].dominoYOffset = 0;
    level[y][x].dominoExtra = 0;

    markDirty(x, y);
    markDirty(x, y+1);

    return;
  }

  // there is a splitter below us, so start that splitter
  // and we vanish
  pushDomino(x, y+1, -1);

  level[y+1][x].dominoExtra = level[y][x].dominoType;
  level[y][x].dominoType = DominoTypeEmpty;
  level[y][x].dominoState = 0;
  level[y][x].dominoDir = 0;
  level[y][x].dominoYOffset = 0;
  level[y][x].dominoExtra = 0;

  markDirty(x, y);
  markDirty(x, y+1);
}

// splitter parts falling further
void level_c::DTA_C(int x, int y, int x2, int y2) {

    // this table contains the positions of the 2 splitter halves
    // for each splitter state, 8 = vertial, 1 = horizontal
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
    int a = SplitterLookup[level[y][x].dominoState*2+1];
    int b = SplitterLookup[level[y][x].dominoState*2];

    // calculate the new position of the left halve
    if (a == 3)
    {
        // left halve is at a pushing place, check if we hit the wall
        if (x > 0 && level[y][x-1].fg != 0xA)
        {
            // no wall, check, if we hit a domino and if so try to push it
            if (level[y][x-1].dominoType == DominoTypeEmpty)
            {
                a--;
            }
            else if (pushDomino(x-1, y, -1))
            {
                a--;
            }
        }
    }
    else if (a == 2 && level[y][x-1].dominoType == 0)
    {
        a--;
    }

    // same as above but for right halve
    if (b == 3)
    {
        if (x < 19)
        {
            if (level[y][x+1].dominoType == DominoTypeEmpty)
            {
                b--;
            }
            else if (pushDomino(x+1, y, 1))
            {
                b--;
            }
        }
    }
    else if (b == 2 && level[y][x+1].dominoType == 0)
    {
        b--;
    }

    // now find the new state of the plitter domino
    for (int i = 0; i < 15; i++)
    {
        if (SplitterLookup[2*i+1] == a && SplitterLookup[2*i] == b)
        {
            if (level[y][x].dominoState != i)
            {
                level[y][x].dominoState = i;
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
void level_c::DTA_7(int x, int y, int x2, int y2) {

  int fg2;

  if (x >= 2)
  {
    fg2 = level[y][x-2].fg;
  }
  else
  {
    fg2 = 0;
  }

  int fg1;

  if (x >= 1)
  {
    fg1 = level[y][x-1].fg;
  }
  else
  {
    fg1 = 0;
  }

  int fg = level[y][x].fg;

  if (fg != FgElementPlatformStart && fg != FgElementPlatformStrip)
  {
    DTA_1(x, y, x2, y2);
    return;
  }

  int doit = 0;

  if (fg1 == 3)
  {
    fg1 = 2;
    if (fg == 1)
    {
      fg = 2;
    }
    else
    {
      fg = 3;
    }
    doit = 1;
  }
  else if (fg1 != 0 || fg2 != 3)
  {
    if (fg1 == 18)
    {
      fg1 = 1;
      if (fg == 1)
      {
        fg = 2;
      }
      else
      {
        fg = 3;
      }
      doit = 1;
    }
    else
    {
      if (fg1 == 0 && fg2 == 18)
      {
        fg2 = 1;
        fg1 = 2;
        if (fg == 1)
        {
          fg = 2;
        }
        else
        {
          fg = 3;
        }
        doit = 1;
      }
    }
  }
  else
  {
    fg2 = 2;
    fg1 = 2;
    if (fg == 1)
    {
      fg = 2;
    }
    else
    {
      fg = 3;
    }
    doit = 1;
  }

  // if we can't build a bridge, we continue falling, maybe topple over and
  // continue downwards
  if (doit == 0)
  {
    DTA_1(x, y, x2, y2);
    return;
  }

  if (x >= 2)
  {
    level[y][x-2].fg = fg2;
    markDirty(x-2, y);
  }

  if (x >= 1)
  {
    level[y][x-1].fg = fg1;
  }

  level[y][x].fg = fg;
  level[y][x].dominoType = 0;
  level[y][x].dominoDir = 0;
  markDirty(x, y);
  markDirty(x-1, y);
  markDirty(x, y-1);
  markDirty(x-1, y-1);

  staticDirty[y] |= 1 << x;
  staticDirty[y] |= 1 << (x-1);
  staticDirty[y] |= 1 << (x-2);
}

// Brider right same as DTA_7 but for other direction
void level_c::DTA_M(int x, int y, int x2, int y2) {

  int fg2;

  if (x < 18)
  {
    fg2 = level[y][x+2].fg;
  }
  else
  {
    fg2 = 0;
  }

  int fg1;

  if (x < 19)
  {
    fg1 = level[y][x+1].fg;
  }
  else
  {
    fg1 = 0;
  }

  int fg = level[y][x].fg;

  if (fg != FgElementPlatformEnd && fg != FgElementPlatformStrip)
  {
    DTA_K(x, y, x2, y2);
    return;
  }

  int doit = 0;

  if (fg1 == 1)
  {
    if (fg == 3)
    {
      fg = 2;
    }
    else
    {
      fg = 1;
    }
    fg1 = 2;
    doit = 1;
  }
  else if (fg1 != 0 || fg2 != 1)
  {
    if (fg1 == 18)
    {
      fg1 = 1;
      if (fg == 3)
      {
        fg = 2;
      }
      else
      {
        fg = 1;
      }
      doit = 1;
    }
    else
    {
      if (fg1 == 0 && fg2 == 18)
      {
        fg2 = 3;
        fg1 = 2;
        if (fg == 3)
        {
          fg = 2;
        }
        else
        {
          fg = 1;
        }
        doit = 1;
      }
    }
  }
  else
  {
    fg2 = 2;
    fg1 = 2;
    if (fg == 3)
    {
      fg = 2;
    }
    else
    {
      fg = 1;
    }
    doit = 1;
  }

  if (doit == 0)
  {
    DTA_K(x, y, x2, y2);
    return;
  }

  if (x < 18)
  {
    level[y][x+2].fg = fg2;
    markDirty(x+2, y);
  }

  if (x < 19)
  {
    level[y][x+1].fg = fg1;
  }

  level[y][x].fg = fg;
  level[y][x].dominoType = 0;
  level[y][x].dominoDir = 0;
  markDirty(x, y);
  markDirty(x+1, y);
  markDirty(x, y-1);
  markDirty(x+1, y-1);

  staticDirty[y] |= 1 << x;
  staticDirty[y] |= 1 << (x+1);
  staticDirty[y] |= 1 << (x+2);
}


// riser
void level_c::DTA_A(int x, int y, int x2, int y2) {

  int a;

  if (level[y][x].dominoExtra == 0x50)
    a = 1;
  else
    a = 2;

  int b;
  if (x > 0)
    b = level[y][x-1].fg;
  else
    b = FgElementEmpty;

  int c;
  c = level[y-a][x].fg;

  if ((c == FgElementPlatformStart || c == FgElementPlatformStrip) &&
      b == FgElementEmpty)
  {
    if (a == 1)
    {
      level[y][x-1].dominoExtra = 0x60;
      level[y][x-1].dominoType = DominoTypeRiser;
      level[y][x-1].dominoState = 14;
      level[y][x-1].dominoDir = -1;
      level[y][x-1].dominoYOffset = level[y][x].dominoYOffset-2;
    }
    else
    {
      if (y > 0)
      {
        level[y-1][x-1].dominoExtra = 0x60;
        level[y-1][x-1].dominoType = DominoTypeRiser;
        level[y-1][x-1].dominoState = 14;
        level[y-1][x-1].dominoDir = -1;
        level[y-1][x-1].dominoYOffset = level[y][x].dominoYOffset+14;
      }
    }

    level[y][x].dominoType = DominoTypeEmpty;
    level[y][x].dominoState = 0;
    level[y][x].dominoDir = 0;
    level[y][x].dominoYOffset = 0;
    level[y][x].dominoExtra = 0;

    markDirty(x, y-a);
    markDirty(x-1, y-a);
    markDirty(x, y-a+1);
    markDirty(x-1, y-a+1);

    return;
  }

  if (c == 0)
  {
    level[y][x].dominoType = DominoTypeEmpty;
    level[y][x].dominoState = 0;
    level[y][x].dominoDir = 0;
    level[y][x].dominoYOffset = 0;
    level[y][x].dominoExtra = 0;

    level[y+1-a][x].dominoExtra = 0x60;
    level[y+1-a][x].dominoType = DominoTypeRiser;
    level[y+1-a][x].dominoState = 8;
    level[y+1-a][x].dominoDir = -1;
    level[y+1-a][x].dominoYOffset = 0;

    markDirty(x, y-a);
    markDirty(x-1, y-a);
    markDirty(x, y-a+1);
    markDirty(x-1, y-a+1);

    return;
  }

  if (level[y][x].dominoExtra != 0x50)
  {
    level[y-1][x].dominoType = level[y][x].dominoType;
    level[y-1][x].dominoState = level[y][x].dominoState;
    level[y-1][x].dominoDir = level[y][x].dominoDir;
    level[y-1][x].dominoYOffset = level[y][x].dominoYOffset+16;
    level[y-1][x].dominoExtra = 0x50;

    level[y][x].dominoType = DominoTypeEmpty;
    level[y][x].dominoState = 0;
    level[y][x].dominoDir = 0;
    level[y][x].dominoYOffset = 0;
    level[y][x].dominoExtra = 0;
  }
}


// Riser
void level_c::DTA_O(int x, int y, int x2, int y2) {

  int a;

  if (level[y][x].dominoExtra == 0x50)
    a = 1;
  else
    a = 2;

  int b;
  if (x < 19)
    b = level[y][x+1].fg;
  else
    b = FgElementEmpty;

  int c;
  c = level[y-a][x].fg;

  if ((c == FgElementPlatformEnd || c == FgElementPlatformStrip) &&
      b == FgElementEmpty)
  {
    if (a == 1)
    {
      level[y][x+1].dominoExtra = 0x60;
      level[y][x+1].dominoType = DominoTypeRiser;
      level[y][x+1].dominoState = 2;
      level[y][x+1].dominoDir = 1;
      level[y][x+1].dominoYOffset = level[y][x].dominoYOffset-2;
    }
    else
    {
      if (y > 0)
      {
        level[y-1][x+1].dominoExtra = 0x60;
        level[y-1][x+1].dominoType = DominoTypeRiser;
        level[y-1][x+1].dominoState = 2;
        level[y-1][x+1].dominoDir = 1;
        level[y-1][x+1].dominoYOffset = level[y][x].dominoYOffset+14;
      }
    }

    level[y][x].dominoType = DominoTypeEmpty;
    level[y][x].dominoState = 0;
    level[y][x].dominoDir = 0;
    level[y][x].dominoYOffset = 0;
    level[y][x].dominoExtra = 0;

    markDirty(x, y-a);
    markDirty(x+1, y-a);
    markDirty(x, y-a+1);
    markDirty(x+1, y-a+1);

    return;
  }

  if (c == 0)
  {
    level[y][x].dominoType = DominoTypeEmpty;
    level[y][x].dominoState = 0;
    level[y][x].dominoDir = 0;
    level[y][x].dominoYOffset = 0;
    level[y][x].dominoExtra = 0;

    level[y+1-a][x].dominoExtra = 0x60;
    level[y+1-a][x].dominoType = DominoTypeRiser;
    level[y+1-a][x].dominoState = 8;
    level[y+1-a][x].dominoDir = 1;
    level[y+1-a][x].dominoYOffset = 0;

    markDirty(x, y-a);
    markDirty(x+1, y-a);
    markDirty(x, y-a+1);
    markDirty(x+1, y-a+1);

    return;
  }

  if (level[y][x].dominoExtra != 0x50)
  {
    level[y-1][x].dominoType = level[y][x].dominoType;
    level[y-1][x].dominoState = level[y][x].dominoState;
    level[y-1][x].dominoDir = level[y][x].dominoDir;
    level[y-1][x].dominoYOffset = level[y][x].dominoYOffset+16;
    level[y-1][x].dominoExtra = 0x50;

    level[y][x].dominoType = DominoTypeEmpty;
    level[y][x].dominoState = 0;
    level[y][x].dominoDir = 0;
    level[y][x].dominoYOffset = 0;
    level[y][x].dominoExtra = 0;
  }
}

// riser risign vertically
void level_c::DTA_H(int x, int y, int x2, int y2) {

    int riserDir = level[y][x].dominoDir;

    if (level[y][x].dominoExtra == 0x60)
    {
        if (level[y][x].dominoYOffset == 4 && y > 1)
        {
            if (isTherePlatform(x, y-2))
            {
                if (level[y][x].fg == 0xA || level[y][x].fg == 0xD)
                {
                    level[y][x].dominoState = 16;
                    level[y][x].dominoExtra = 0x50;

                    markDirty(x, y);
                    markDirty(x, y-2);
                    markDirty(x, y-1);
                    return;
                }
            }
        }
        if (level[y][x].dominoYOffset == -6 && y > 1)
        {

            if (isTherePlatform(x, y-2))
            {
                if (level[y][x].fg == 9 || level[y][x].fg == 0xE)
                {
                    level[y][x].dominoState = 16;
                    level[y][x].dominoExtra = 0x50;
                    markDirty(x, y);
                    markDirty(x, y-2);
                    markDirty(x, y-1);
                    return;
                }
            }
        }
        if (level[y][x].dominoYOffset == -10)
        {
            if (y > 1)
            {
                if (isTherePlatform(x, y-2))
                {
                    level[y][x].dominoState = 16;
                    level[y][x].dominoExtra = 0;

                    markDirty(x, y);
                    markDirty(x, y-2);
                    markDirty(x, y-1);
                    return;
                }
            }

            if (y > 0)
            {
                level[y-1][x].dominoExtra = 0x60;
                level[y-1][x].dominoYOffset = 4;
                level[y-1][x].dominoDir = level[y][x].dominoDir;
                level[y-1][x].dominoState = 8;
                level[y-1][x].dominoType = DominoTypeRiser;
            }

            level[y][x].dominoType = 0;
            level[y][x].dominoState = 0;
            level[y][x].dominoDir = 0;
            level[y][x].dominoYOffset = 0;
            level[y][x].dominoExtra = 0;

            markDirty(x, y);
            markDirty(x, y-2);
            markDirty(x, y-1);
            return;
        }

        if (level[y][x].dominoYOffset == -8 && y > 1)
        {
            if (isTherePlatform(x, y-2))
            {
                level[y][x].dominoState = 16;
            }
        }

        if (level[y][x].dominoYOffset == -6 && y > 1)
        {
            if (isTherePlatform(x, y-2))
            {
                level[y][x].dominoState = 17;
            }
        }
        level[y][x].dominoYOffset -= 2;

        markDirty(x, y);
        markDirty(x, y-2);
        markDirty(x, y-1);
        return;
    }

    if (riserDir != 0)
    {
        if (level[y][x].dominoState == 16)
            level[y][x].dominoState = 8;

        DTA_4(x, y, x2, y2);
    }
}

// Stone completely falln down right used for
// standard, Trigger, Delay, Bridger
void level_c::DTA_K(int x, int y, int x2, int y2) {
    int fg;

    if (x < 19)
        fg = level[y][x+1].fg;
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
            level[y][x].dominoType = 0;
            level[y][x].dominoState = 0;
            level[y][x].dominoDir = 0;

            markDirty(x, y-1);
            markDirty(x+1, y-1);
            markDirty(x, y);
            markDirty(x+1, y);
        }
        else
        {
            level[y][x+1].dominoType = getDominoType(x, y);
            level[y][x+1].dominoState = 2;
            level[y][x+1].dominoDir = getDominoDir(x, y);
            level[y][x+1].dominoYOffset = 2;
            level[y][x+1].dominoExtra = 0x70;

// 		mov	ax, 0Ah
// 		push	ax
// 		call	SomethingWithSound
//
            level[y][x].dominoType = 0;
            level[y][x].dominoState = 0;
            level[y][x].dominoDir = 0;

            markDirty(x, y-1);
            markDirty(x+1, y-1);
            markDirty(x, y);
            markDirty(x+1, y);
        }
    }
    else
    {
        if (x < 19 && level[y][x].fg == FgElementPlatformStep1 && fg == FgElementPlatformStep2)
        {
            if (level[y][x+1].dominoType != DominoTypeEmpty)
            {
                DominoCrash(x+1, y, level[y][x].dominoType, level[y][x].dominoExtra);
            }
            else
            {
                level[y][x+1].dominoType = level[y][x].dominoType;
                level[y][x+1].dominoState = 2;
                level[y][x+1].dominoDir = level[y][x].dominoDir;
                level[y][x+1].dominoYOffset = 2;
                level[y][x+1].dominoExtra = 0x40;
            }

            level[y][x].dominoType = 0;
            level[y][x].dominoState = 0;
            level[y][x].dominoDir = 0;
            level[y][x].dominoYOffset = 0;

            markDirty(x, y-1);
            markDirty(x+1, y-1);
            markDirty(x, y);
            markDirty(x+1, y);

            return;
        }

        if (x < 19 && y < 11 && getFg(x, y) == FgElementPlatformStep2)
        {
            if (level[y+1][x+1].dominoType != 0)
            {
                DominoCrash(x+1, y+1, level[y][x].dominoType, level[y][x].dominoExtra);
            }
            else
            {
                level[y+1][x+1].dominoType = level[y][x].dominoType;
                level[y+1][x+1].dominoState = 2;
                level[y+1][x+1].dominoDir = level[y][x].dominoDir;
                level[y+1][x+1].dominoYOffset = -6;
                level[y+1][x+1].dominoExtra = 0x40;
            }

            level[y][x].dominoType = 0;
            level[y][x].dominoState = 0;
            level[y][x].dominoDir = 0;
            level[y][x].dominoYOffset = 0;

            markDirty(x, y-1);
            markDirty(x+1, y-1);
            markDirty(x, y);
            markDirty(x+1, y);

            return;
        }

        level[y][x].dominoDir = 0;
        return;
    }

}

// Stone completely falln down left used for
// standard, Trigger, Delay, Bridger
void level_c::DTA_1(int x, int y, int x2, int y2) {

    int fg;

    if (x > 0)
        fg = level[y][x-1].fg;
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
            level[y][x].dominoType = 0;
            level[y][x].dominoState = 0;
            level[y][x].dominoDir = 0;

            markDirty(x, y-1);
            markDirty(x-1, y-1);
            markDirty(x, y);
            markDirty(x-1, y);

            return;
        }
        else
        {
            level[y][x-1].dominoType = getDominoType(x, y);
            level[y][x-1].dominoState = 0xE;
            level[y][x-1].dominoDir = getDominoDir(x, y);
            level[y][x-1].dominoYOffset = 2;
            level[y][x-1].dominoExtra = 0x70;

// 		mov	ax, 0Ah
// 		push	ax
// 		call	SomethingWithSound
//
            level[y][x].dominoType = 0;
            level[y][x].dominoState = 0;
            level[y][x].dominoDir = 0;

            markDirty(x, y-1);
            markDirty(x-1, y-1);
            markDirty(x, y);
            markDirty(x-1, y);
        }
    }
    else
    {
        if (x > 0 && level[y][x].fg == FgElementPlatformStep6 && fg == FgElementPlatformStep5)
        {
            if (level[y][x-1].dominoType != DominoTypeEmpty)
            {
                DominoCrash(x-1, y, level[y][x].dominoType, level[y][x].dominoExtra);
            }
            else
            {
                level[y][x-1].dominoType = level[y][x].dominoType;
                level[y][x-1].dominoState = 0xE;
                level[y][x-1].dominoDir = level[y][x].dominoDir;
                level[y][x-1].dominoYOffset = 2;
                level[y][x-1].dominoExtra = 0x40;
            }

            level[y][x].dominoType = 0;
            level[y][x].dominoState = 0;
            level[y][x].dominoDir = 0;
            level[y][x].dominoYOffset = 0;

            markDirty(x, y-1);
            markDirty(x-1, y-1);
            markDirty(x, y);
            markDirty(x-1, y);

            return;
        }

        if (x > 0 && y < 11 && getFg(x, y) == FgElementPlatformStep5)
        {
            if (level[y+1][x-1].dominoType != 0)
            {
                DominoCrash(x-1, y+1, level[y][x].dominoType, level[y][x].dominoExtra);
            }
            else
            {
                level[y+1][x-1].dominoType = level[y][x].dominoType;
                level[y+1][x-1].dominoState = 0xE;
                level[y+1][x-1].dominoDir = level[y][x].dominoDir;
                level[y+1][x-1].dominoYOffset = -6;
                level[y+1][x-1].dominoExtra = 0x40;
            }

            level[y][x].dominoType = 0;
            level[y][x].dominoState = 0;
            level[y][x].dominoDir = 0;
            level[y][x].dominoYOffset = 0;

            markDirty(x, y-1);
            markDirty(x-1, y-1);
            markDirty(x, y);
            markDirty(x-1, y);

            return;
        }

        level[y][x].dominoDir = 0;
        return;
    }

}

// Tumbler falln down left
void level_c::DTA_6(int x, int y, int x2, int y2) {

  int fg;

  if (x > 0)
    fg = level[y][x-1].fg;
  else
    fg = 0;

  if (fg == FgElementLadder)
    fg = 0;

  if ((level[y][x].fg == FgElementPlatformStart || level[y][x].fg == FgElementPlatformStrip) && fg == 0)
  {
    if (level[y+1][x-1].dominoType != 0)
    {
      DominoCrash(x-1, y+1, level[y][x].dominoType, level[y][x].dominoExtra);
    }
    else
    {
      level[y][x-1].dominoType = level[y][x].dominoType;
      level[y][x-1].dominoState = 14;
      level[y][x-1].dominoDir = level[y][x].dominoDir;
      level[y][x-1].dominoYOffset = 2;
      level[y][x-1].dominoExtra = 0x70;

// 		mov	ax, 0Ah
// 		push	ax
// 		call	SomethingWithSound
    }

    level[y][x].dominoType = 0;
    level[y][x].dominoState = 0;
    level[y][x].dominoDir = 0;

    markDirty(x, y);
    markDirty(x-1, y);
    markDirty(x, y-1);
    markDirty(x-1, y-1);

    return;
  }

  if (x > 0 && level[y][x].fg == FgElementPlatformStep6 && fg == FgElementPlatformStep5)
  {
    if (level[y][x-1].dominoType != 0)
    {
      DominoCrash(x-1, y, level[y][x].dominoType, level[y][x].dominoExtra);
    }
    else
    {
      level[y][x-1].dominoType = level[y][x].dominoType;
      level[y][x-1].dominoState = 14;
      level[y][x-1].dominoDir = level[y][x].dominoDir;
      level[y][x-1].dominoYOffset = 2;
      level[y][x-1].dominoExtra = 0x40;
    }

    level[y][x].dominoType = 0;
    level[y][x].dominoState = 0;
    level[y][x].dominoDir = 0;
    level[y][x].dominoYOffset = 0;

    markDirty(x, y);
    markDirty(x-1, y);
    markDirty(x, y-1);
    markDirty(x-1, y-1);

    return;
  }

  if (x > 0 && y < 11 && level[y][x].fg == FgElementPlatformStep5)
  {
    if (level[y+1][x-1].dominoType != 0)
    {
      DominoCrash(x-1, y+1, level[y][x].dominoType, level[y][x].dominoExtra);
    }
    else
    {
      level[y+1][x-1].dominoType = level[y][x].dominoType;
      level[y+1][x-1].dominoState = 14;
      level[y+1][x-1].dominoDir = level[y][x].dominoDir;
      level[y+1][x-1].dominoYOffset = -6;
      level[y+1][x-1].dominoExtra = 0x40;
    }

    level[y][x].dominoType = 0;
    level[y][x].dominoState = 0;
    level[y][x].dominoDir = 0;
    level[y][x].dominoYOffset = 0;

    markDirty(x, y);
    markDirty(x-1, y);
    markDirty(x, y-1);
    markDirty(x-1, y-1);

    return;
  }

  if (level[y][x-1].dominoType != 0)
  {
    DominoCrash(x-1, y, level[y][x].dominoType, level[y][x].dominoExtra);
  }
  else
  {
    level[y][x-1].dominoType = DominoTypeTumbler;
    level[y][x-1].dominoState = 14;
    level[y][x-1].dominoDir = level[y][x].dominoDir;
    level[y][x-1].dominoYOffset = 0;
  }

  level[y][x].dominoType = 0;
  level[y][x].dominoDir = 0;

  markDirty(x, y);
  markDirty(x-1, y);
  markDirty(x, y-1);
  markDirty(x-1, y-1);


}

// Tumbler falln down right
void level_c::DTA_L(int x, int y, int x2, int y2) {

  int fg;

  if (x < 19)
    fg = level[y][x+1].fg;
  else
    fg = 0;

  if (fg == FgElementLadder)
    fg = 0;

  if ((level[y][x].fg == FgElementPlatformEnd || level[y][x].fg == FgElementPlatformStrip) && fg == 0)
  {
    if (level[y+1][x+1].dominoType != 0)
    {
      DominoCrash(x+1, y+1, level[y][x].dominoType, level[y][x].dominoExtra);
    }
    else
    {
      level[y][x+1].dominoType = level[y][x].dominoType;
      level[y][x+1].dominoState = 2;
      level[y][x+1].dominoDir = level[y][x].dominoDir;
      level[y][x+1].dominoYOffset = 2;
      level[y][x+1].dominoExtra = 0x70;

// 		mov	ax, 0Ah
// 		push	ax
// 		call	SomethingWithSound
    }

    level[y][x].dominoType = 0;
    level[y][x].dominoState = 0;
    level[y][x].dominoDir = 0;

    markDirty(x, y);
    markDirty(x+1, y);
    markDirty(x, y-1);
    markDirty(x+1, y-1);

    return;
  }

  if (x < 19 && level[y][x].fg == FgElementPlatformStep1 && fg == FgElementPlatformStep2)
  {
    if (level[y][x+1].dominoType != 0)
    {
      DominoCrash(x+1, y, level[y][x].dominoType, level[y][x].dominoExtra);
    }
    else
    {
      level[y][x+1].dominoType = level[y][x].dominoType;
      level[y][x+1].dominoState = 2;
      level[y][x+1].dominoDir = level[y][x].dominoDir;
      level[y][x+1].dominoYOffset = 2;
      level[y][x+1].dominoExtra = 0x40;
    }

    level[y][x].dominoType = 0;
    level[y][x].dominoState = 0;
    level[y][x].dominoDir = 0;
    level[y][x].dominoYOffset = 0;

    markDirty(x, y);
    markDirty(x+1, y);
    markDirty(x, y-1);
    markDirty(x+1, y-1);

    return;
  }

  if (x < 19 && y < 11 && level[y][x].fg == FgElementPlatformStep2)
  {
    if (level[y+1][x+1].dominoType != 0)
    {
      DominoCrash(x+1, y+1, level[y][x].dominoType, level[y][x].dominoExtra);
    }
    else
    {
      level[y+1][x+1].dominoType = level[y][x].dominoType;
      level[y+1][x+1].dominoState = 2;
      level[y+1][x+1].dominoDir = level[y][x].dominoDir;
      level[y+1][x+1].dominoYOffset = -6;
      level[y+1][x+1].dominoExtra = 0x40;
    }

    level[y][x].dominoType = 0;
    level[y][x].dominoState = 0;
    level[y][x].dominoDir = 0;
    level[y][x].dominoYOffset = 0;

    markDirty(x, y);
    markDirty(x+1, y);
    markDirty(x, y-1);
    markDirty(x+1, y-1);

    return;
  }

  if (level[y][x+1].dominoType != 0)
  {
    DominoCrash(x+1, y, level[y][x].dominoType, level[y][x].dominoExtra);
  }
  else
  {
    level[y][x+1].dominoType = DominoTypeTumbler;
    level[y][x+1].dominoState = 2;
    level[y][x+1].dominoDir = level[y][x].dominoDir;
  }

  level[y][x].dominoType = 0;
  level[y][x].dominoDir = 0;

  markDirty(x, y);
  markDirty(x+1, y);
  markDirty(x, y-1);
  markDirty(x+1, y-1);
}

void level_c::callStateFunction(int type, int state, int x, int y, int x2, int y2) {

  switch ((type-1)*17+state-1) {

    // DominoTypeStandard
    case   0: DTA_1(x, y, x2, y2); break;
    case   1: DTA_2(x, y, x2, y2); break;
    case   2: DTA_3(x, y, x2, y2); break;
    case   3: DTA_4(x, y, x2, y2); break;
    case   4: DTA_4(x, y, x2, y2); break;
    case   5: DTA_4(x, y, x2, y2); break;
    case   6: DTA_4(x, y, x2, y2); break;
    case   7: DTA_E(x, y, x2, y2); break;
    case   8: DTA_4(x, y, x2, y2); break;
    case   9: DTA_4(x, y, x2, y2); break;
    case  10: DTA_4(x, y, x2, y2); break;
    case  11: DTA_4(x, y, x2, y2); break;
    case  12: DTA_I(x, y, x2, y2); break;
    case  13: DTA_J(x, y, x2, y2); break;
    case  14: DTA_K(x, y, x2, y2); break;

    // DominoTypeBlocker
    case  24: DTA_F(x, y, x2, y2); break;

    // DominoTypeSplitter
    case  35: DTA_C(x, y, x2, y2); break;
    case  36: DTA_C(x, y, x2, y2); break;
    case  37: DTA_D(x, y, x2, y2); break;
    case  38: DTA_D(x, y, x2, y2); break;
    case  39: DTA_D(x, y, x2, y2); break;
    case  40: DTA_D(x, y, x2, y2); break;
    case  41: DTA_F(x, y, x2, y2); break;
    case  42: DTA_C(x, y, x2, y2); break;
    case  43: DTA_C(x, y, x2, y2); break;
    case  44: DTA_C(x, y, x2, y2); break;
    case  45: DTA_C(x, y, x2, y2); break;
    case  46: DTA_C(x, y, x2, y2); break;
    case  47: DTA_C(x, y, x2, y2); break;

    // DominoTypeExploder
    case  51: DTA_5(x, y, x2, y2); break;
    case  52: DTA_4(x, y, x2, y2); break;
    case  53: DTA_4(x, y, x2, y2); break;
    case  54: DTA_4(x, y, x2, y2); break;
    case  55: DTA_4(x, y, x2, y2); break;
    case  56: DTA_4(x, y, x2, y2); break;
    case  57: DTA_4(x, y, x2, y2); break;
    case  58: DTA_F(x, y, x2, y2); break;

    // DominoTypeDelay
    case  68: DTA_1(x, y, x2, y2); break;
    case  69: DTA_2(x, y, x2, y2); break;
    case  70: DTA_3(x, y, x2, y2); break;
    case  71: DTA_4(x, y, x2, y2); break;
    case  72: DTA_4(x, y, x2, y2); break;
    case  73: DTA_4(x, y, x2, y2); break;
    case  74: DTA_4(x, y, x2, y2); break;
    case  75: DTA_G(x, y, x2, y2); break;
    case  76: DTA_4(x, y, x2, y2); break;
    case  77: DTA_4(x, y, x2, y2); break;
    case  78: DTA_4(x, y, x2, y2); break;
    case  79: DTA_4(x, y, x2, y2); break;
    case  80: DTA_I(x, y, x2, y2); break;
    case  81: DTA_J(x, y, x2, y2); break;
    case  82: DTA_K(x, y, x2, y2); break;

    // DominoTypeTumbler
    case  85: DTA_6(x, y, x2, y2); break;
    case  86: DTA_2(x, y, x2, y2); break;
    case  87: DTA_3(x, y, x2, y2); break;
    case  88: DTA_4(x, y, x2, y2); break;
    case  89: DTA_4(x, y, x2, y2); break;
    case  90: DTA_4(x, y, x2, y2); break;
    case  91: DTA_4(x, y, x2, y2); break;
    case  92: DTA_E(x, y, x2, y2); break;
    case  93: DTA_4(x, y, x2, y2); break;
    case  94: DTA_4(x, y, x2, y2); break;
    case  95: DTA_4(x, y, x2, y2); break;
    case  96: DTA_4(x, y, x2, y2); break;
    case  97: DTA_I(x, y, x2, y2); break;
    case  98: DTA_J(x, y, x2, y2); break;
    case  99: DTA_L(x, y, x2, y2); break;

    // DominoTypeBridger
    case 102: DTA_7(x, y, x2, y2); break;
    case 103: DTA_2(x, y, x2, y2); break;
    case 104: DTA_3(x, y, x2, y2); break;
    case 105: DTA_4(x, y, x2, y2); break;
    case 106: DTA_4(x, y, x2, y2); break;
    case 107: DTA_4(x, y, x2, y2); break;
    case 108: DTA_4(x, y, x2, y2); break;
    case 109: DTA_E(x, y, x2, y2); break;
    case 110: DTA_4(x, y, x2, y2); break;
    case 111: DTA_4(x, y, x2, y2); break;
    case 112: DTA_4(x, y, x2, y2); break;
    case 113: DTA_4(x, y, x2, y2); break;
    case 114: DTA_I(x, y, x2, y2); break;
    case 115: DTA_J(x, y, x2, y2); break;
    case 116: DTA_M(x, y, x2, y2); break;

    // DominoTypeVanisher
    case 119: DTA_8(x, y, x2, y2); break;
    case 120: DTA_4(x, y, x2, y2); break;
    case 121: DTA_3(x, y, x2, y2); break;
    case 122: DTA_4(x, y, x2, y2); break;
    case 123: DTA_4(x, y, x2, y2); break;
    case 124: DTA_4(x, y, x2, y2); break;
    case 125: DTA_4(x, y, x2, y2); break;
    case 126: DTA_E(x, y, x2, y2); break;
    case 127: DTA_4(x, y, x2, y2); break;
    case 128: DTA_4(x, y, x2, y2); break;
    case 129: DTA_4(x, y, x2, y2); break;
    case 130: DTA_4(x, y, x2, y2); break;
    case 131: DTA_I(x, y, x2, y2); break;
    case 132: DTA_4(x, y, x2, y2); break;
    case 133: DTA_8(x, y, x2, y2); break;

    // DominoTypeTrigger
    case 136: DTA_9(x, y, x2, y2); break;
    case 137: DTA_2(x, y, x2, y2); break;
    case 138: DTA_3(x, y, x2, y2); break;
    case 139: DTA_4(x, y, x2, y2); break;
    case 140: DTA_4(x, y, x2, y2); break;
    case 141: DTA_4(x, y, x2, y2); break;
    case 142: DTA_4(x, y, x2, y2); break;
    case 143: DTA_E(x, y, x2, y2); break;
    case 144: DTA_4(x, y, x2, y2); break;
    case 145: DTA_4(x, y, x2, y2); break;
    case 146: DTA_4(x, y, x2, y2); break;
    case 147: DTA_4(x, y, x2, y2); break;
    case 148: DTA_I(x, y, x2, y2); break;
    case 149: DTA_J(x, y, x2, y2); break;
    case 150: DTA_N(x, y, x2, y2); break;

    // DominoTypeRiser
    case 153: DTA_A(x, y, x2, y2); break;
    case 154: DTA_4(x, y, x2, y2); break;
    case 155: DTA_3(x, y, x2, y2); break;
    case 156: DTA_4(x, y, x2, y2); break;
    case 157: DTA_4(x, y, x2, y2); break;
    case 158: DTA_4(x, y, x2, y2); break;
    case 159: DTA_4(x, y, x2, y2); break;
    case 160: DTA_H(x, y, x2, y2); break;
    case 161: DTA_4(x, y, x2, y2); break;
    case 162: DTA_4(x, y, x2, y2); break;
    case 163: DTA_4(x, y, x2, y2); break;
    case 164: DTA_4(x, y, x2, y2); break;
    case 165: DTA_I(x, y, x2, y2); break;
    case 166: DTA_4(x, y, x2, y2); break;
    case 167: DTA_O(x, y, x2, y2); break;
    case 168: DTA_H(x, y, x2, y2); break;
    case 169: DTA_H(x, y, x2, y2); break;

    // DominoTypeCrash0
    case 170: DTA_B(x, y, x2, y2); break;
    case 171: DTA_B(x, y, x2, y2); break;
    case 172: DTA_B(x, y, x2, y2); break;
    case 173: DTA_B(x, y, x2, y2); break;
    case 174: DTA_B(x, y, x2, y2); break;

    // DominoTypeCrash1
    case 187: DTA_B(x, y, x2, y2); break;
    case 188: DTA_B(x, y, x2, y2); break;
    case 189: DTA_B(x, y, x2, y2); break;
    case 190: DTA_B(x, y, x2, y2); break;
    case 191: DTA_B(x, y, x2, y2); break;

    // DominoTypeCrash2
    case 204: DTA_B(x, y, x2, y2); break;
    case 205: DTA_B(x, y, x2, y2); break;
    case 206: DTA_B(x, y, x2, y2); break;
    case 207: DTA_B(x, y, x2, y2); break;
    case 208: DTA_B(x, y, x2, y2); break;

    // DominoTypeCrash3
    case 221: DTA_B(x, y, x2, y2); break;
    case 222: DTA_B(x, y, x2, y2); break;
    case 223: DTA_B(x, y, x2, y2); break;
    case 224: DTA_B(x, y, x2, y2); break;
    case 225: DTA_B(x, y, x2, y2); break;

    // DominoTypeCrash4
    case 238: DTA_B(x, y, x2, y2); break;
    case 239: DTA_B(x, y, x2, y2); break;
    case 240: DTA_B(x, y, x2, y2); break;
    case 241: DTA_B(x, y, x2, y2); break;
    case 242: DTA_B(x, y, x2, y2); break;

    // DominoTypeCrash5
    case 255: DTA_B(x, y, x2, y2); break;
    case 256: DTA_B(x, y, x2, y2); break;
    case 257: DTA_B(x, y, x2, y2); break;
    case 258: DTA_B(x, y, x2, y2); break;
    case 259: DTA_B(x, y, x2, y2); break;

    // DominoTypeRiserCont
    // DominoTypeQuaver
  }
}

void level_c::performDominos(void) {

  for (int y = 0; y < 13; y++)
    for (int x = 0; x < 20; x++)
      if (getDominoType(x, y) != DominoTypeEmpty &&
          getDominoState(x, y) != 0) {

        callStateFunction(getDominoType(x, y), getDominoState(x, y), x, y, x, y);

        if (getDominoType(x, y) == DominoTypeRiser)
        {
          if (isDirty(x-1, y)) markDirty(x-1, y-1);
          if (isDirty(x  , y)) markDirty(x  , y-1);
          if (isDirty(x+1, y)) markDirty(x+1, y-1);
        }
      }
}
