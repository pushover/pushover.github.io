#include "window.h"

#include "screen.h"
#include "graphics.h"
#include "levelset.h"
#include "textsections.h"
#include "solvedmap.h"

#include <SDL.h>

#include <stdexcept>



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

      surf.blitBlock(gr.getBoxBlock(yp*3+xp), (x+i)*gr.blockX(), (y+j)*gr.blockY());
      surf.markDirty(x+i, y+j);
    }

  done = false;

}

void window_c::clearInside(void) {

  for (int i = 0; i < w-2; i++)
    for (int j = 0; j < h-2; j++) {

      surf.blitBlock(gr.getBoxBlock(4), (1+x+i)*gr.blockX(), (1+y+j)*gr.blockY());
      surf.markDirty(1+x+i, 1+y+j);
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
    s.fillRect(SX*positions[2*d+0]+TX,   SY*positions[2*d+1]+TY,   50,   80, 0, 0, 0);
    s.fillRect(SX*positions[2*d+0]+TX+2, SY*positions[2*d+1]+TY+2, 50-4, 80-4, 112, 39, 0);
    s.blitBlock(g.getDomino(d, 7), SX*positions[2*d+0]+TX - 80, SY*positions[2*d+1]+TY + 5);

    par.font = FNT_SMALL;
    par.alignment = ALN_CENTER;
    par.color.r = 112; par.color.g = 39; par.color.b = 0;
    par.shadow = false;
    par.box.x = SX*positions[2*d+0]+TX-15;
    par.box.w = 80;
    par.box.y = SY*positions[2*d+1]+TY-25;
    par.box.h = 20;

    s.renderText(&par, texts[d]);
  }

  par.font = FNT_NORMAL;
  par.alignment = ALN_CENTER;
  par.color.r = par.color.g = 255; par.color.b = 0;
  par.shadow = false;
  par.box.x = 400-3*55;
  par.box.w = 6*55;
  par.box.y = 235;
  par.box.h = 145;

  s.renderText(&par, text);
}

bool helpWindow_c::handleEvent(const SDL_Event & event) {
  if (event.type == SDL_KEYDOWN &&
      (event.key.keysym.sym == SDLK_ESCAPE ||
       event.key.keysym.sym == SDLK_RETURN)
     )
  {
    done = true;
    return true;
  }

  return false;
}

class aboutWindow_c : public window_c {

  public:

    aboutWindow_c(surface_c & s, graphics_c & g);
    bool handleEvent(const SDL_Event & event);
};

aboutWindow_c::aboutWindow_c(surface_c & s, graphics_c & g) : window_c(2, 1, 16, 10, s, g) {

  clearInside();

  fontParams_s par;

  par.font = FNT_BIG;
  par.alignment = ALN_CENTER;
  par.color.r = 112; par.color.g = 39; par.color.b = 0;
  par.shadow = false;
  par.box.x = gr.blockX()*(X()+1);
  par.box.y = gr.blockY()*(Y()+1);
  par.box.w = gr.blockX()*(W()-2);
  par.box.h = getFontHeight(FNT_BIG);

  surf.renderText(&par, "Pushover - About");

  int ypos = gr.blockY()*(Y()+1) + getFontHeight(FNT_BIG);

  surf.fillRect(gr.blockX()*(X()+1), ypos, gr.blockX()*(W()-2), 2, 112, 39, 0);
  ypos += 20;

  unsigned int lineH = getFontHeight(FNT_SMALL);  // hight of one entry line

  par.font = FNT_SMALL;
  par.alignment = ALN_TEXT;
  par.box.x = gr.blockX()*(X()+1);
  par.box.y = ypos;
  par.box.w = gr.blockX()*(W()-2)-30;
  par.box.h = lineH;

  par.box.y += surf.renderText(&par, "Original Concept:")*lineH; par.box.x += 30;
  par.box.y += surf.renderText(&par, "Chas Partington")*lineH; par.box.x -= 30;

  par.box.y += surf.renderText(&par, "Original Programming:")*lineH; par.box.x += 30;
  par.box.y += surf.renderText(&par, "Dave Elcock, Helen Elcock, Keith Watterson")*lineH; par.box.x -= 30;

  par.box.y += surf.renderText(&par, "Original Graphics:")*lineH; par.box.x += 30;
  par.box.y += surf.renderText(&par, "Bryan King, Barry Armstrong")*lineH; par.box.x -= 30;

  par.box.y += surf.renderText(&par, "Original Music & SFX:")*lineH; par.box.x += 30;
  par.box.y += surf.renderText(&par, "Keith Tinman, Dean Evans, Johnathan Dunn")*lineH; par.box.x -= 30;

  par.box.y += surf.renderText(&par, "Original Levels:")*lineH; par.box.x += 30;
  par.box.y += surf.renderText(&par, "Harry Nadler, Avril Rigby, Don Rigby, Chris Waterworth")*lineH; par.box.x -= 30;

  par.box.y += surf.renderText(&par, "New Programming:")*lineH; par.box.x += 30;
  par.box.y += surf.renderText(&par, "Andreas Röver, Volker Grabsch")*lineH; par.box.x -= 30;
}


bool aboutWindow_c::handleEvent(const SDL_Event & event) {
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

  surf.renderText(&par, title);

  int ypos = gr.blockY()*(y+1) + getFontHeight(FNT_BIG);

  surf.fillRect(gr.blockX()*(x+1), ypos, gr.blockX()*(w-2), 2, 112, 39, 0);
  ypos += 20;

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

    surf.renderText(&par, entries[line]);

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
        entries.push_back("About");
        entries.push_back("Quit");
    }

    return new listWindow_c(4, 3, 12, 7, surf, gr, "Main menu", entries, false);
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

    if (entries.size() == 0) throw std::runtime_error("No Level in Levelset");

    // when all levels have been solved, return to the first
    if (index == -1) index = 0;

    return new listWindow_c(4, 0, 12, 12, surf, gr, "Select Level", entries, true, index);
}

listWindow_c * getQuitWindow(surface_c & surf, graphics_c & gr) {
    static std::vector<std::string> entries;

    if (!entries.size())
    {
        entries.push_back("Return to level");
        entries.push_back("Restart level");
        entries.push_back("Return to menu");
    }

    return new listWindow_c(4, 3, 12, 6, surf, gr, "Nu What?", entries, true);
}

listWindow_c * getLevelWindow(levelset_c & ls, surface_c & surf, graphics_c & gr) {
    return new listWindow_c(3, 2, 14, 9, surf, gr, "Select Level", ls.getLevelNames(), true);
}

window_c * getAboutWindow(surface_c & surf, graphics_c & gr) {
    return new aboutWindow_c(surf, gr);
}

listWindow_c * getSolvedWindow(surface_c & surf, graphics_c & gr) {
    static std::vector<std::string> entries;

    if (!entries.size())
    {
        entries.push_back("Continue");
    }

    return new listWindow_c(2, 3, 16, 6, surf, gr, "Gratulation! You solved the level.", entries, false);
}

listWindow_c * getFailedWindow(const std::string & reason, surface_c & surf, graphics_c & gr) {
    static std::vector<std::string> entries;

    if (!entries.size())
    {
        entries.push_back("Retry the level");
        entries.push_back("Return to menu");
    }

    std::string title = std::string("You failed: ") + reason;

    unsigned int w = (getTextWidth(FNT_BIG, title) + gr.blockX() - 1) / gr.blockX();

    // make sure minimum size is fulfilled
    if (w < 14) w = 14;
    // try to accommodate the header in the withd
    if (w > 18) w = 18;
    // make sure widow is centerable by having an even width
    if (w & 1) w++;

    // add the border
    w += 2;

    return new listWindow_c((20-w)/2, 3, w, 6, surf, gr, title, entries, false);
}


