#include "window.h"

#include "screen.h"
#include "graphics.h"
#include "levelset.h"
#include "textsections.h"
#include "solvedmap.h"

#include <SDL.h>

#include <stdexcept>
#include <libintl.h>



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

#define NUM_DOMINOS 11
static std::string texts[NUM_DOMINOS] = {
  _("Standard: nothing special about this stone, it simply falls"),
  _("Blocker: can not fall, may still stand at level end"),
  _("Splitter: when something falls on its top it will split in two"),
  _("Exploder: will blast a hole into the platform below it"),
  _("Delay: falls not immediately but a few seconds after beeing pushed"),
  _("Tumbler: will continue rolling until it hits an obstacle"),
  _("Bridger: will bridge the platform if there is a one unit gap"),
  _("Vanish: pushed the next domino but then vanishes"),
  _("Trigger: this is the last domino that must fall and it must lie flat"),
  _("Ascender: will raise as if filled with helium"),
  _("Entangled: all stones of this type will fall together as if quantum entangled"),
};

#define SX 310
#define SY 85
#define TX 100
#define TY 100

#define NUM_STONES_PER_PAGE 8

void helpWindow_c::displayCurrentPage(void)
{
  clearInside();

  fontParams_s par;

  for (unsigned int d = 0; d < NUM_STONES_PER_PAGE; d++)
  {
    // out of dominos
    if (NUM_STONES_PER_PAGE*page+d >= NUM_DOMINOS) break;

    int x = d % 2;
    int y = d / 2;

    s.fillRect(SX*x+TX,   SY*y+TY,   50,   75, 0, 0, 0);
    s.fillRect(SX*x+TX+2, SY*y+TY+2, 50-4, 75-4, 112, 39, 0);

    if (NUM_STONES_PER_PAGE*page+d == 10)
    {
      s.blitBlock(g.getDomino(10, 7), SX*x+TX - 80 - 9, SY*y+TY + 4);
      s.blitBlock(g.getDomino(11, 7), SX*x+TX - 80 + 9, SY*y+TY + 4);
    }
    else
    {
      s.blitBlock(g.getDomino(NUM_STONES_PER_PAGE*page+d, 7), SX*x+TX - 80, SY*y+TY + 4);
    }

    par.font = FNT_SMALL;
    par.alignment = ALN_TEXT;
    par.color.r = 112; par.color.g = 39; par.color.b = 0;
    par.shadow = 0;
    par.box.x = SX*x+TX+55;
    par.box.w = SX-65;
    par.box.y = SY*y+TY;
    par.box.h = 80;

    s.renderText(&par, texts[NUM_STONES_PER_PAGE*page+d]);
  }

  par.font = FNT_NORMAL;
  par.alignment = ALN_CENTER;
  par.color.r = par.color.g = 255; par.color.b = 0;
  par.shadow = 0;
  par.box.x = (800-16*40)/2;
  par.box.w = 16*40;
  par.box.y = TY+NUM_STONES_PER_PAGE*SY/2-10;
  par.box.h = 12*48-TY-NUM_STONES_PER_PAGE*SY/2-10;

  if (getTextHeight(&par, help) > par.box.h) {
    printf("%i  %i\n", getTextHeight(&par, help), par.box.h);
    par.font = FNT_SMALL;
  }

  s.renderText(&par, help);

  par.font = FNT_SMALL;
  par.box.x = (800-15*40)/2;
  par.box.w = 0;
  par.box.y = TY+NUM_STONES_PER_PAGE*SY/2;
  par.box.h = 0;
  if (page > 0)
  {
    s.renderText(&par, "<<");
  }

  par.box.x = (800+15*40)/2;
  if (NUM_STONES_PER_PAGE*(page+1) < NUM_DOMINOS)
  {
    s.renderText(&par, ">>");
  }

}

helpWindow_c::helpWindow_c(const std::string & t, surface_c & su, graphics_c & gr) : window_c(1, 1, 18, 11, su, gr), help(t), page(0),
  s(su), g(gr)
{
  displayCurrentPage();
}

bool helpWindow_c::handleEvent(const SDL_Event & event) {
  if (event.type == SDL_KEYDOWN)
  {
    if (   event.key.keysym.sym == SDLK_ESCAPE
        || event.key.keysym.sym == SDLK_RETURN
       )
    {
      done = true;
      return true;
    }
    if (event.key.keysym.sym == SDLK_LEFT)
    {
      if (page > 0)
      {
        page--;
        displayCurrentPage();
        return true;
      }
    }
    if (event.key.keysym.sym == SDLK_RIGHT)
    {
      if (NUM_STONES_PER_PAGE*(page+1) < NUM_DOMINOS)
      {
        page++;
        displayCurrentPage();
        return true;
      }
    }
  }

  return false;
}

class aboutWindow_c : public window_c {

  public:

    aboutWindow_c(surface_c & s, graphics_c & g);
    bool handleEvent(const SDL_Event & event);
};

aboutWindow_c::aboutWindow_c(surface_c & s, graphics_c & g) : window_c(2, 0, 16, 12, s, g) {

  clearInside();

  fontParams_s par;

  par.font = FNT_BIG;
  par.alignment = ALN_CENTER;
  par.color.r = 112; par.color.g = 39; par.color.b = 0;
  par.shadow = 2;
  par.box.x = gr.blockX()*(X()+1);
  par.box.y = gr.blockY()*(Y()+1);
  par.box.w = gr.blockX()*(W()-2);
  par.box.h = getFontHeight(FNT_BIG);

  surf.renderText(&par, _("Pushover - About"));

  int ypos = gr.blockY()*(Y()+1) + getFontHeight(FNT_BIG);

  surf.fillRect(gr.blockX()*(X()+1)+1, ypos+1, gr.blockX()*(W()-2), 2, 0, 0, 0);
  surf.fillRect(gr.blockX()*(X()+1), ypos, gr.blockX()*(W()-2), 2, 112, 39, 0);
  ypos += 20;

  unsigned int lineH = getFontHeight(FNT_SMALL);  // hight of one entry line

  par.font = FNT_SMALL;
  par.alignment = ALN_TEXT;
  par.box.x = gr.blockX()*(X()+1);
  par.box.y = ypos;
  par.box.w = gr.blockX()*(W()-2)-30;
  par.box.h = lineH;

  par.shadow = 2; par.box.y += surf.renderText(&par, _("Original Concept:"))*lineH; par.box.x += 30;
  par.shadow = 0; par.box.y += surf.renderText(&par, "Chas Partington")*lineH; par.box.x -= 30;

  par.shadow = 2; par.box.y += surf.renderText(&par, _("Original Programming:"))*lineH; par.box.x += 30;
  par.shadow = 0; par.box.y += surf.renderText(&par, "Dave Elcock, Helen Elcock, Keith Watterson")*lineH; par.box.x -= 30;

  par.shadow = 2; par.box.y += surf.renderText(&par, _("Original Graphics:"))*lineH; par.box.x += 30;
  par.shadow = 0; par.box.y += surf.renderText(&par, "Bryan King, Barry Armstrong")*lineH; par.box.x -= 30;

  par.shadow = 2; par.box.y += surf.renderText(&par, _("Original Music & SFX:"))*lineH; par.box.x += 30;
  par.shadow = 0; par.box.y += surf.renderText(&par, "Keith Tinman, Dean Evans, Johnathan Dunn")*lineH; par.box.x -= 30;

  par.shadow = 2; par.box.y += surf.renderText(&par, _("Original Levels:"))*lineH; par.box.x += 30;
  par.shadow = 0; par.box.y += surf.renderText(&par, "Harry Nadler, Avril Rigby, Don Rigby, Chris Waterworth")*lineH; par.box.x -= 30;

  par.shadow = 2; par.box.y += surf.renderText(&par, _("New Programming:"))*lineH; par.box.x += 30;
  par.shadow = 0; par.box.y += surf.renderText(&par, "Andreas Röver, Volker Grabsch")*lineH; par.box.x -= 30;

  par.shadow = 2; par.box.y += surf.renderText(&par, _("New Music:"))*lineH; par.box.x += 30;
  par.shadow = 0; par.box.y += surf.renderText(&par, "Roberto Lorenz")*lineH; par.box.x -= 30;

  par.shadow = 2; par.box.y += surf.renderText(&par, _("New Graphics:"))*lineH; par.box.x += 30;
  par.shadow = 0; par.box.y += surf.renderText(&par, "Harald Radke")*lineH; par.box.x -= 30;
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
  par.shadow = 2;
  par.box.x = gr.blockX()*(x+1);
  par.box.y = gr.blockY()*(y+1);
  par.box.w = gr.blockX()*(w-2);
  par.box.h = getFontHeight(FNT_BIG);

  surf.renderText(&par, title);

  int ypos = gr.blockY()*(y+1) + getFontHeight(FNT_BIG);

  surf.fillRect(gr.blockX()*(x+1)+1, ypos+1, gr.blockX()*(w-2), 2, 0, 0, 0);
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

    par.shadow = 0;
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
        entries.push_back(_("Play Levelset"));
        entries.push_back(_("Configuration"));
        entries.push_back(_("About"));
        entries.push_back(_("Quit"));
    }

    return new listWindow_c(4, 3, 12, 7, surf, gr, _("Main menu"), entries, false);
}

listWindow_c * getConfigWindow(surface_c & surf, graphics_c & gr) {
    static std::vector<std::string> entries;

    if (!entries.size())
    {
        entries.push_back(_("Toggle Fullscreen"));
        entries.push_back(_("Toggle Sound Effects"));
        entries.push_back(_("Toggle Background Music"));
    }

    return new listWindow_c(3, 2, 14, 9, surf, gr, _("Configuration"), entries, true);
}

listWindow_c * getMissionWindow(const levelsetList_c & ls, surface_c & surf, graphics_c & gr) {
    return new listWindow_c(4, 2, 12, 9, surf, gr, _("Select Levelset"), ls.getLevelsetNames(), true);
}

listWindow_c * getLevelWindow(const levelset_c & ls, const solvedMap_c & solv, surface_c & surf, graphics_c & gr) {
    std::vector<std::string> entries;

    int index = -1;

    for (unsigned int i = 0; i < ls.getLevelNames().size(); i++)
    {
        std::string e = ls.getLevelNames()[i];

        if (solv.solved(ls.getChecksum(e)))
            e =  std::string(gettext(e.c_str())) + " " + gettext(_("(done)"));
        else if (index == -1)
            index = i;

        entries.push_back(e);
    }

    if (entries.size() == 0) throw std::runtime_error(_("No Level in Levelset"));

    // when all levels have been solved, return to the first
    if (index == -1) index = 0;

    return new listWindow_c(4, 0, 12, 12, surf, gr, _("Select Level"), entries, true, index);
}

listWindow_c * getQuitWindow(surface_c & surf, graphics_c & gr) {
    static std::vector<std::string> entries;

    if (!entries.size())
    {
        entries.push_back(_("Return to level"));
        entries.push_back(_("Restart level"));
        entries.push_back(_("Configuration"));
        entries.push_back(_("Return to menu"));
    }

    return new listWindow_c(4, 3, 12, 7, surf, gr, _("Nu What?"), entries, true);
}

listWindow_c * getLevelWindow(levelset_c & ls, surface_c & surf, graphics_c & gr) {
    return new listWindow_c(3, 2, 14, 9, surf, gr, _("Select Level"), ls.getLevelNames(), true);
}

window_c * getAboutWindow(surface_c & surf, graphics_c & gr) {
    return new aboutWindow_c(surf, gr);
}

listWindow_c * getSolvedWindow(surface_c & surf, graphics_c & gr) {
    static std::vector<std::string> entries;

    if (!entries.size())
    {
        entries.push_back(_("Continue"));
    }

    return new listWindow_c(2, 3, 16, 6, surf, gr, _("Gratulation! You solved the level."), entries, false);
}

listWindow_c * getFailedWindow(int failReason, surface_c & surf, graphics_c & gr) {
    static std::vector<std::string> entries;

    if (!entries.size())
    {
        entries.push_back(_("Retry the level"));
        entries.push_back(_("Return to menu"));
    }

    std::string title;
    switch (failReason) {
      case 2: title = _("You failed: You've been too slow"); break;
      case 3: title = _("You failed: Some dominoes crashed"); break;
      case 4: title = _("You failed: Not all dominoes fell"); break;
      case 5: title = _("You failed: You died"); break;
      case 6: title = _("You failed: Trigger was not last to fall"); break;
      case 7: title = _("You failed: Trigger not flat on the ground"); break;
    }

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


