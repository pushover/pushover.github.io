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
      if ((dynamicDirty[y] >> x) & 1) {

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

        /* paint the foreground */
        if (getDominoType(x, y) > 0) {
          SDL_Surface * v = gr->getDomino(getDominoType(x, y)-1, getDominoState(x, y)-1);

          if (v) {

            SDL_Rect dst;
            dst.x = (x-2)*gr->blockX();
            dst.y = y*gr->blockY()-v->h + 8;
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
      if ((dynamicDirty[y] >> x) & 1) {
        if (getFg(x, y) == FgElementPlatformLadderDown || getFg(x, y) == FgElementLadder) {
          SDL_Rect dst;
          dst.x = x*gr->blockX();
          dst.y = y*gr->blockY();
          dst.w = gr->blockX();
          dst.h = gr->blockY();
          SDL_BlitSurface(gr->getFgTile(FgElementLadder2), 0, target, &dst);
	} else if (getFg(x, y) == FgElementPlatformLadderUp) {
          SDL_Rect dst;
          dst.x = x*gr->blockX();
          dst.y = y*gr->blockY();
          dst.w = gr->blockX();
          dst.h = gr->blockY();
          SDL_BlitSurface(gr->getFgTile(FgElementLadderMiddle), 0, target, &dst);
	}
      }
      dynamicDirty[y] &= ~(1 << x);
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

