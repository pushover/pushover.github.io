#include "window.h"

#include "screen.h"
#include "graphics.h"
#include "text.h"

#include <SDL.h>

window_c::window_c(unsigned char x_, unsigned char y_, unsigned char w_, unsigned char h_, surface_c & s, graphics_c & gr) : x(x_), y(y_), w(w_), h(h_), surf(s) {

  if (w < 2 || h < 2) return;

  for (unsigned int i = 0; i < w; i++)
    for (unsigned int j = 0; j < h; j++) {
      surf.markDirty(x+i, y+j);

      int xp = 1;
      int yp = 1;

      if (i == 0) xp = 0;
      if (i+1 == w) xp = 2;

      if (j == 0) yp = 0;
      if (j+1 == h) yp = 2;


      SDL_Surface * v = gr.getBoxBlock(yp*3+xp);

      SDL_Rect r;

      r.x = (x+i)*gr.blockX();
      r.y = (y+j)*gr.blockY();

      r.w = gr.blockX();
      r.h = gr.blockY();

      SDL_BlitSurface(v, 0, surf.getVideo(), &r);
    }



}

void window_c::handleEvent(void) {}
void window_c::redrawBlock(unsigned char x, unsigned char y) {}

static int positions[20] = {

  0, 0,
  1, 0,
  2, 0,
  3, 0,
  0, 1,
  3, 1,
  0, 2,
  1, 2,
  2, 2,
  3, 2
};

static std::string texts[20] = {
  "Standard",
  "Blocker",
  "Splitter",
  "Exploder",
  "Delay",
  "Tumbler",
  "Bridger",
  "Vanish",
  "Trigger",
  "Ascender"
};

#define SX 135
#define SY 125
#define TX 175
#define TY 155

helpwindow_c::helpwindow_c(const std::string text, surface_c & s, graphics_c & g) : window_c(3, 2, 14, 9, s, g), help(text) {

  fontParams_s par;

  for (unsigned int d = 0; d < 10; d++)
  {
    SDL_Rect r;

    r.x = SX*positions[2*d+0]+TX;
    r.y = SY*positions[2*d+1]+TY;
    r.w = 50;
    r.h = 80;

    SDL_FillRect(s.getVideo(), &r, SDL_MapRGB(s.getVideo()->format, 0, 0, 0));

    r.x += 2;
    r.y += 2;
    r.w -= 4;
    r.h -= 4;

    SDL_FillRect(s.getVideo(), &r, SDL_MapRGB(s.getVideo()->format, 112, 39, 0));

    SDL_Surface * v = g.getDomino(d, 7);

    r.x = SX*positions[2*d+0]+TX  - 80;
    r.y = SY*positions[2*d+1]+TY  + 5;
    r.w = v->w;
    r.h = v->h;

    SDL_BlitSurface(v, 0, s.getVideo(), &r);

    par.font = FNT_SMALL;
    par.alignment = ALN_CENTER;
    par.color.r = 112; par.color.g = 39; par.color.b = 0;
    par.shadow = false;
    par.box.x = SX*positions[2*d+0]+TX-15;
    par.box.w = 80;
    par.box.y = SY*positions[2*d+1]+TY-25;
    par.box.h = 20;

    renderText(s.getVideo(), &par, texts[d]);
  }

  par.font = FNT_NORMAL;
  par.alignment = ALN_TEXT_CENTER;
  par.color.r = par.color.g = 255; par.color.b = 0;
  par.shadow = false;
  par.box.x = 400-3*55;
  par.box.w = 6*55;
  par.box.y = 240;
  par.box.h = 160;

  renderText(s.getVideo(), &par, text);
}

