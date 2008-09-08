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

void level_c::putDownDomino(int x, int y, int domino) {

  if (level[y][x].dominoType != 0) {

    // TODO:::: crash

  } else if (x > 0 && level[y][x-1].dominoType && level[y][x-1].dominoState >= 12) {

    // TODO:::: crash

  } else if (x < 19 && level[y][x+1].dominoType && level[y][x-1].dominoState <= 4) {

    // TODO:::: crash

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
        // crash
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

  if (level[y][x].dominoType != DominoTypeSplitter)
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

void level_c::DTA_C(int x, int y, int x2, int y2) {

// var_6		= byte ptr -6
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
// 		mov	al, [bx+4]
// 		cbw
// 		mov	si, ax
// 		shl	si, 1
// 		mov	al, byte ptr DominoTypeAnimAction[si] ;	a callback function for	each domino type for each possible animation state
// 					; the real array is below this are just	the missing entries for	the empty domino type
// 		mov	[bp+var_2], al
// 		mov	al, byte ptr (DominoTypeAnimAction+1)[si] ; a callback function	for each domino	type for each possible animation state
// 					; the real array is below this are just	the missing entries for	the empty domino type
// 		mov	[bp+var_6], al
// 		cmp	[bp+var_2], 3
// 		jnz	loc_6BA4
//
// 		cmp	[bp+Xpos], 0
// 		jle	loc_6B75
//
// 		mov	ax, 160
// 		imul	[bp+Ypos]
// 		mov	di, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		cmp	(AntAnimBArray+36h)[bx+di], 0Ah	; probalby LevelDat_srcPtr_6
// 		jz	loc_6BC3
//
//
// loc_6B75:				; CODE XREF: DTA_C+2Bj
// 		mov	ax, 160
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		cmp	(AntAnimBArray+37h)[bx+si], DominoTypeEmpty ; probalby LevelDat_srcPtr_5
// 		jz	loc_6BC0
//
// 		mov	ax, 0FFFFh
// 		push	ax		; Direction
// 		push	[bp+Ypos]	; Ypos
// 		mov	ax, [bp+Xpos]
// 		dec	ax
// 		push	ax		; Xpos
// 		call	PushDomino	; starts the domino at position	falling	in the given direction
//
// 		add	sp, 6
// 		or	ax, ax
// 		jz	loc_6BC3
//
// 		jmp	short loc_6BC0
//
// loc_6BA4:				; CODE XREF: DTA_C+25j
// 		cmp	[bp+var_2], 2
// 		jnz	loc_6BC3
//
// 		mov	ax, 160
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		cmp	(AntAnimBArray+37h)[bx+si], 0 ;	probalby LevelDat_srcPtr_5
// 		jnz	loc_6BC3
//
//
// loc_6BC0:				; CODE XREF: DTA_C+57j	DTA_C+6Fj
// 		dec	[bp+var_2]
//
// loc_6BC3:				; CODE XREF: DTA_C+41j	DTA_C+6Dj ...
// 		cmp	[bp+var_6], 3
// 		jnz	loc_6C0C
//
// 		cmp	[bp+Xpos], 13h
// 		jge	loc_6BDE
//
// 		mov	ax, 160
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
//
// loc_6BDE:				; CODE XREF: DTA_C+9Bj
// 		mov	ax, 160
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		cmp	(LevelData_srcPtr+0Bh)[bx+si], 0 ; 260 tiles per level,	8 bytes	per tile
// 					;
// 					; 2 bytes background
// 					; 1 bytes foreground
// 					; 1 Byte domino	type
// 					; 1 Byte domino	state (animation state
// 					; 1 Byte Y-Offset
// 		jz	loc_6C28
//
// 		mov	ax, 1
// 		push	ax		; Direction
// 		push	[bp+Ypos]	; Ypos
// 		mov	ax, [bp+Xpos]
// 		inc	ax
// 		push	ax		; Xpos
// 		call	PushDomino	; starts the domino at position	falling	in the given direction
//
// 		add	sp, 6
// 		or	ax, ax
// 		jz	loc_6C2B
//
// 		jmp	short loc_6C28
//
// loc_6C0C:				; CODE XREF: DTA_C+95j
// 		cmp	[bp+var_6], 2
// 		jnz	loc_6C2B
//
// 		mov	ax, 160
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		mov	bx, [bp+Xpos]
// 		mov	cl, 3
// 		shl	bx, cl
// 		cmp	(LevelData_srcPtr+0Bh)[bx+si], 0 ; 260 tiles per level,	8 bytes	per tile
// 					;
// 					; 2 bytes background
// 					; 1 bytes foreground
// 					; 1 Byte domino	type
// 					; 1 Byte domino	state (animation state
// 					; 1 Byte Y-Offset
// 		jnz	loc_6C2B
//
//
// loc_6C28:				; CODE XREF: DTA_C+C0j	DTA_C+D8j
// 		dec	[bp+var_6]
//
// loc_6C2B:				; CODE XREF: DTA_C+D6j	DTA_C+DEj ...
// 		mov	[bp+var_4], 0
// 		jmp	short loc_6C35
//
// loc_6C32:				; CODE XREF: DTA_C+118j DTA_C+121j
// 		inc	[bp+var_4]
//
// loc_6C35:				; CODE XREF: DTA_C+FDj
// 		cmp	[bp+var_4], 0Eh
// 		jge	loc_6C55
//
// 		mov	al, [bp+var_4]
// 		cbw
// 		mov	si, ax
// 		shl	si, 1
// 		mov	al, [bp+var_2]
// 		cmp	byte ptr (DominoTypeAnimAction+2)[si], al ; a callback function	for each domino	type for each possible animation state
// 					; the real array is below this are just	the missing entries for	the empty domino type
// 		jnz	loc_6C32
//
// 		mov	al, [bp+var_6]
// 		cmp	byte ptr (DominoTypeAnimAction+3)[si], al ; a callback function	for each domino	type for each possible animation state
// 					; the real array is below this are just	the missing entries for	the empty domino type
// 		jnz	loc_6C32
//
//
// loc_6C55:				; CODE XREF: DTA_C+107j
// 		inc	[bp+var_4]
// 		mov	bx, [bp+LeveldataEntry]
// 		mov	al, [bp+var_4]
// 		cmp	[bx+4],	al
// 		jz	loc_6C8F
//
// 		mov	[bx+4],	al
// 		mov	ax, 14h
// 		imul	[bp+Ypos]
// 		mov	si, ax
// 		add	si, [bp+Xpos]
// 		mov	(DirtyBlocksPrefix+13h)[si], 1 ; 2 additional lines of direty blocks
// 					; to access blocks above the screen without additional checks
// 		mov	(DirtyBlocksPrefix+14h)[si], 1 ; 2 additional lines of direty blocks
// 					; to access blocks above the screen without additional checks
// 		mov	(DirtyBlocksPrefix+15h)[si], 1 ; 2 additional lines of direty blocks
// 					; to access blocks above the screen without additional checks
// 		mov	(DirtyBlocksPrefix+27h)[si], 1 ; 2 additional lines of direty blocks
// 					; to access blocks above the screen without additional checks
// 		mov	DirtyBlocks[si], 1 ; changed blocks on screen, there are 20 blocks per row and 13 blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
// 		mov	(DirtyBlocks+1)[si], 1 ; changed blocks	on screen, there are 20	blocks per row and 13 blocks
// 					; the last block is only halve.	There are 2 additional rows at the bottom
//
// loc_6C8F:				; CODE XREF: DTA_C+12Fj

    return;

}

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
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
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
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
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
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
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
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//
// loc_74A6:				; CODE XREF: DTA_M+6Ej
// 		mov	al, 1
//
// loc_74A8:				; CODE XREF: DTA_M+72j
// 		mov	[bp+CurrentLevelForeground], al
// 		mov	[bp+var_4], 2
// 		jmp	short loc_7517
//
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// 		align 2
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
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
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
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// 		align 2
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
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//
// loc_74E8:				; CODE XREF: DTA_M+B0j
// 		mov	al, 1
//
// loc_74EA:				; CODE XREF: DTA_M+B4j
// 		mov	[bp+CurrentLevelForeground], al
// 		mov	[bp+var_4], 3
// 		jmp	short loc_7517
//
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// 		align 2
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
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
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
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
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
// ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
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
void level_c::DTA_A(int x, int y, int x2, int y2) {
}
void level_c::DTA_O(int x, int y, int x2, int y2) {
}
void level_c::DTA_H(int x, int y, int x2, int y2) {
}

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

void level_c::DTA_6(int x, int y, int x2, int y2) {
}
void level_c::DTA_L(int x, int y, int x2, int y2) {
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
