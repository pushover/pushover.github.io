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

void level_c::drawDominos(SDL_Surface * target, graphics_c * gr) {
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

  for (unsigned int y = 0; y < 13; y++)
    for (unsigned int x = 0; x < 20; x++) {
      SDL_Rect dst;
      dst.w = dst.h = 4;
      dst.x = x*gr->blockX();
      dst.y = y*gr->blockY();
      Uint32 color = ((dynamicDirty[y] >> x) & 1)
        ? SDL_MapRGB(target->format, 255, 0, 0)
        : SDL_MapRGB(target->format, 0, 0, 0);
      SDL_FillRect(target, &dst, color);
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

  if (level[y][x].dominoType != 0) {

    if (pushin)
      DominoCrash(x, y, domino, 0);
    else
      DominoCrash(x, y, domino, 0x70);

  } else if (x > 0 && level[y][x-1].dominoType && level[y][x-1].dominoState >= 12) {

    DominoCrash(x, y, domino, 0);

  } else if (x < 19 && level[y][x+1].dominoType && level[y][x-1].dominoState <= 4) {

    DominoCrash(x, y, domino, 0);

  } else {

    // we can place the domino
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

  switch (getDominoType(x, y)) {

    case DominoTypeSplitter:
      if (getDominoDir(x, y) != 0)
      {
        DominoCrash(x, y, level[y][x+dir].dominoYOffset, level[y][x+dir].dominoExtra);
        return false;
      }
      break;

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


    case DominoTypeTumbler:
    case DominoTypeBridger:
    case DominoTypeTrigger:
    case DominoTypeEmpty:
    case DominoTypeStandard:
    case DominoTypeBlocker:
       if (getDominoDir(x, y) == -dir)
       {
         DominoCrash(x, y, level[y][x+dir].dominoYOffset, level[y][x+dir].dominoExtra);
         // crash
         return false;
       }
       break;
  }

  switch(getDominoType(x, y)) {

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

    case DominoTypeSplitter:
      if (getDominoState(x, y) == 8) {
        // play dound
        level[y][x].dominoDir = -1;
        level[y][x].dominoState --;
      }
      break;

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


void level_c::DTA_9(int x, int y, int x2, int y2) {
  DTA_1(x, y, x2, y2);

  triggerFalln = true;
}
void level_c::DTA_N(int x, int y, int x2, int y2) {
  DTA_K(x, y, x2, y2);

  triggerFalln = true;
}
void level_c::DTA_F(int x, int y, int x2, int y2) {
  if (getDominoExtra(x, y) == 0x70)
    DTA_E(x, y, x2, y2);
}
void level_c::DTA_G(int x, int y, int x2, int y2) {
  if (getDominoExtra(x, y) <= 1 || getDominoExtra(x, y) == 0x70)
    DTA_E(x, y, x2, y2);
  else
    level[y][x].dominoExtra--;
}
void level_c::DTA_B(int x, int y, int x2, int y2) {
  DTA_E(x, y, x2, y2);

  markDirty(x2+1, y2);
  markDirty(x2+1, y2-1);
}
void level_c::DTA_D(int x, int y, int x2, int y2) {
  markDirty(x2+1, y2-1);
  markDirty(x2+1, y2);

  DTA_4(x, y, x2, y2);

  if (getDominoState(x, y) == 5)
    markDirty(x2, y2-2);
}
void level_c::DTA_8(int x, int y, int x2, int y2) {
  markDirty(x2, y2);
  markDirty(x2+getDominoDir(x, y), y2);
  markDirty(x2, y2-1);
  markDirty(x2+getDominoDir(x, y), y2-1);

  level[y][x].dominoType = DominoTypeEmpty;
  level[y][x].dominoDir = 0;
  level[y][x].dominoState = 0;
}
void level_c::DTA_2(int x, int y, int x2, int y2) {

  if (getDominoExtra(x, y) == 0x40 || getDominoExtra(x, y) == 0x70)
  {
    DTA_4(x, y, x2, y2);
    return;
  }

  if (getDominoDir(x, y) == 1)
  {
    DTA_4(x, y, x2, y2);
    return;
  }

  if (getDominoType(x-1, y) != 0)
    return;

  if (x >= 2 && getDominoType(x-2, y) != 0 && getDominoState(x-2, y) > 13)
    return;

  if (getDominoType(x-2, y) != DominoTypeSplitter ||
      getDominoState(x-2, y) != 1 &&
      getDominoState(x-2, y) != 10 &&
      getDominoState(x-2, y) != 12 &&
      getDominoState(x-2, y) != 13)
  {
    DTA_4(x, y, x2, y2);
  }
}
void level_c::DTA_J(int x, int y, int x2, int y2) {

  if (getDominoExtra(x, y) == 0x40 || getDominoExtra(x, y) == 0x70)
  {
    DTA_4(x, y, x2, y2);
    return;
  }

  if (getDominoDir(x, y) == -1)
  {
    DTA_4(x, y, x2, y2);
    return;
  }

  if (getDominoType(x+1, y) != 0)
    return;

  if (x <= 17 && getDominoType(x+2, y) != 0 && getDominoState(x+2, y) < 3)
    return;

  if (getDominoType(x+2, y) != DominoTypeSplitter ||
      getDominoState(x+2, y) != 1 &&
      getDominoState(x+2, y) != 9 &&
      getDominoState(x+2, y) != 11 &&
      getDominoState(x+2, y) != 14)
  {
    DTA_4(x, y, x2, y2);
  }
}
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

  level[y][x].dominoState += getDominoDir(x, y);

  if (getDominoType(x, y) == DominoTypeRiser &&
      getDominoState(x, y) == 8 &&
      level[y][x].dominoYOffset == -10)
  {
    level[y][x].dominoState = 16;
  }

  markDirty(x, y);

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
void level_c::DTA_3(int x, int y, int x2, int y2) {

  if (x2 > 0 && getFg(x2-1, y2) == FgElementPlatformStep4)
    return;

  if (level[y][x].dominoYOffset == 8 && x2 > 0 && getFg(x2-1, y2) == FgElementPlatformStep1)
    return;

  if (getDominoType(x-1, y) == DominoTypeEmpty) {
    DTA_4(x, y, x2, y2);
    return;
  }

  if (getDominoType(x2-1, y2) != DominoTypeBlocker &&
      getDominoType(x2-1, y2) != DominoTypeDelay)
  {
    if (pushDomino(x2-1, y2, -1))
      DTA_4(x, y, x2, y2);
    return;
  }

  if (getDominoType(x-1, y) == DominoTypeDelay)
  {
    if (getDominoState(x-1, y) != 8) {
      DTA_4(x, y, x2, y2);
      return;
    }
  }

  if (getDominoType(x+1, y) == DominoTypeSplitter && getDominoState(x+1, y) != 8)
  {
    if (pushDomino(x2-1, y2, -1))
      DTA_4(x, y, x2, y2);
    return;
  }

  if (getDominoType(x+1, y) != DominoTypeEmpty && getDominoState(x+1, y) < 8)
  {
    if (pushDomino(x2-1, y2, -1))
      DTA_4(x, y, x2, y2);
    return;
  }

  level[y][x].dominoDir = 1;
  pushDomino(x2-1, y2, -1);
  DTA_4(x, y, x2, y2);
}

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

void level_c::DominoCrash(int x, int y, int type, int extra) {

    int next = level[y][x].dominoType;

    if ((next == DominoTypeStandard || next == DominoTypeCrash4) &&
        (type == DominoTypeStandard || type == DominoTypeCrash4))
    {
        next = DominoTypeCrash4;
    }
    else if ((next == DominoTypeBlocker || next == DominoTypeCrash3) &&
             (type == DominoTypeBlocker || type == DominoTypeCrash3))
    {
        next = DominoTypeCrash3;
    }
    else
    {
        next = DominoTypeCrash2;
    }

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

  if (level[y][x].dominoExtra != 0x70)
  {
    if (level[y][x].dominoDir != 0)
    {
      DTA_4(x, y, x2, y2);
    }
    return;
  }

  if (level[y][x].dominoYOffset != 0)
  {
    if (level[y][x].dominoYOffset != 12) {
      level[y][x].dominoYOffset += 4;
      markDirty(x, y+1);
      markDirty(x, y);
      return;
    }

    if (y < 12)
    {
      level[y+1][x].dominoType = level[y][x].dominoType;
      level[y+1][x].dominoState = level[y][x].dominoState;
      level[y+1][x].dominoDir = level[y][x].dominoDir;
      level[y+1][x].dominoYOffset = 0;
      level[y+1][x].dominoExtra = 0x70;
    }

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

  if (isTherePlatform(x, y))
  {
    if (getDominoType(x, y+1) != DominoTypeEmpty)
    {
        DominoCrash(x, y+1, level[y][x].dominoType, level[y][x].dominoExtra);
    }

    level[y][x].dominoExtra = 0;

    if (level[y][x].fg == 8 || level[y][x].fg == 11)
    {
      level[y][x].dominoYOffset = 8;
    }

    markDirty(x, y);
    markDirty(x, y+1);
    return;
  }

  if (y >= 12 || level[y+1][x].dominoType == DominoTypeEmpty)
  {
    level[y][x].dominoYOffset += 4;
    markDirty(x, y);
    markDirty(x, y+1);
    return;
  }

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

    int a = SplitterLookup[level[y][x].dominoState*2+1];
    int b = SplitterLookup[level[y][x].dominoState*2];

    if (a == 3)
    {
        if (x > 0 && level[y][x-1].fg != 0xA)
        {
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

    printf("oops missing splitter image");
}

// bridger left
void level_c::DTA_7(int x, int y, int x2, int y2) {
// var_8		= word ptr -8
// var_6		= byte ptr -6
// var_4		= byte ptr -4
// var_2		= byte ptr -2
// Xpos		= word ptr  4
// Ypos		= word ptr  6
// LeveldataEntry	= word ptr  8
//
// 		push	bp
// 		mov	bp, sp
// 		sub	sp, 8
// 		push	si
// 		mov	[bp+var_8], 0
// 		cmp	[bp+Xpos], 2
// 		jl	loc_72FA
//
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		mov	al, (AntAnimBArray+2Eh)[bx+si] ; 6 ant sprites
// 		jmp	short loc_72FC
//
// loc_72FA:				; CODE XREF: DTA_7+10j
// 		sub	al, al
//
// loc_72FC:				; CODE XREF: DTA_7+25j
// 		mov	[bp+var_2], al
// 		cmp	[bp+Xpos], 1
// 		jl	loc_731A
//
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		mov	al, (AntAnimBArray+36h)[bx+si] ; 6 ant sprites
// 		jmp	short loc_731C
//
// loc_731A:				; CODE XREF: DTA_7+31j
// 		sub	al, al
//
// loc_731C:				; CODE XREF: DTA_7+46j
// 		mov	[bp+var_4], al
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bx+2]
// 		mov	[bp+var_6], al
// 		cmp	al, 1
// 		jz	loc_7333
//
// 		cmp	al, 12h
// 		jz	loc_7333
//
// 		jmp	loc_741E
//
// loc_7333:				; CODE XREF: DTA_7+58j	DTA_7+5Cj
// 		cmp	[bp+var_4], 3
// 		jz	loc_7349
//
// 		cmp	[bp+var_4], 0
// 		jnz	loc_7350
//
// 		cmp	[bp+var_2], 3
// 		jnz	loc_7350
//
// 		mov	[bp+var_2], 2
//
// loc_7349:				; CODE XREF: DTA_7+65j
// 		mov	[bp+var_4], 2
// 		jmp	short loc_735A
//
// loc_7350:				; CODE XREF: DTA_7+6Bj	DTA_7+71j
// 		cmp	[bp+var_4], 12h
// 		jnz	loc_7364
//
// 		mov	[bp+var_4], 1
//
// loc_735A:				; CODE XREF: DTA_7+7Bj
// 		cmp	[bp+var_6], 1
// 		jnz	loc_737E
//
//
// loc_7360:				; CODE XREF: DTA_7+AAj
// 		mov	al, 2
// 		jmp	short loc_7380
//
// loc_7364:				; CODE XREF: DTA_7+82j
// 		cmp	[bp+var_4], 0
// 		jnz	loc_7388
//
// 		cmp	[bp+var_2], 12h
// 		jnz	loc_7388
//
// 		mov	[bp+var_2], 1
// 		mov	[bp+var_4], 2
// 		cmp	[bp+var_6], 1
// 		jz	loc_7360
//
//
// loc_737E:				; CODE XREF: DTA_7+8Cj
// 		mov	al, 3
//
// loc_7380:				; CODE XREF: DTA_7+90j
// 		mov	[bp+var_6], al
// 		mov	[bp+var_8], 1
//
// loc_7388:				; CODE XREF: DTA_7+96j	DTA_7+9Cj
// 		cmp	[bp+var_8], 0
// 		jnz	loc_7391
//
// 		jmp	loc_741E
//
// loc_7391:				; CODE XREF: DTA_7+BAj
// 		cmp	[bp+Xpos], 2
// 		jl	loc_73BD
//
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		mov	al, [bp+var_2]
// 		mov	(AntAnimBArray+2Eh)[bx+si], al ; 6 ant sprites
// 		mov	ax, 14h
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	byte ptr [bx+si+7A0h], 1
//
// loc_73BD:				; CODE XREF: DTA_7+C3j
// 		cmp	[bp+Xpos], 1
// 		jl	loc_73D9
//
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		mov	al, [bp+var_4]
// 		mov	(AntAnimBArray+36h)[bx+si], al ; probalby LevelDat_srcPtr_6
//
// loc_73D9:				; CODE XREF: DTA_7+EFj
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bp+var_6]
// 		mov	[bx+2],	al
// 		mov	byte ptr [bx+3], 0
// 		mov	byte ptr [bx+5], 0
// 		mov	ax, 14h
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		add	si, [bp+Xpos]
// 		mov	(DirtyBlocksPrefix+13h)[si], 1 ; 2 additional lines of direty blocks
// 					; to access blocks above the screen without additional checks
// 		mov	(DirtyBlocksPrefix+14h)[si], 1 ; 2 additional lines of direty blocks
// 					; to access blocks above the screen without additional checks
// 		mov	(DirtyBlocksPrefix+27h)[si], 1 ; 2 additional lines of direty blocks
// 					; to access blocks above the screen without additional checks
// 		mov	DirtyBlocks[si], 1 ; changed blocks on screen, there are 20 blocks per row and 13 blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
// 		mov	SecondPageDirty[si], 1 ; if set	then we	can NOT	copy the static	level from
// 					; background probably because it changed
// 		mov	(AntAnimDArray+4Fh)[si], 1 ; dirty page	-1
// 		mov	(AntAnimDArray+4Eh)[si], 1 ; dirty page	-2
// 		pop	si
// 		mov	sp, bp
// 		pop	bp
// 		retn
//
// loc_741E:				; CODE XREF: DTA_7+5Ej	DTA_7+BCj
// 		push	[bp+LeveldataEntry]
// 		push	[bp+Ypos]
// 		push	[bp+Xpos]
// 		call	DTA_1
//
// 		add	sp, 6
// 		pop	si
// 		mov	sp, bp
// 		pop	bp
// 		retn
}

// Brider right
void level_c::DTA_M(int x, int y, int x2, int y2) {

// var_8		= word ptr -8
// var_6		= byte ptr -6
// var_4		= byte ptr -4
// CurrentLevelForeground=	byte ptr -2
// Xpos		= word ptr  4
// Ypos		= word ptr  6
// LeveldataEntry	= word ptr  8
//
// 		push	bp
// 		mov	bp, sp
// 		sub	sp, 8
// 		push	si
// 		mov	[bp+var_8], 0
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bx+LevelDataForeground]
// 		mov	[bp+CurrentLevelForeground], al
// 		cmp	[bp+Xpos], 18
// 		jg	loc_7462
//
// 		mov	ax, 160
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		mov	al, (LevelData_srcPtr+0Ah)[bx+si] ; 260	tiles per level, 8 bytes per tile
// 					;
// 					; 2 bytes background
// 					; 1 bytes foreground
// 					; 1 Byte domino	type
// 					; 1 Byte domino	state (animation state
// 					; 1 Byte Y-Offset
// 		jmp	short loc_7464
//
// loc_7462:				; CODE XREF: DTA_M+19j
// 		sub	al, al
//
// loc_7464:				; CODE XREF: DTA_M+2Ej
// 		mov	[bp+var_4], al
// 		cmp	[bp+Xpos], 11h
// 		jg	loc_7482
//
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		mov	al, (LevelData_srcPtr+12h)[bx+si] ; 260	tiles per level, 8 bytes per tile
// 					;
// 					; 2 bytes background
// 					; 1 bytes foreground
// 					; 1 Byte domino	type
// 					; 1 Byte domino	state (animation state
// 					; 1 Byte Y-Offset
// 		jmp	short loc_7484
//
// loc_7482:				; CODE XREF: DTA_M+39j
// 		sub	al, al
//
// loc_7484:				; CODE XREF: DTA_M+4Ej
// 		mov	[bp+var_6], al
// 		cmp	[bp+CurrentLevelForeground], 3
// 		jz	loc_7496
//
// 		cmp	[bp+CurrentLevelForeground], 12h
// 		jz	loc_7496
//
// 		jmp	loc_75B4
//
// loc_7496:				; CODE XREF: DTA_M+59j	DTA_M+5Fj
// 		cmp	[bp+var_4], 1
// 		jnz	loc_74B2
//
// 		cmp	[bp+CurrentLevelForeground], 3
// 		jnz	loc_74A6
//
// 		mov	al, 2
// 		jmp	short loc_74A8
//
// loc_74A6:				; CODE XREF: DTA_M+6Ej
// 		mov	al, 1
//
// loc_74A8:				; CODE XREF: DTA_M+72j
// 		mov	[bp+CurrentLevelForeground], al
// 		mov	[bp+var_4], 2
// 		jmp	short loc_7517
//
// loc_74B2:				; CODE XREF: DTA_M+68j
// 		cmp	[bp+var_4], 0
// 		jnz	loc_74D8
//
// 		cmp	[bp+var_6], 1
// 		jnz	loc_74D8
//
// 		cmp	[bp+CurrentLevelForeground], 3
// 		jnz	loc_74C8
//
// 		mov	al, 2
// 		jmp	short loc_74CA
//
// loc_74C8:				; CODE XREF: DTA_M+90j
// 		mov	al, 1
//
// loc_74CA:				; CODE XREF: DTA_M+94j
// 		mov	[bp+CurrentLevelForeground], al
// 		mov	[bp+var_4], 2
// 		mov	[bp+var_6], 2
// 		jmp	short loc_7517
//
// loc_74D8:				; CODE XREF: DTA_M+84j	DTA_M+8Aj
// 		cmp	[bp+var_4], 12h
// 		jnz	loc_74F4
//
// 		cmp	[bp+CurrentLevelForeground], 3
// 		jnz	loc_74E8
//
// 		mov	al, 2
// 		jmp	short loc_74EA
//
// loc_74E8:				; CODE XREF: DTA_M+B0j
// 		mov	al, 1
//
// loc_74EA:				; CODE XREF: DTA_M+B4j
// 		mov	[bp+CurrentLevelForeground], al
// 		mov	[bp+var_4], 3
// 		jmp	short loc_7517
//
// loc_74F4:				; CODE XREF: DTA_M+AAj
// 		cmp	[bp+var_4], 0
// 		jnz	loc_751C
//
// 		cmp	[bp+var_6], 12h
// 		jnz	loc_751C
//
// 		cmp	[bp+CurrentLevelForeground], 3
// 		jnz	loc_750A
//
// 		mov	al, 2
// 		jmp	short loc_750C
//
// loc_750A:				; CODE XREF: DTA_M+D2j
// 		mov	al, 1
//
// loc_750C:				; CODE XREF: DTA_M+D6j
// 		mov	[bp+CurrentLevelForeground], al
// 		mov	[bp+var_4], 2
// 		mov	[bp+var_6], 3
//
// loc_7517:				; CODE XREF: DTA_M+7Dj	DTA_M+A3j ...
// 		mov	[bp+var_8], 1
//
// loc_751C:				; CODE XREF: DTA_M+C6j	DTA_M+CCj
// 		cmp	[bp+var_8], 0
// 		jnz	loc_7525
//
// 		jmp	loc_75B4
//
// loc_7525:				; CODE XREF: DTA_M+EEj
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bp+CurrentLevelForeground]
// 		mov	[bx+2],	al
// 		cmp	[bp+Xpos], 12h
// 		jg	loc_754A
//
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		mov	al, [bp+var_4]
// 		mov	(LevelData_srcPtr+0Ah)[bx+si], al ; 260	tiles per level, 8 bytes per tile
// 					;
// 					; 2 bytes background
// 					; 1 bytes foreground
// 					; 1 Byte domino	type
// 					; 1 Byte domino	state (animation state
// 					; 1 Byte Y-Offset
//
// loc_754A:				; CODE XREF: DTA_M+100j
// 		cmp	[bp+Xpos], 11h
// 		jg	loc_7576
//
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		mov	al, [bp+var_6]
// 		mov	[bx+si+4B0Ah], al
// 		mov	ax, 14h
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	byte ptr [bx+si+7A4h], 1
//
// loc_7576:				; CODE XREF: DTA_M+11Cj
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	byte ptr [bx+3], 0
// 		mov	byte ptr [bx+5], 0
// 		mov	ax, 14h
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		add	si, [bp+Xpos]
// 		mov	(DirtyBlocksPrefix+15h)[si], 1 ; 2 additional lines of direty blocks
// 					; to access blocks above the screen without additional checks
// 		mov	(DirtyBlocksPrefix+14h)[si], 1 ; 2 additional lines of direty blocks
// 					; to access blocks above the screen without additional checks
// 		mov	(DirtyBlocks+1)[si], 1 ; changed blocks	on screen, there are 20	blocks per row and 13 blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
// 		mov	DirtyBlocks[si], 1 ; changed blocks on screen, there are 20 blocks per row and 13 blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
// 		mov	SecondPageDirty[si], 1 ; if set	then we	can NOT	copy the static	level from
// 					; background probably because it changed
// 		mov	(SecondPageDirty+1)[si], 1 ; if	set then we can	NOT copy the static level from
// 					; background probably because it changed
// 		mov	(SecondPageDirty+2)[si], 1 ; if	set then we can	NOT copy the static level from
// 					; background probably because it changed
// 		pop	si
// 		mov	sp, bp
// 		pop	bp
// 		retn
//
// loc_75B4:				; CODE XREF: DTA_M+61j	DTA_M+F0j
// 		push	[bp+LeveldataEntry]
// 		push	[bp+Ypos]
// 		push	[bp+Xpos]
// 		call	DTA_K
//
// 		add	sp, 6
// 		pop	si
// 		mov	sp, bp
// 		pop	bp
// 		retn

}


// riser
void level_c::DTA_A(int x, int y, int x2, int y2) {

// var_6		= word ptr -6
// var_4		= byte ptr -4
// var_2		= byte ptr -2
// Xpos		= word ptr  4
// Ypos		= word ptr  6
// LeveldataEntry	= word ptr  8
//
// 		push	bp
// 		mov	bp, sp
// 		sub	sp, 6
// 		push	di
// 		push	si
// 		mov	bx, [bp+LeveldataEntry]
// 		cmp	byte ptr [bx+7], 50h ; 'P'
// 		jnz	loc_7664
//
// 		mov	[bp+var_6], 1
// 		jmp	short loc_7669
//
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//
// loc_7664:				; CODE XREF: DTA_A+Fj
// 		mov	[bp+var_6], 2
//
// loc_7669:				; CODE XREF: DTA_A+16j
// 		cmp	[bp+Xpos], 0
// 		jle	loc_768A
//
// 		mov	ax, [bp+Ypos]
// 		sub	ax, [bp+var_6]
// 		mov	cx, 0A0h ; ' '
// 		imul	cx
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		mov	al, [bx+si+4AF2h]
// 		jmp	short loc_768C
//
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// 		align 2
//
// loc_768A:				; CODE XREF: DTA_A+21j
// 		sub	al, al
//
// loc_768C:				; CODE XREF: DTA_A+3Bj
// 		mov	[bp+var_2], al
// 		mov	ax, [bp+Ypos]
// 		sub	ax, [bp+var_6]
// 		mov	cx, 0A0h ; ' '
// 		imul	cx
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		mov	al, [bx+si+4AFAh]
// 		mov	[bp+var_4], al
// 		cmp	al, 1
// 		jz	loc_76B5
//
// 		cmp	al, 12h
// 		jz	loc_76B5
//
// 		jmp	loc_776C
//
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//
// loc_76B5:				; CODE XREF: DTA_A+60j	DTA_A+64j
// 		cmp	[bp+var_2], 0
// 		jz	loc_76BE
//
// 		jmp	loc_776C
//
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//
// loc_76BE:				; CODE XREF: DTA_A+6Dj
// 		cmp	[bp+var_6], 1
// 		jnz	loc_76F6
//
// 		mov	si, [bp+Xpos]
// 		mov	cl, 3
// 		shl	si, cl
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		add	si, ax
// 		mov	(AntAnimBArray+3Bh)[si], 60h ; '`' ; 6 ant sprites
// 		mov	(AntAnimBArray+37h)[si], 0Ah ; 6 ant sprites
// 		mov	(AntAnimBArray+38h)[si], 0Eh ; 6 ant sprites
// 		mov	(AntAnimBArray+39h)[si], 0FFh ;	6 ant sprites
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bx+6]
// 		sub	al, 2
// 		mov	[si+4AF6h], al
// 		jmp	short loc_772B
//
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// 		align 2
//
// loc_76F6:				; CODE XREF: DTA_A+76j
// 		cmp	[bp+Ypos], 0
// 		jle	loc_772B
//
// 		mov	si, [bp+Xpos]
// 		mov	cl, 3
// 		shl	si, cl
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		add	si, ax
// 		mov	(AntAnimKArray+4Fh)[si], 60h ; '`' ; 15 ant sprites
// 		mov	(AntAnimKArray+4Bh)[si], 0Ah ; 15 ant sprites
// 		mov	(AntAnimKArray+4Ch)[si], 0Eh ; 15 ant sprites
// 		mov	(AntAnimKArray+4Dh)[si], 0FFh ;	15 ant sprites
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bx+6]
// 		add	al, 0Eh
// 		mov	[si+4A56h], al
//
// loc_772B:				; CODE XREF: DTA_A+A7j	DTA_A+AEj
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	byte ptr [bx+3], 0
// 		mov	byte ptr [bx+4], 0
// 		mov	byte ptr [bx+5], 0
// 		mov	byte ptr [bx+6], 0
// 		mov	byte ptr [bx+7], 0
// 		mov	ax, [bp+Ypos]
// 		sub	ax, [bp+var_6]
// 		mov	cx, 14h
// 		imul	cx
// 		mov	si, ax
// 		add	si, [bp+Xpos]
// 		mov	(DirtyBlocks+14h)[si], 1 ; changed blocks on screen, there are 20 blocks per row and 13	blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
// 		mov	DirtyBlocks[si], 1 ; changed blocks on screen, there are 20 blocks per row and 13 blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
// 		mov	(DirtyBlocks+13h)[si], 1 ; changed blocks on screen, there are 20 blocks per row and 13	blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
// 		mov	byte ptr [si+7A1h], 1
// 		pop	si
// 		pop	di
// 		mov	sp, bp
// 		pop	bp
// 		retn
//
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//
// loc_776C:				; CODE XREF: DTA_A+66j	DTA_A+6Fj
// 		cmp	[bp+var_4], 0
// 		jnz	loc_77DA
//
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	byte ptr [bx+3], 0
// 		mov	byte ptr [bx+4], 0
// 		mov	byte ptr [bx+5], 0
// 		mov	byte ptr [bx+6], 0
// 		mov	byte ptr [bx+7], 0
// 		mov	si, [bp+Ypos]
// 		sub	si, [bp+var_6]
// 		mov	di, [bp+Xpos]
// 		mov	cl, 3
// 		shl	di, cl
// 		mov	ax, 0A0h ; ' '
// 		imul	si
// 		add	di, ax
// 		mov	(LevelData_srcPtr+0A7h)[di], 60h ; '`' ; 260 tiles per level, 8 bytes per tile
// 					;
// 					; 2 bytes background
// 					; 1 bytes foreground
// 					; 1 Byte domino	type
// 					; 1 Byte domino	state (animation state
// 					; 1 Byte Y-Offset
// 		mov	(LevelData_srcPtr+0A3h)[di], 0Ah ; 260 tiles per level,	8 bytes	per tile
// 					;
// 					; 2 bytes background
// 					; 1 bytes foreground
// 					; 1 Byte domino	type
// 					; 1 Byte domino	state (animation state
// 					; 1 Byte Y-Offset
// 		mov	(LevelData_srcPtr+0A4h)[di], 8 ; 260 tiles per level, 8	bytes per tile
// 					;
// 					; 2 bytes background
// 					; 1 bytes foreground
// 					; 1 Byte domino	type
// 					; 1 Byte domino	state (animation state
// 					; 1 Byte Y-Offset
// 		mov	(LevelData_srcPtr+0A5h)[di], 0FFh ; 260	tiles per level, 8 bytes per tile
// 					;
// 					; 2 bytes background
// 					; 1 bytes foreground
// 					; 1 Byte domino	type
// 					; 1 Byte domino	state (animation state
// 					; 1 Byte Y-Offset
// 		mov	(LevelData_srcPtr+0A6h)[di], 0 ; 260 tiles per level, 8	bytes per tile
// 					;
// 					; 2 bytes background
// 					; 1 bytes foreground
// 					; 1 Byte domino	type
// 					; 1 Byte domino	state (animation state
// 					; 1 Byte Y-Offset
// 		mov	ax, 14h
// 		imul	si
// 		mov	di, ax
// 		add	di, [bp+Xpos]
// 		mov	(DirtyBlocks+14h)[di], 1 ; changed blocks on screen, there are 20 blocks per row and 13	blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
// 		mov	DirtyBlocks[di], 1 ; changed blocks on screen, there are 20 blocks per row and 13 blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
// 		mov	(DirtyBlocks+13h)[di], 1 ; changed blocks on screen, there are 20 blocks per row and 13	blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
// 		mov	byte ptr [di+7A1h], 1
// 		pop	si
// 		pop	di
// 		mov	sp, bp
// 		pop	bp
// 		retn
//
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//
// loc_77DA:				; CODE XREF: DTA_A+124j
// 		mov	bx, [bp+LeveldataEntry]
// 		cmp	byte ptr [bx+7], 50h ; 'P'
// 		jz	loc_7829
//
// 		mov	si, [bp+Xpos]
// 		mov	cl, 3
// 		shl	si, cl
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		add	si, ax
// 		mov	al, [bx+3]
// 		mov	(AntAnimKArray+53h)[si], al ; 15 ant sprites
// 		mov	al, [bx+4]
// 		mov	(AntAnimKArray+54h)[si], al ; 15 ant sprites
// 		mov	al, [bx+5]
// 		mov	(AntAnimKArray+55h)[si], al ; 15 ant sprites
// 		mov	al, [bx+6]
// 		add	al, 10h
// 		mov	(AntAnimKArray+56h)[si], al ; 15 ant sprites
// 		mov	(AntAnimKArray+57h)[si], 50h ; 'P' ; 15 ant sprites
// 		mov	byte ptr [bx+3], 0
// 		mov	byte ptr [bx+4], 0
// 		mov	byte ptr [bx+5], 0
// 		mov	byte ptr [bx+6], 0
// 		mov	byte ptr [bx+7], 0
//
// loc_7829:				; CODE XREF: DTA_A+195j
// 		pop	si
// 		pop	di
// 		mov	sp, bp
// 		pop	bp
// 		retn
}


// Riser
void level_c::DTA_O(int x, int y, int x2, int y2) {

// var_6		= word ptr -6
// var_4		= byte ptr -4
// var_2		= byte ptr -2
// Xpos		= word ptr  4
// Ypos		= word ptr  6
// LeveldataEntry	= word ptr  8
//
// 		push	bp
// 		mov	bp, sp
// 		sub	sp, 6
// 		push	di
// 		push	si
// 		mov	bx, [bp+LeveldataEntry]
// 		cmp	byte ptr [bx+7], 50h ; 'P'
// 		jnz	loc_7A40
//
// 		mov	[bp+var_6], 1
// 		jmp	short loc_7A45
//
// loc_7A40:				; CODE XREF: DTA_O+Fj
// 		mov	[bp+var_6], 2
//
// loc_7A45:				; CODE XREF: DTA_O+16j
// 		cmp	[bp+Xpos], 13h
// 		jge	loc_7A66
//
// 		mov	ax, [bp+Ypos]
// 		sub	ax, [bp+var_6]
// 		mov	cx, 0A0h ; ' '
// 		imul	cx
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		mov	al, (LevelData_srcPtr+0Ah)[bx+si] ; 260	tiles per level, 8 bytes per tile
// 					;
// 					; 2 bytes background
// 					; 1 bytes foreground
// 					; 1 Byte domino	type
// 					; 1 Byte domino	state (animation state
// 					; 1 Byte Y-Offset
// 		jmp	short loc_7A68
//
// loc_7A66:				; CODE XREF: DTA_O+21j
// 		sub	al, al
//
// loc_7A68:				; CODE XREF: DTA_O+3Bj
// 		mov	[bp+var_2], al
// 		mov	ax, [bp+Ypos]
// 		sub	ax, [bp+var_6]
// 		mov	cx, 0A0h ; ' '
// 		imul	cx
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		mov	al, (LevelData_srcPtr+2)[bx+si]	; 260 tiles per	level, 8 bytes per tile
// 					;
// 					; 2 bytes background
// 					; 1 bytes foreground
// 					; 1 Byte domino	type
// 					; 1 Byte domino	state (animation state
// 					; 1 Byte Y-Offset
// 		mov	[bp+var_4], al
// 		cmp	al, cl
// 		jz	loc_7A91
//
// 		cmp	al, 12h
// 		jz	loc_7A91
//
// 		jmp	loc_7B48
//
// loc_7A91:				; CODE XREF: DTA_O+60j	DTA_O+64j
// 		cmp	[bp+var_2], 0
// 		jz	loc_7A9A
//
// 		jmp	loc_7B48
//
// loc_7A9A:				; CODE XREF: DTA_O+6Dj
// 		cmp	[bp+var_6], 1
// 		jnz	loc_7AD2
//
// 		mov	si, [bp+Xpos]
// 		mov	cl, 3
// 		shl	si, cl
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		add	si, ax
// 		mov	(LevelData_srcPtr+0Fh)[si], 60h	; '`' ; 260 tiles per level, 8 bytes per tile
// 					;
// 					; 2 bytes background
// 					; 1 bytes foreground
// 					; 1 Byte domino	type
// 					; 1 Byte domino	state (animation state
// 					; 1 Byte Y-Offset
// 		mov	(LevelData_srcPtr+0Bh)[si], 0Ah	; 260 tiles per	level, 8 bytes per tile
// 					;
// 					; 2 bytes background
// 					; 1 bytes foreground
// 					; 1 Byte domino	type
// 					; 1 Byte domino	state (animation state
// 					; 1 Byte Y-Offset
// 		mov	(LevelData_srcPtr+0Ch)[si], 2 ;	260 tiles per level, 8 bytes per tile
// 					;
// 					; 2 bytes background
// 					; 1 bytes foreground
// 					; 1 Byte domino	type
// 					; 1 Byte domino	state (animation state
// 					; 1 Byte Y-Offset
// 		mov	(LevelData_srcPtr+0Dh)[si], 1 ;	260 tiles per level, 8 bytes per tile
// 					;
// 					; 2 bytes background
// 					; 1 bytes foreground
// 					; 1 Byte domino	type
// 					; 1 Byte domino	state (animation state
// 					; 1 Byte Y-Offset
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bx+6]
// 		sub	al, 2
// 		mov	(LevelData_srcPtr+0Eh)[si], al ; 260 tiles per level, 8	bytes per tile
// 					;
// 					; 2 bytes background
// 					; 1 bytes foreground
// 					; 1 Byte domino	type
// 					; 1 Byte domino	state (animation state
// 					; 1 Byte Y-Offset
// 		jmp	short loc_7B07
//
//
// loc_7AD2:				; CODE XREF: DTA_O+76j
// 		cmp	[bp+Ypos], 0
// 		jle	loc_7B07
//
// 		mov	si, [bp+Xpos]
// 		mov	cl, 3
// 		shl	si, cl
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		add	si, ax
// 		mov	byte ptr [si+4A67h], 60h ; '`'
// 		mov	byte ptr [si+4A63h], 0Ah
// 		mov	byte ptr [si+4A64h], 2
// 		mov	byte ptr [si+4A65h], 1
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bx+6]
// 		add	al, 0Eh
// 		mov	[si+4A66h], al
//
// loc_7B07:				; CODE XREF: DTA_O+A7j	DTA_O+AEj
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	byte ptr [bx+3], 0
// 		mov	byte ptr [bx+4], 0
// 		mov	byte ptr [bx+5], 0
// 		mov	byte ptr [bx+6], 0
// 		mov	byte ptr [bx+7], 0
// 		mov	ax, [bp+Ypos]
// 		sub	ax, [bp+var_6]
// 		mov	cx, 14h
// 		imul	cx
// 		mov	si, ax
// 		add	si, [bp+Xpos]
// 		mov	(DirtyBlocks+14h)[si], 1 ; changed blocks on screen, there are 20 blocks per row and 13	blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
// 		mov	DirtyBlocks[si], 1 ; changed blocks on screen, there are 20 blocks per row and 13 blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
// 		mov	(DirtyBlocks+15h)[si], 1 ; changed blocks on screen, there are 20 blocks per row and 13	blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
// 		mov	(DirtyBlocks+1)[si], 1 ; changed blocks	on screen, there are 20	blocks per row and 13 blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
// 		pop	si
// 		pop	di
// 		mov	sp, bp
// 		pop	bp
// 		retn
//
// loc_7B48:				; CODE XREF: DTA_O+66j	DTA_O+6Fj
// 		cmp	[bp+var_4], 0
// 		jnz	loc_7BB6
//
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	byte ptr [bx+3], 0
// 		mov	byte ptr [bx+4], 0
// 		mov	byte ptr [bx+5], 0
// 		mov	byte ptr [bx+6], 0
// 		mov	byte ptr [bx+7], 0
// 		mov	si, [bp+Ypos]
// 		sub	si, [bp+var_6]
// 		mov	di, [bp+Xpos]
// 		mov	cl, 3
// 		shl	di, cl
// 		mov	ax, 0A0h ; ' '
// 		imul	si
// 		add	di, ax
// 		mov	(LevelData_srcPtr+0A7h)[di], 60h ; '`' ; 260 tiles per level, 8 bytes per tile
// 					;
// 					; 2 bytes background
// 					; 1 bytes foreground
// 					; 1 Byte domino	type
// 					; 1 Byte domino	state (animation state
// 					; 1 Byte Y-Offset
// 		mov	(LevelData_srcPtr+0A3h)[di], 0Ah ; 260 tiles per level,	8 bytes	per tile
// 					;
// 					; 2 bytes background
// 					; 1 bytes foreground
// 					; 1 Byte domino	type
// 					; 1 Byte domino	state (animation state
// 					; 1 Byte Y-Offset
// 		mov	(LevelData_srcPtr+0A4h)[di], 8 ; 260 tiles per level, 8	bytes per tile
// 					;
// 					; 2 bytes background
// 					; 1 bytes foreground
// 					; 1 Byte domino	type
// 					; 1 Byte domino	state (animation state
// 					; 1 Byte Y-Offset
// 		mov	(LevelData_srcPtr+0A5h)[di], 1 ; 260 tiles per level, 8	bytes per tile
// 					;
// 					; 2 bytes background
// 					; 1 bytes foreground
// 					; 1 Byte domino	type
// 					; 1 Byte domino	state (animation state
// 					; 1 Byte Y-Offset
// 		mov	(LevelData_srcPtr+0A6h)[di], 0 ; 260 tiles per level, 8	bytes per tile
// 					;
// 					; 2 bytes background
// 					; 1 bytes foreground
// 					; 1 Byte domino	type
// 					; 1 Byte domino	state (animation state
// 					; 1 Byte Y-Offset
// 		mov	ax, 14h
// 		imul	si
// 		mov	di, ax
// 		add	di, [bp+Xpos]
// 		mov	(DirtyBlocks+14h)[di], 1 ; changed blocks on screen, there are 20 blocks per row and 13	blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
// 		mov	DirtyBlocks[di], 1 ; changed blocks on screen, there are 20 blocks per row and 13 blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
// 		mov	(DirtyBlocks+15h)[di], 1 ; changed blocks on screen, there are 20 blocks per row and 13	blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
// 		mov	(DirtyBlocks+1)[di], 1 ; changed blocks	on screen, there are 20	blocks per row and 13 blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
// 		pop	si
// 		pop	di
// 		mov	sp, bp
// 		pop	bp
// 		retn
//
// loc_7BB6:				; CODE XREF: DTA_O+124j
// 		mov	bx, [bp+LeveldataEntry]
// 		cmp	byte ptr [bx+7], 50h ; 'P'
// 		jz	loc_7C05
//
// 		mov	si, [bp+Xpos]
// 		mov	cl, 3
// 		shl	si, cl
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		add	si, ax
// 		mov	al, [bx+3]
// 		mov	[si+4A5Bh], al
// 		mov	al, [bx+4]
// 		mov	[si+4A5Ch], al
// 		mov	al, [bx+5]
// 		mov	[si+4A5Dh], al
// 		mov	al, [bx+6]
// 		add	al, 10h
// 		mov	[si+4A5Eh], al
// 		mov	byte ptr [si+4A5Fh], 50h ; 'P'
// 		mov	byte ptr [bx+3], 0
// 		mov	byte ptr [bx+4], 0
// 		mov	byte ptr [bx+5], 0
// 		mov	byte ptr [bx+6], 0
// 		mov	byte ptr [bx+7], 0
//
// loc_7C05:				; CODE XREF: DTA_O+195j
// 		pop	si
// 		pop	di
// 		mov	sp, bp
// 		pop	bp
// 		retn

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
            getFg(x, y) == FgElementLadder2)
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

// Timbler falln down left
void level_c::DTA_6(int x, int y, int x2, int y2) {


// var_2		= byte ptr -2
// Xpos		= word ptr  4
// Ypos		= word ptr  6
// LeveldataEntry	= word ptr  8
//
// 		push	bp
// 		mov	bp, sp
// 		sub	sp, 2
// 		push	si
// 		cmp	[bp+Xpos], 0
// 		jle	loc_6DD0
//
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		mov	al, (AntAnimBArray+36h)[bx+si] ; probalby LevelDat_srcPtr_6
// 		jmp	short loc_6DD2
//
// loc_6DD0:				; CODE XREF: DTA_6+Bj
// 		sub	al, al
//
// loc_6DD2:				; CODE XREF: DTA_6+20j
// 		mov	[bp+var_2], al
// 		cmp	al, 5
// 		jnz	loc_6DDD
//
// 		mov	[bp+var_2], 0
//
// loc_6DDD:				; CODE XREF: DTA_6+29j
// 		mov	bx, [bp+LeveldataEntry]
// 		cmp	byte ptr [bx+2], 1
// 		jz	loc_6DEF
//
// 		cmp	byte ptr [bx+2], 12h
// 		jz	loc_6DEF
//
// 		jmp	loc_6E74
//
// loc_6DEF:				; CODE XREF: DTA_6+36j	DTA_6+3Cj
// 		cmp	[bp+var_2], 0
// 		jnz	loc_6E74
//
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		cmp	byte ptr [bx+si+4B93h],	0
// 		jz	loc_6E2C
//
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bx+7]
// 		cbw
// 		push	ax		; DominoDelay
// 		mov	al, [bx+3]
// 		cbw
// 		push	ax		; DominoType
// 		mov	ax, [bp+Ypos]
// 		inc	ax
// 		push	ax		; Ypos
// 		mov	ax, [bp+Xpos]
// 		dec	ax
// 		push	ax		; Xpos
// 		call	DominoCrash
//
// 		add	sp, 8
// 		jmp	short loc_6E65
//
// loc_6E2C:				; CODE XREF: DTA_6+5Bj
// 		mov	si, [bp+Xpos]
// 		mov	cl, 3
// 		shl	si, cl
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		add	si, ax
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bx+3]
// 		mov	[si+4AF3h], al
// 		mov	byte ptr [si+4AF4h], 0Eh
// 		mov	al, [bx+5]
// 		mov	[si+4AF5h], al
// 		mov	byte ptr [si+4AF6h], 2
// 		mov	byte ptr [si+4AF7h], 70h ; 'p'
// 		mov	ax, 0Ah
// 		push	ax
// 		call	SomethingWithSound
//
// 		add	sp, 2
//
// loc_6E65:				; CODE XREF: DTA_6+7Aj
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	byte ptr [bx+3], 0
// 		mov	byte ptr [bx+4], 0
// 		mov	byte ptr [bx+5], 0
// 		mov	ax, 14h
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		add	si, [bp+Xpos]
// 		mov	byte ptr [si+78Dh], 1
// 		mov	byte ptr [si+78Eh], 1
// 		mov	byte ptr [si+7A1h], 1
// 		mov	DirtyBlocks[si], 1 ; changed blocks on screen, there are 20 blocks per row and 13 blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
 		return;
//
// loc_6E74:				; CODE XREF: DTA_6+3Ej	DTA_6+45j
// 		cmp	[bp+Xpos], 0
// 		jg	loc_6E7D
//
// 		jmp	loc_6F0A
//
// loc_6E7D:				; CODE XREF: DTA_6+CAj
// 		mov	bx, [bp+LeveldataEntry]
// 		cmp	byte ptr [bx+2], 0Ch
// 		jz	loc_6E89
//
// 		jmp	loc_6F0A
//
// loc_6E89:				; CODE XREF: DTA_6+D6j
// 		cmp	[bp+var_2], 0Bh
// 		jnz	loc_6F0A
//
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		cmp	byte ptr [bx+si+4AF3h],	0
// 		jz	loc_6EC4
//
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bx+7]
// 		cbw
// 		push	ax		; DominoDelay
// 		mov	al, [bx+3]
// 		cbw
// 		push	ax		; DominoType
// 		push	[bp+Ypos]	; Ypos
// 		mov	ax, [bp+Xpos]
// 		dec	ax
// 		push	ax		; Xpos
// 		call	DominoCrash
//
// 		add	sp, 8
// 		jmp	short loc_6EF3
//
// loc_6EC4:				; CODE XREF: DTA_6+F5j
// 		mov	si, [bp+Xpos]
// 		mov	cl, 3
// 		shl	si, cl
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		add	si, ax
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bx+3]
// 		mov	[si+4AF3h], al
// 		mov	byte ptr [si+4AF4h], 0Eh
// 		mov	al, [bx+5]
// 		mov	[si+4AF5h], al
// 		mov	byte ptr [si+4AF6h], 2
// 		mov	byte ptr [si+4AF7h], 40h ; '@'
//
// loc_6EF3:				; CODE XREF: DTA_6+112j
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	byte ptr [bx+3], 0
// 		mov	byte ptr [bx+4], 0
// 		mov	byte ptr [bx+5], 0
// 		mov	byte ptr [bx+6], 0
// 		mov	ax, 14h
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		add	si, [bp+Xpos]
// 		mov	byte ptr [si+78Dh], 1
// 		mov	byte ptr [si+78Eh], 1
// 		mov	byte ptr [si+7A1h], 1
// 		mov	DirtyBlocks[si], 1 ; changed blocks on screen, there are 20 blocks per row and 13 blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
 		return;
//
// loc_6F0A:				; CODE XREF: DTA_6+CCj	DTA_6+D8j ...
// 		cmp	[bp+Xpos], 0
// 		jg	loc_6F13
//
// 		jmp	loc_6FB2
//
// loc_6F13:				; CODE XREF: DTA_6+160j
// 		cmp	[bp+Ypos], 0Bh
// 		jl	loc_6F1C
//
// 		jmp	loc_6FB2
//
// loc_6F1C:				; CODE XREF: DTA_6+169j
// 		mov	bx, [bp+LeveldataEntry]
// 		cmp	byte ptr [bx+2], 0Bh
// 		jz	loc_6F28
//
// 		jmp	loc_6FB2
//
// loc_6F28:				; CODE XREF: DTA_6+175j
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		cmp	byte ptr [bx+si+4B93h],	0
// 		jz	loc_6F5E
//
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bx+7]
// 		cbw
// 		push	ax		; DominoDelay
// 		mov	al, [bx+3]
// 		cbw
// 		push	ax		; DominoType
// 		mov	ax, [bp+Ypos]
// 		inc	ax
// 		push	ax		; Ypos
// 		mov	ax, [bp+Xpos]
// 		dec	ax
// 		push	ax		; Xpos
// 		call	DominoCrash
//
// 		add	sp, 8
// 		jmp	short loc_6F8D
//
// loc_6F5E:				; CODE XREF: DTA_6+18Ej
// 		mov	si, [bp+Xpos]
// 		mov	cl, 3
// 		shl	si, cl
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		add	si, ax
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bx+3]
// 		mov	[si+4B93h], al
// 		mov	byte ptr [si+4B94h], 0Eh
// 		mov	al, [bx+5]
// 		mov	[si+4B95h], al
// 		mov	byte ptr [si+4B96h], 0FAh ; 'ú'
// 		mov	byte ptr [si+4B97h], 40h ; '@'
//
// loc_6F8D:				; CODE XREF: DTA_6+1ADj
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	byte ptr [bx+3], 0
// 		mov	byte ptr [bx+4], 0
// 		mov	byte ptr [bx+5], 0
// 		mov	byte ptr [bx+6], 0
// 		mov	ax, 14h
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	byte ptr [bx+si+7B5h], 1
// 		mov	ax, 14h
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		add	si, [bp+Xpos]
// 		mov	byte ptr [si+78Dh], 1
// 		mov	byte ptr [si+78Eh], 1
// 		mov	byte ptr [si+7A1h], 1
// 		mov	DirtyBlocks[si], 1 ; changed blocks on screen, there are 20 blocks per row and 13 blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
 		return;
//
// loc_6FB2:				; CODE XREF: DTA_6+162j DTA_6+16Bj ...
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		cmp	byte ptr [bx+si+4AF3h],	0
// 		jz	loc_6FE6
//
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bx+7]
// 		cbw
// 		push	ax		; DominoDelay
// 		mov	al, [bx+3]
// 		cbw
// 		push	ax		; DominoType
// 		push	[bp+Ypos]	; Ypos
// 		mov	ax, [bp+Xpos]
// 		dec	ax
// 		push	ax		; Xpos
// 		call	DominoCrash
//
// 		add	sp, 8
// 		jmp	short loc_7009
//
// loc_6FE6:				; CODE XREF: DTA_6+218j
// 		mov	si, [bp+Xpos]
// 		mov	cl, 3
// 		shl	si, cl
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		add	si, ax
// 		mov	byte ptr [si+4AF3h], 6
// 		mov	byte ptr [si+4AF4h], 0Eh
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bx+5]
// 		mov	[si+4AF5h], al
//
// loc_7009:				; CODE XREF: DTA_6+235j
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	byte ptr [bx+3], 0
// 		mov	byte ptr [bx+5], 0
// 		mov	ax, 14h
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		add	si, [bp+Xpos]
// 		mov	byte ptr [si+78Dh], 1
// 		mov	byte ptr [si+78Eh], 1
// 		mov	byte ptr [si+7A1h], 1
// 		mov	DirtyBlocks[si], 1 ; changed blocks on screen, there are 20 blocks per row and 13 blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
 		return;

}

// Tumbler falln down right
void level_c::DTA_L(int x, int y, int x2, int y2) {

// var_2		= byte ptr -2
// Xpos		= word ptr  4
// Ypos		= word ptr  6
// LeveldataEntry	= word ptr  8
//
// 		push	bp
// 		mov	bp, sp
// 		sub	sp, 2
// 		push	si
// 		cmp	[bp+Xpos], 13h
// 		jge	loc_705A
//
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		mov	al, (LevelData_srcPtr+0Ah)[bx+si] ; 260	tiles per level, 8 bytes per tile
// 					;
// 					; 2 bytes background
// 					; 1 bytes foreground
// 					; 1 Byte domino	type
// 					; 1 Byte domino	state (animation state
// 					; 1 Byte Y-Offset
// 		jmp	short loc_705C
//
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//
// loc_705A:				; CODE XREF: DTA_L+Bj
// 		sub	al, al
//
// loc_705C:				; CODE XREF: DTA_L+20j
// 		mov	[bp+var_2], al
// 		cmp	al, 5
// 		jnz	loc_7067
//
// 		mov	[bp+var_2], 0
//
// loc_7067:				; CODE XREF: DTA_L+29j
// 		mov	bx, [bp+LeveldataEntry]
// 		cmp	byte ptr [bx+2], 3
// 		jz	loc_7079
//
// 		cmp	byte ptr [bx+2], 12h
// 		jz	loc_7079
//
// 		jmp	loc_70FE
//
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//
// loc_7079:				; CODE XREF: DTA_L+36j	DTA_L+3Cj
// 		cmp	[bp+var_2], 0
// 		jnz	loc_70FE
//
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		cmp	byte ptr [bx+si+4BA3h],	0
// 		jz	loc_70B6
//
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bx+7]
// 		cbw
// 		push	ax		; DominoDelay
// 		mov	al, [bx+3]
// 		cbw
// 		push	ax		; DominoType
// 		mov	ax, [bp+Ypos]
// 		inc	ax
// 		push	ax		; Ypos
// 		mov	ax, [bp+Xpos]
// 		inc	ax
// 		push	ax		; Xpos
// 		call	DominoCrash
//
// 		add	sp, 8
// 		jmp	short loc_70EF
//
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// 		nop
// 		nop
//
// loc_70B6:				; CODE XREF: DTA_L+5Bj
// 		mov	si, [bp+Xpos]
// 		mov	cl, 3
// 		shl	si, cl
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		add	si, ax
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bx+3]
// 		mov	[si+4B03h], al
// 		mov	byte ptr [si+4B04h], 2
// 		mov	al, [bx+5]
// 		mov	[si+4B05h], al
// 		mov	byte ptr [si+4B06h], 2
// 		mov	byte ptr [si+4B07h], 70h ; 'p'
// 		mov	ax, 0Ah
// 		push	ax
// 		call	SomethingWithSound
//
// 		add	sp, 2
//
// loc_70EF:				; CODE XREF: DTA_L+7Aj
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	byte ptr [bx+3], 0
// 		mov	byte ptr [bx+4], 0
// 		jmp	loc_72AA
//
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// 		align 2
//
// loc_70FE:				; CODE XREF: DTA_L+3Ej	DTA_L+45j
// 		cmp	[bp+Xpos], 13h
// 		jl	loc_7107
//
// 		jmp	loc_7194
//
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//
// loc_7107:				; CODE XREF: DTA_L+CAj
// 		mov	bx, [bp+LeveldataEntry]
// 		cmp	byte ptr [bx+2], 7
// 		jz	loc_7113
//
// 		jmp	loc_7194
//
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//
// loc_7113:				; CODE XREF: DTA_L+D6j
// 		cmp	[bp+var_2], 8
// 		jnz	loc_7194
//
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		cmp	byte ptr [bx+si+4B03h],	0
// 		jz	loc_714E
//
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bx+7]
// 		cbw
// 		push	ax		; DominoDelay
// 		mov	al, [bx+3]
// 		cbw
// 		push	ax		; DominoType
// 		push	[bp+Ypos]	; Ypos
// 		mov	ax, [bp+Xpos]
// 		inc	ax
// 		push	ax		; Xpos
// 		call	DominoCrash
//
// 		add	sp, 8
// 		jmp	short loc_717D
//
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// 		nop
// 		nop
//
// loc_714E:				; CODE XREF: DTA_L+F5j
// 		mov	si, [bp+Xpos]
// 		mov	cl, 3
// 		shl	si, cl
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		add	si, ax
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bx+3]
// 		mov	[si+4B03h], al
// 		mov	byte ptr [si+4B04h], 2
// 		mov	al, [bx+5]
// 		mov	[si+4B05h], al
// 		mov	byte ptr [si+4B06h], 2
// 		mov	byte ptr [si+4B07h], 40h ; '@'
//
// loc_717D:				; CODE XREF: DTA_L+112j
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	byte ptr [bx+3], 0
// 		mov	byte ptr [bx+4], 0
// 		mov	byte ptr [bx+5], 0
// 		mov	byte ptr [bx+6], 0
// 		jmp	loc_72AE
//
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// 		align 2
//
// loc_7194:				; CODE XREF: DTA_L+CCj	DTA_L+D8j ...
// 		cmp	[bp+Xpos], 13h
// 		jl	loc_719D
//
// 		jmp	loc_724C
//
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//
// loc_719D:				; CODE XREF: DTA_L+160j
// 		cmp	[bp+Ypos], 0Bh
// 		jl	loc_71A6
//
// 		jmp	loc_724C
//
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//
// loc_71A6:				; CODE XREF: DTA_L+169j
// 		mov	bx, [bp+LeveldataEntry]
// 		cmp	byte ptr [bx+2], 8
// 		jz	loc_71B2
//
// 		jmp	loc_724C
//
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//
// loc_71B2:				; CODE XREF: DTA_L+175j
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		cmp	byte ptr [bx+si+4BA3h],	0
// 		jz	loc_71F8
//
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bx+7]
// 		cbw
// 		push	ax		; DominoDelay
// 		mov	al, [bx+3]
// 		cbw
// 		push	ax		; DominoType
// 		mov	ax, [bp+Ypos]
// 		inc	ax
// 		push	ax		; Ypos
// 		mov	ax, [bp+Xpos]
// 		inc	ax
// 		push	ax		; Xpos
// 		call	DominoCrash
//
// 		add	sp, 8
// 		cmp	CopyProtectionFailure, 0 ; when	1 copy protection failed
// 		jz	loc_7227
//
// 		push	TxtProtectionViolation2	; MsgStringOfs
// 		call	CleanupAndExitWithMesg
//
// 		add	sp, 2
// 		jmp	short loc_7227
//
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//
// loc_71F8:				; CODE XREF: DTA_L+18Ej
// 		mov	si, [bp+Xpos]
// 		mov	cl, 3
// 		shl	si, cl
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		add	si, ax
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bx+3]
// 		mov	[si+4BA3h], al
// 		mov	byte ptr [si+4BA4h], 2
// 		mov	al, [bx+5]
// 		mov	[si+4BA5h], al
// 		mov	byte ptr [si+4BA6h], 0FAh ; 'ú'
// 		mov	byte ptr [si+4BA7h], 40h ; '@'
//
// loc_7227:				; CODE XREF: DTA_L+1B2j DTA_L+1BEj
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	byte ptr [bx+3], 0
// 		mov	byte ptr [bx+4], 0
// 		mov	byte ptr [bx+5], 0
// 		mov	byte ptr [bx+6], 0
// 		mov	ax, 14h
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	byte ptr [bx+si+7B7h], 1
// 		jmp	short loc_72AE
//
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//
// loc_724C:				; CODE XREF: DTA_L+162j DTA_L+16Bj ...
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		cmp	byte ptr [bx+si+4B03h],	0
// 		jz	loc_7280
//
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bx+7]
// 		cbw
// 		push	ax		; DominoDelay
// 		mov	al, [bx+3]
// 		cbw
// 		push	ax		; DominoType
// 		push	[bp+Ypos]	; Ypos
// 		mov	ax, [bp+Xpos]
// 		inc	ax
// 		push	ax		; Xpos
// 		call	DominoCrash
//
// 		add	sp, 8
// 		jmp	short loc_72A3
//
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// 		align 2
//
// loc_7280:				; CODE XREF: DTA_L+228j
// 		mov	si, [bp+Xpos]
// 		mov	cl, 3
// 		shl	si, cl
// 		mov	ax, 0A0h ; ' '
// 		imul	[bp+Ypos]
// 		add	si, ax
// 		mov	byte ptr [si+4B03h], 6
// 		mov	byte ptr [si+4B04h], 2
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bx+5]
// 		mov	[si+4B05h], al
//
// loc_72A3:				; CODE XREF: DTA_L+245j
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	byte ptr [bx+3], 0
//
// loc_72AA:				; CODE XREF: DTA_L+C2j
// 		mov	byte ptr [bx+5], 0
//
// loc_72AE:				; CODE XREF: DTA_L+158j DTA_L+212j
// 		mov	ax, 14h
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		add	si, [bp+Xpos]
// 		mov	byte ptr [si+78Eh], 1
// 		mov	byte ptr [si+78Fh], 1
// 		mov	DirtyBlocks[si], 1 ; changed blocks on screen, there are 20 blocks per row and 13 blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
// 		mov	(DirtyBlocks+1)[si], 1 ; changed blocks	on screen, there are 20	blocks per row and 13 blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
// 		pop	si
// 		mov	sp, bp
// 		pop	bp
// 		retn

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
