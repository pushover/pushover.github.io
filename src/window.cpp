#include "window.h"

#include "screen.h"
#include "graphics.h"
#include "text.h"
#include "levelset.h"
#include "textsections.h"
#include "solvedmap.h"

#include <SDL.h>



window_c::window_c(unsigned char x_, unsigned char y_, unsigned char w_, unsigned char h_, surface_c & s, graphics_c & g) : x(x_), y(y_), w(w_), h(h_), surf(s), gr(g) {

  if (w < 2 || h < 2) return;

  for (unsigned int i = 0; i < w; i++)
    for (unsigned int j = 0; j < h; j++) {

      int xp = 1;
      int yp = 1;

      if (i == 0) xp = 0;
      if (i+1 == w) xp = 2;

      if (j == 0) yp = 0;
      if (j+1 == h) yp = 2;

      if (xp == 1 && yp == 1) continue;

      SDL_Surface * v = gr.getBoxBlock(yp*3+xp);

      SDL_Rect r;

      r.x = (x+i)*gr.blockX();
      r.y = (y+j)*gr.blockY();

      r.w = gr.blockX();
      r.h = gr.blockY();

      SDL_BlitSurface(v, 0, surf.getVideo(), &r);

      surf.markDirty(x+i, y+j);
    }

  done = false;

}

void window_c::clearInside(void) {

  for (int i = 0; i < w-2; i++)
    for (int j = 0; j < h-2; j++) {
      surf.markDirty(1+x+i, 1+y+j);

      SDL_Surface * v = gr.getBoxBlock(4);

      SDL_Rect r;

      r.x = (1+x+i)*gr.blockX();
      r.y = (1+y+j)*gr.blockY();

      r.w = gr.blockX();
      r.h = gr.blockY();

      SDL_BlitSurface(v, 0, surf.getVideo(), &r);
    }
}

window_c::~window_c(void) {
  for (unsigned int i = 0; i < w; i++)
    for (unsigned int j = 0; j < h; j++)
      surf.markDirty(x+i, y+j);
}

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

helpWindow_c::helpWindow_c(const std::string text, surface_c & s, graphics_c & g) : window_c(3, 2, 14, 9, s, g), help(text) {

  clearInside();

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
  par.alignment = ALN_CENTER;
  par.color.r = par.color.g = 255; par.color.b = 0;
  par.shadow = false;
  par.box.x = 400-3*55;
  par.box.w = 6*55;
  par.box.y = 235;
  par.box.h = 145;

  renderText(s.getVideo(), &par, text);
}

bool helpWindow_c::handleEvent(const SDL_Event & event) {
  if (event.type == SDL_KEYDOWN &&
      event.key.keysym.sym == SDLK_ESCAPE)
  {
    done = true;
    return true;
  }

  return false;
}

void listWindow_c::redraw(void) {

  clearInside();

  fontParams_s par;

  par.font = FNT_BIG;
  par.alignment = ALN_CENTER;
  par.color.r = 112; par.color.g = 39; par.color.b = 0;
  par.shadow = false;
  par.box.x = gr.blockX()*(x+1);
  par.box.y = gr.blockY()*(y+1);
  par.box.w = gr.blockX()*(w-2);
  par.box.h = getFontHeight(FNT_BIG);

  renderText(surf.getVideo(), &par, title);

  int ypos = gr.blockY()*(y+1) + getFontHeight(FNT_BIG);

  SDL_Rect r;

  r.x = gr.blockX()*(x+1);
  r.y = ypos;
  r.w = gr.blockX()*(w-2);
  r.h = 2;

  ypos += 20;

  SDL_FillRect(surf.getVideo(), &r, SDL_MapRGB(surf.getVideo()->format, 112, 39, 0));

  unsigned int lineH = getFontHeight(FNT_NORMAL);  // hight of one entry line
  menuLines = (gr.blockY()*(y+h-1)-ypos) / lineH;  // number of entriy lines that fit

  unsigned int line = 0;

  if (entries.size() > menuLines)
  {
      // choose line so that current is in the middle of the selected Lines
      if (current > menuLines/2)
          line = current - menuLines/2;

      // make shure that the first line is shoosen so that the window is full
      if (line + menuLines > entries.size())
          line -= (line+menuLines-entries.size());
  }

  while (ypos + lineH < gr.blockY()*(y+h-1)) {

    if (line >= entries.size()) break;

    par.font = FNT_NORMAL;
    par.alignment = ALN_CENTER;

    if (line == current)
    {
      par.color.r = par.color.g = par.color.b = 255;
    }
    else
    {
      par.color.r = 112; par.color.g = 39; par.color.b = 0;
    }

    par.shadow = false;
    par.box.x = gr.blockX()*(x+1);
    par.box.y = ypos;
    par.box.w = gr.blockX()*(w-2);
    par.box.h = lineH;

    renderText(surf.getVideo(), &par, entries[line]);

    line++;
    ypos += lineH;
  }
}

listWindow_c::listWindow_c(int x, int y, int w, int h, surface_c & s, graphics_c & gr,
    const std::string & t, const std::vector<std::string> & e, bool esc, int initial)
  : window_c(x, y, w, h, s, gr), entries(e), title(t), current(initial), escape(esc)
{

  redraw();
}

bool listWindow_c::handleEvent(const SDL_Event & event) {

  if (window_c::handleEvent(event)) return true;

  if (event.type == SDL_KEYDOWN)
  {
    if (escape && event.key.keysym.sym == SDLK_ESCAPE)
    {
      current = entries.size();  // make sure nothing valis is selected
      done = true;
      return true;
    }
    if (event.key.keysym.sym == SDLK_RETURN)
    {
      done = true;
      return true;
    }
    else if (event.key.keysym.sym == SDLK_UP)
    {
      if (current > 0) {
        current--;
        redraw();
      }
      return true;
    }
    else if (event.key.keysym.sym == SDLK_DOWN)
    {
      if (current+1 < entries.size()) {
        current++;
        redraw();
      }
      return true;
    }
    else if (event.key.keysym.sym == SDLK_PAGEUP)
    {
        if (current > 0)
        {
            if (current > menuLines) current -= menuLines; else current = 0;
            redraw();
        }
    }
    else if (event.key.keysym.sym == SDLK_PAGEDOWN)
    {
        if (current+1 < entries.size())
        {
            if (current+menuLines < entries.size()) current += menuLines; else current = entries.size()-1;
            redraw();
        }
    }
  }

  return false;
}


listWindow_c * getMainWindow(surface_c & surf, graphics_c & gr) {
    static std::vector<std::string> entries;

    if (!entries.size())
    {
        entries.push_back("Play Levelset");
        entries.push_back("Configuration");
        entries.push_back("Quit");
    }

    return new listWindow_c(4, 3, 12, 6, surf, gr, "Main menu", entries, false);
}

listWindow_c * getConfigWindow(surface_c & surf, graphics_c & gr) {
    static std::vector<std::string> entries;

    if (!entries.size())
    {
        entries.push_back("Toggle Fullscreen");
        entries.push_back("Toggle Sound Effects");
    }

    return new listWindow_c(3, 2, 14, 9, surf, gr, "Configuration", entries, true);
}

listWindow_c * getMissionWindow(const levelsetList_c & ls, surface_c & surf, graphics_c & gr) {
    return new listWindow_c(4, 2, 12, 9, surf, gr, "Select Levelset", ls.getLevelsetNames(), true);
}

listWindow_c * getLevelWindow(const levelset_c & ls, const solvedMap_c & solv, surface_c & surf, graphics_c & gr) {
    std::vector<std::string> entries;

    int index = -1;

    for (unsigned int i = 0; i < ls.getLevelNames().size(); i++)
    {
        std::string e = ls.getLevelNames()[i];

        if (solv.solved(ls.getChecksum(e)))
            e += " (solved)";
        else if (index == -1)
            index = i;

        entries.push_back(e);
    }

    return new listWindow_c(4, 0, 12, 12, surf, gr, "Select Level", entries, true, index);
}

listWindow_c * getQuitWindow(surface_c & surf, graphics_c & gr) {
    static std::vector<std::string> entries;

    if (!entries.size())
    {
        entries.push_back("Return to level");
        entries.push_back("Restart level");
        entries.push_back("Return to Main menu");
    }

    return new listWindow_c(4, 3, 12, 6, surf, gr, "Nu What?", entries, true);
}

listWindow_c * getLevelWindow(levelset_c & ls, surface_c & surf, graphics_c & gr) {
    return new listWindow_c(3, 2, 14, 9, surf, gr, "Select Level", ls.getLevelNames(), true);
}

listWindow_c * getSolvedWindow(surface_c & surf, graphics_c & gr) {
    static std::vector<std::string> entries;

    if (!entries.size())
    {
        entries.push_back("Continue with next level");
        entries.push_back("Return to Main menu");
    }

    return new listWindow_c(2, 3, 16, 6, surf, gr, "Gratulation You solved the level", entries, false);
}

listWindow_c * getFailedWindow(const std::string & reason, surface_c & surf, graphics_c & gr) {
    static std::vector<std::string> entries;

    if (!entries.size())
    {
        entries.push_back("Retry the level");
        entries.push_back("Return to Main menu");
    }

    return new listWindow_c(3, 3, 14, 6, surf, gr, "You failed: " + reason, entries, false);
}


