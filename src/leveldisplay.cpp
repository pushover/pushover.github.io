#include "leveldisplay.h"

#include "textsections.h"
#include "sha1.h"
#include "graphics.h"
#include "soundsys.h"
#include "text.h"
#include "screen.h"
#include "ant.h"

#include <stdio.h>

#include <sstream>
#include <iomanip>

void levelDisplay_c::load(const textsections_c & sections) {

  levelData_c::load(sections);

  // attention don't use many levels on the same graphics object
  gr.setTheme(getTheme());
  background.markAllDirty();
  target.markAllDirty();
}

levelDisplay_c::levelDisplay_c(surface_c & t, graphics_c & g) : background(t), target(t), gr(g) {
}

levelDisplay_c::~levelDisplay_c(void) { }

void levelDisplay_c::updateBackground(void)
{
  for (unsigned int y = 0; y < 13; y++)
  {
    for (unsigned int x = 0; x < 20; x++)
    {
      // when the current block is dirty, recreate it
      if (background.isDirty(x, y))
      {
        SDL_Rect dst;
        dst.x = x*gr.blockX();
        dst.y = y*gr.blockY();
        dst.w = gr.blockX();
        dst.h = gr.blockY();
        for (unsigned char b = 0; b < numBg; b++)
          SDL_BlitSurface(gr.getBgTile(level[y][x].bg[b]), 0, background.getVideo(), &dst);
        SDL_BlitSurface(gr.getFgTile(level[y][x].fg), 0, background.getVideo(), &dst);

        // apply gradient effect
        for (unsigned int i = 0; i < gr.blockY() && y*gr.blockY()+i < (unsigned int)background.getVideo()->h; i++)
          for (unsigned int j = 0; j < gr.blockX(); j++) {

            uint32_t col = *((uint32_t*)(((uint8_t*)background.getVideo()->pixels) + (y*gr.blockY()+i) * background.getVideo()->pitch +
                  background.getVideo()->format->BytesPerPixel*(x*gr.blockX()+j)));

            Uint8 r, g, b;

            SDL_GetRGB(col, background.getVideo()->format, &r, &g, &b);

            double val = (2.0-((1.0*x*gr.blockX()+j)/background.getVideo()->w + (1.0*y*gr.blockY()+i)/background.getVideo()->h));
            val += (1.0*rand()/RAND_MAX)/20 - 1.0/40;
            if (val < 0) val = 0;
            if (val > 2) val = 2;

            r = (Uint8)(((255.0-r)*val+r)*r/255);
            g = (Uint8)(((255.0-g)*val+g)*g/255);
            b = (Uint8)(((255.0-b)*val+b)*b/255);

            col = SDL_MapRGB(background.getVideo()->format, r, g, b);

            *((uint32_t*)(((uint8_t*)background.getVideo()->pixels) + (y*gr.blockY()+i) * background.getVideo()->pitch +
                  background.getVideo()->format->BytesPerPixel*(x*gr.blockX()+j))) = col;
          }
      }
    }

  }
  background.clearDirty();
}


static void PutSprite(int nr, int x, int y, SDL_Surface * v, SDL_Surface * target)
{
  if (v)
  {
    SDL_Rect dst;
    dst.x = x;
    dst.y = y - v->h;
    dst.w = v->w;
    dst.h = v->h;

    SDL_BlitSurface(v, 0, target, &dst);

#if 0 // this is for debugging domino display functionality, it draws a little colored rectangle
      // below the dominos for me to see which if the many ifs in the function below is painting it
    dst.x = x+2*40+20-4;
    dst.y = y;
    dst.w = 8;
    dst.h = 8;

    Uint32 cols[] = {
      0xFF0000, // 0
      0x00FF00, // 1
      0x0000FF, // 2
      0x00FFFF, // 3
      0xFF00FF, // 4
      0xFFFF00, // 5
      0xFFFFFF, // 6
      0x800000, // 7
      0x008000, // 8
      0x000080, // 9
      0x008080, // 10
      0x800080, // 11
      0x808000, // 12
      0x808080, // 13
      0xFF8000, // 14
      0x00FF80, // 15
      0xFF0080, // 16
    };


    SDL_FillRect(target, &dst, cols[nr]);
#endif
  }
}

void levelDisplay_c::drawDominos(bool debug) {

  // the dirty marks for the clock
  {
    // calculate the second left
    int tm = timeLeft/18;

    // if negative make positive again
    if (timeLeft < 0)
      tm = -tm+1;

    int newSec = tm%60;
    int newMin = tm/60;

    if (newSec != Sec || timeLeft == -1)
    {
      target.markDirty(3, 11);
      target.markDirty(3, 12);
    }

    if (newSec != Sec || newMin != Min || timeLeft % 18 == 17 || timeLeft % 18 == 8 || timeLeft == -1)
    {
      target.markDirty(2, 11);
      target.markDirty(2, 12);
    }

    if (newMin != Min || timeLeft == -1)
    {
      target.markDirty(1, 11);
      target.markDirty(1, 12);
    }

    Min = newMin;
    Sec = newSec;
  }

  for (unsigned int y = 0; y < 13; y++)
    for (unsigned int x = 0; x < 20; x++) {
      if (target.isDirty(x, y) || debug) {

        // copy background from background surface
        {
          SDL_Rect src, dst;
          dst.x = x*gr.blockX();
          dst.y = y*gr.blockY();
          dst.w = gr.blockX();
          dst.h = gr.blockY();
          src.x = x*gr.blockX();
          src.y = y*gr.blockY();
          src.w = gr.blockX();
          src.h = gr.blockY();
          SDL_BlitSurface(background.getVideo(), &src, target.getVideo(), &dst);
        }
      }
    }

  static int XposOffset[] = {-16, -16,  0,-16,  0,  0, 0, 0, 0,  0, 0, 16,  0, 16, 16, 0};
  static int YposOffset[] = { -8,  -6,  0, -4,  0, -2, 0, 0, 0, -2, 0, -4,  0, -6, -8, 0};
  static int StoneImageOffset[] = {  7, 6, 0, 5, 0, 4, 0, 0, 0, 3, 0, 2, 0, 1, 0, 0};

  // the idea behind this code is to repaint the dirty blocks. Dominos that are actually
  // within neighbor block must be repaint, too, when they might reach into the actual
  // block. But painting the neighbors is only necessary, when they are not drawn on
  // their own anyway, so always check for !dirty of the "homeblock" of each domino

  int SpriteYPos = gr.getDominoYStart();

  for (int y = 0; y < 13; y++, SpriteYPos += gr.blockY()) {

    int SpriteXPos = -2*gr.blockX();

    for (int x = 0; x < 20; x++, SpriteXPos += gr.blockX()) {

      if (!target.isDirty(x, y) && !debug) continue;

      // paint the left neighbor domino, if it leans in our direction and is not painted on its own
      if (y < 12 && x > 0 && !target.isDirty(x-1, y+1) && level[y+1][x-1].dominoType != DominoTypeEmpty &&
          (level[y+1][x-1].dominoState > 8 ||
           level[y+1][x-1].dominoType == DominoTypeSplitter && level[y+1][x-1].dominoState != 8 ||
           level[y+1][x-1].dominoState >= DominoTypeCrash0))
      {
        PutSprite(0,
            SpriteXPos-gr.blockX(),
            SpriteYPos+gr.convertDominoY(level[y+1][x-1].dominoYOffset)+gr.blockY(),
            gr.getDomino(level[y+1][x-1].dominoType-1, level[y+1][x-1].dominoState-1), target.getVideo()
            );
      }

      if (x > 0 && !target.isDirty(x-1, y) && level[y][x-1].dominoType != DominoTypeEmpty &&
          (level[y][x-1].dominoState > 8 ||
           level[y][x-1].dominoType == DominoTypeSplitter && level[y][x-1].dominoState != 8 ||
           level[y][x-1].dominoType >= DominoTypeCrash0))
      {
        PutSprite(1,
            SpriteXPos-gr.blockX(),
            SpriteYPos+gr.convertDominoY(level[y][x-1].dominoYOffset),
            gr.getDomino(level[y][x-1].dominoType-1, level[y][x-1].dominoState-1), target.getVideo()
            );
      }

      if (y < 12 && !target.isDirty(x, y+1) && level[y+1][x].dominoType != DominoTypeEmpty)
      {
        PutSprite(2,
            SpriteXPos,
            SpriteYPos+gr.convertDominoY(level[y+1][x].dominoYOffset)+gr.blockY(),
            gr.getDomino(level[y+1][x].dominoType-1, level[y+1][x].dominoState-1), target.getVideo()
            );
      }

      // paint the splitting domino for the splitter
      if (level[y][x].dominoType == DominoTypeSplitter &&
          level[y][x].dominoState == 6 &&
          level[y][x].dominoExtra != 0)
      {
        PutSprite(3,
            SpriteXPos,
            SpriteYPos-gr.splitterY(),
            gr.getDomino(level[y][x].dominoExtra-1, level[y][x].dominoExtra>=DominoTypeCrash0?0:7), target.getVideo()
            );
        level[y][x].dominoExtra = 0;
      }

      // paint the actual domino but take care of the special cases of the ascender domino
      if (level[y][x].dominoType == DominoTypeAscender && level[y][x].dominoExtra == 0x60 &&
          level[y][x].dominoState < 16 && level[y][x].dominoState != 8)
      {
        PutSprite(4,
            SpriteXPos+gr.convertDominoX(XposOffset[level[y][x].dominoState-1]),
            SpriteYPos+gr.convertDominoY(YposOffset[level[y][x].dominoState-1]+level[y][x].dominoYOffset),
            gr.getDomino(DominoTypeRiserCont-1, StoneImageOffset[level[y][x].dominoState-1]), target.getVideo()
            );
      }
      else if (level[y][x].dominoType == DominoTypeAscender && level[y][x].dominoState == 1 && level[y][x].dominoExtra == 0 &&
          level[y-2][x-1].fg == 0)
      { // this is the case of the ascender domino completely horizontal and with the plank it is below not existing
        // so we see the above face of the domino. Normally there is a wall above us so we only see
        // the front face of the domino
        PutSprite(5,
            SpriteXPos+gr.convertDominoX(XposOffset[level[y][x].dominoState-1]+6),
            SpriteYPos+gr.convertDominoY(YposOffset[level[y][x].dominoState-1]+level[y][x].dominoYOffset),
            gr.getDomino(DominoTypeRiserCont-1, StoneImageOffset[level[y][x].dominoState-1]), target.getVideo()
            );
      }
      else if (level[y][x].dominoType == DominoTypeAscender && level[y][x].dominoState == 15 && level[y][x].dominoExtra == 0 &&
          level[y-2][x+1].fg == 0)
      {
        PutSprite(6,
            SpriteXPos+gr.convertDominoX(XposOffset[level[y][x].dominoState-1]-2),
            SpriteYPos+gr.convertDominoY(YposOffset[level[y][x].dominoState-1]+level[y][x].dominoYOffset),
            gr.getDomino(DominoTypeRiserCont-1, StoneImageOffset[level[y][x].dominoState-1]), target.getVideo()
            );
      }
      else if (level[y][x].dominoType != DominoTypeEmpty)
      {
        PutSprite(7,
            SpriteXPos,
            SpriteYPos+gr.convertDominoY(level[y][x].dominoYOffset),
            gr.getDomino(level[y][x].dominoType-1, level[y][x].dominoState-1), target.getVideo()
            );
      }

      // paint the right neighor if it is leaning in our direction
      if (x < 19 && y < 12 && !target.isDirty(x+1, y+1) && level[y+1][x+1].dominoType != DominoTypeEmpty &&
          (level[y+1][x+1].dominoState < 8 ||
           level[y+1][x+1].dominoType == DominoTypeSplitter && level[y+1][x+1].dominoState != 8 ||
           level[y+1][x+1].dominoType >= DominoTypeCrash0))
      {
        PutSprite(8,
            SpriteXPos+gr.blockX(),
            SpriteYPos+gr.convertDominoY(level[y+1][x+1].dominoYOffset)+gr.blockY(),
            gr.getDomino(level[y+1][x+1].dominoType-1, level[y+1][x+1].dominoState-1), target.getVideo()
            );
      }

      if (x < 19 && !target.isDirty(x+1, y) && level[y][x+1].dominoType != DominoTypeEmpty &&
          (level[y][x+1].dominoState < 8 ||
           level[y][x+1].dominoType == DominoTypeSplitter && level[y][x+1].dominoState != 8 ||
           level[y][x+1].dominoType >= DominoTypeCrash0))
      {
        PutSprite(9,
            SpriteXPos+gr.blockX(),
            SpriteYPos+gr.convertDominoY(level[y][x+1].dominoYOffset),
            gr.getDomino(level[y][x+1].dominoType-1, level[y][x+1].dominoState-1), target.getVideo()
            );
      }

      if (y >= 11) continue;

      if (!target.isDirty(x, y+2) && level[y+2][x].dominoType == DominoTypeAscender)
      {
        PutSprite(10,
            SpriteXPos,
            SpriteYPos+gr.convertDominoY(level[y+2][x].dominoYOffset)+2*gr.blockY(),
            gr.getDomino(level[y+2][x].dominoType-1, level[y+2][x].dominoState-1), target.getVideo()
            );
      }

      if (x > 0 && !target.isDirty(x-1, y+2) && level[y+2][x-1].dominoType == DominoTypeAscender)
      {
        PutSprite(11,
            SpriteXPos-gr.blockX(),
            SpriteYPos+gr.convertDominoY(level[y+2][x-1].dominoYOffset)+2*gr.blockY(),
            gr.getDomino(level[y+2][x-1].dominoType-1, level[y+2][x-1].dominoState-1), target.getVideo()
            );
      }

      if (x < 19 && !target.isDirty(x+1, y+2) && level[y+2][x+1].dominoType == DominoTypeAscender)
      {
        PutSprite(12,
            SpriteXPos+gr.blockX(),
            SpriteYPos+gr.convertDominoY(level[y+2][x+1].dominoYOffset)+2*gr.blockY(),
            gr.getDomino(level[y+2][x+1].dominoType-1, level[y+2][x+1].dominoState-1), target.getVideo()
            );
      }

      if (level[y][x].dominoType != DominoTypeAscender) continue;

      if (!target.isDirty(x, y+2) && level[y+2][x].dominoType != DominoTypeEmpty)
      {
        PutSprite(13,
            SpriteXPos,
            SpriteYPos+gr.convertDominoY(level[y+2][x].dominoYOffset)+2*gr.blockY(),
            gr.getDomino(level[y+2][x].dominoType-1, level[y+2][x].dominoState-1), target.getVideo()
            );
      }

      if (x > 0 && !target.isDirty(x-1, y+2) && level[y+2][x-1].dominoType != DominoTypeEmpty)
      {
        PutSprite(14,
            SpriteXPos-gr.blockX(),
            SpriteYPos+gr.convertDominoY(level[y+2][x-1].dominoYOffset)+2*gr.blockY(),
            gr.getDomino(level[y+2][x-1].dominoType-1, level[y+2][x-1].dominoState-1), target.getVideo()
            );
      }

      if (x >= 19) continue;

      if (!target.isDirty(x+1, y+2)) continue;

      if (level[y+2][x+1].dominoType == DominoTypeEmpty) continue;

      PutSprite(15,
          SpriteXPos+gr.blockX(),
          SpriteYPos+gr.convertDominoY(level[y+2][x+1].dominoYOffset)+2*gr.blockY(),
          gr.getDomino(level[y+2][x+1].dominoType-1, level[y+2][x+1].dominoState-1), target.getVideo()
          );
    }
  }

  // repaint the ladders in front of dominos
  for (unsigned int y = 0; y < 13; y++)
    for (unsigned int x = 0; x < 20; x++) {
      if (target.isDirty(x, y) || debug) {
        if (getFg(x, y) == FgElementPlatformLadderDown || getFg(x, y) == FgElementLadder) {
          SDL_Rect dst;
          dst.x = x*gr.blockX();
          dst.y = y*gr.blockY();
          dst.w = gr.blockX();
          dst.h = gr.blockY();
          SDL_BlitSurface(gr.getFgTile(FgElementLadder2), 0, target.getVideo(), &dst);
        }
        else if (getFg(x, y) == FgElementPlatformLadderUp)
        {
          SDL_Rect dst;
          dst.x = x*gr.blockX();
          dst.y = y*gr.blockY();
          dst.w = gr.blockX();
          dst.h = gr.blockY();
          SDL_BlitSurface(gr.getFgTile(FgElementLadderMiddle), 0, target.getVideo(), &dst);
        }
      }
    }

  if (debug)
  {
    for (unsigned int y = 0; y < 13; y++)
      for (unsigned int x = 0; x < 20; x++) {

        if (target.isDirty(x, y))
        {
          for (unsigned int h = 0; h < gr.blockY(); h+=2)
          {
            SDL_Rect dst;
            dst.w = gr.blockX();
            dst.h = 1;
            dst.x = x*gr.blockX();
            dst.y = y*gr.blockY()+h;
            Uint32 color = SDL_MapRGB(target.getVideo()->format, 0, 0, 0);
            SDL_FillRect(target.getVideo(), &dst, color);
          }
        }
      }
  }

  if (timeLeft < 60*60*18)
  { // output the time
    char time[6];

    // care for the : between the minutes and seconds and
    // make a string out of the time
    // in the new font ':' and ' ' have different width, so keep it
    // just a colon for now, we will make it blink later on again
    // TODO
//    if (timeLeft % 18 < 9)
      snprintf(time, 6, "%02i:%02i", Min, Sec);
//    else
//      snprintf(time, 6, "%02i %02i", Min, Sec);

    fontParams_s pars;
    if (timeLeft >= 0)
    {
      pars.color.r = pars.color.g = 255; pars.color.b = 0;
    }
    else
    {
      pars.color.r = 255; pars.color.g = pars.color.b = 0;
    }
    pars.font = FNT_BIG;
    pars.alignment = ALN_TEXT;
    pars.box.x = gr.timeXPos();
    pars.box.y = gr.timeYPos();
    pars.box.w = 50;
    pars.box.h = 50;
    pars.shadow = true;

    renderText(target.getVideo(), &pars, time);
  }
}


