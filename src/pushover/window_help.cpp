#include "window.h"

#include "colors.h"
#include "screen.h"
#include "graphicsn.h"

class helpWindow_c : public window_c {

  private:
    const levelData_c & level;
    DominoType carried;
    const std::string & mission;
    std::vector<uint32_t> pages;
    uint32_t nextPage;

    surface_c & s;
    graphicsN_c & g;

  private:

    void displayCurrentPage(void);

  public:

    helpWindow_c(const std::string & mission, const levelData_c & level, DominoType carried, surface_c & s, graphicsN_c & g);
    bool handleEvent(const SDL_Event & event);

};

#define NUM_DOMINOES 12
static struct {
  uint16_t numDominoes;
  DominoType dominoes[3];
  uint16_t boxWidth;
  uint16_t spacing;
  std::string text;
} dominoHelp[NUM_DOMINOES] = {
  { 1, {DominoTypeStandard},   50,  0, _("Standard: nothing special about this stone, it simply falls") },
  { 1, {DominoTypeStopper},    50,  0, _("Blocker: can not fall, only stone allowed to stand at level end") },
  { 1, {DominoTypeSplitter},   50,  0, _("Splitter: when something falls on its top it will split in two") },
  { 1, {DominoTypeExploder},   50,  0, _("Exploder: will blast a hole into the platform below it") },
  { 1, {DominoTypeDelay},      50,  0, _("Delay: falls not immediately but a while after being pushed") },
  { 1, {DominoTypeTumbler},    50,  0, _("Tumbler: will continue rolling until it hits an obstacle") },
  { 1, {DominoTypeBridger},    50,  0, _("Bridger: will connect the platform if there is a gap of one unit") },
  { 1, {DominoTypeVanish},     50,  0, _("Vanish: pushes next block but then vanishes, only stone you may place in front of doors") },
  { 1, {DominoTypeTrigger},    50,  0, _("Trigger: the last domino that must fall and it must lie flat, can not be moved") },
  { 1, {DominoTypeAscender},   50,  0, _("Ascender: will raise to ceiling when pushed and then flip up there") },
  { 2, {DominoTypeConnectedA,
        DominoTypeConnectedB}, 50, 18, _("Entangled: all stones of this type will fall together as if quantum entangled") },
  { 3, {DominoTypeCounter1,
        DominoTypeCounter2,
        DominoTypeCounter3}, 60, 15, _("Semiblocker: these behave like blocker as long as there is a stone still standing that has more lines") },
};

#define SX 310
#define SY 85
#define TX 100
#define TY 100

void helpWindow_c::displayCurrentPage(void)
{
  clearInside();

  fontParams_s par;

  uint32_t page = *(pages.rbegin());
  uint32_t ypos = (Y()+1)*gr.blockY();

  std::string help;

  if (level.someTimeLeft())
    help = _("Arrange dominoes in a run so that trigger falls last. You have 1 push.");
  else
    help = level.getHint();

  if (page == 0)
  {
    par.font = FNT_NORMAL;
    par.alignment = ALN_TEXT_CENTER;
    par.color.r = HLP_COL_R; par.color.g = HLP_COL_G; par.color.b = HLP_COL_B;
    par.shadow = 0;
    par.box.x = (800-16*40)/2;
    par.box.w = 16*40;
    par.box.y = ypos;
    par.box.h = (H()-2)/3*gr.blockY();

    if (getTextHeight(&par, _(help)) > par.box.h) {
      par.font = FNT_SMALL;
    }

    s.renderText(&par, _(help));

    ypos += getTextHeight(&par, _(help)) + 10;

    s.fillRect(par.box.x, ypos, par.box.w, 2, TXT_COL_R, TXT_COL_G, TXT_COL_B);

    ypos += 12;

    par.font = FNT_SMALL;
    par.shadow = false;
    par.color.r = TXT_COL_R; par.color.g = TXT_COL_G; par.color.b = TXT_COL_B;
    par.box.y = ypos;
    par.box.h = getFontHeight(FNT_SMALL);
    par.alignment = ALN_TEXT;
    s.renderText(&par, _("Level information"));
    ypos += getFontHeight(FNT_SMALL);

    par.box.y = ypos;
    par.box.x += 10;
    par.box.w -= 20;

    std::string n;

    if (mission != "")
      //TRANSLATORS: this is the separator between the levelset and the levelname in the help window
      n = std::string(_(mission)) + _(" / ") + _(level.getName());
    else
      n = _(level.getName());

    s.renderText(&par, std::string(_("Level name:")) + " " + n);

    ypos += getFontHeight(FNT_SMALL);

    par.box.y = ypos;

    std::vector<std::string> authors = level.getAuthor();

    std::string a;

    for (size_t i = 0; i < authors.size(); i++)
    {
      //TRANSLATORS: this is the separator that is placed between the different authors in the autor list in the help window
      if (a != "") a = a + _(", ");

      a += authors[i];
    }

    s.renderText(&par, std::string(_("Level author:")) + " " + a);

    ypos += getFontHeight(FNT_SMALL);
    ypos += 10;

    par.box.x -= 10;
    par.box.w += 20;
    s.fillRect(par.box.x, ypos, par.box.w, 2, TXT_COL_R, TXT_COL_G, TXT_COL_B);

    ypos += 12;
  }

  nextPage = 0;

  uint32_t column = 0;
  uint32_t linehight = SY;

  while (page < NUM_DOMINOES)
  {
    bool dominoInLevel = false;

    for (int i = 0; i < dominoHelp[page].numDominoes; i++)
      if (   levelContainsDomino(level, dominoHelp[page].dominoes[i])
          || (carried == dominoHelp[page].dominoes[i]))
      {
        dominoInLevel = true;
        break;
      }

    if (!dominoInLevel)
    {
      page++;
      continue;
    }

    int displaywidth = dominoHelp[page].boxWidth;

    par.font = FNT_SMALL;
    par.alignment = ALN_TEXT;
    par.color.r = TXT_COL_R; par.color.g = TXT_COL_G; par.color.b = TXT_COL_B;
    par.shadow = 0;

    if (rightToLeft())
    {
      par.box.x = SX*column+TX;
    }
    else
    {
      par.box.x = SX*column+TX+displaywidth+5;
    }
    par.box.w = SX-displaywidth-15;
    par.box.y = ypos;
    par.box.h = 80;

    if (  (ypos + getTextHeight(&par, _(dominoHelp[page].text.c_str())) > (Y()+H()-1)*gr.blockY())
        ||(ypos + 75                                                    > (Y()+H()-1)*gr.blockY())
       )
    {
      nextPage = page;
      break;
    }

    s.renderText(&par, _(dominoHelp[page].text.c_str()));

    {
      uint32_t h = getTextHeight(&par, _(dominoHelp[page].text.c_str()));
      h += 10;
      if (h > linehight)
        linehight = h;
    }

    int rlOffset = rightToLeft() ? SX-displaywidth-10 : 0;

    s.fillRect(rlOffset+SX*column+TX,   ypos,   displaywidth,   75, 0, 0, 0);
    s.fillRect(rlOffset+SX*column+TX+2, ypos+2, displaywidth-4, 75-4, TXT_COL_R, TXT_COL_G, TXT_COL_B);

    for (int i = 0; i < dominoHelp[page].numDominoes; i++)
      s.blitBlock(*g.getHelpDominoImage(dominoHelp[page].dominoes[i]),
          rlOffset+SX*column+TX-105+displaywidth/2+int(dominoHelp[page].spacing*(1.0*i-(dominoHelp[page].numDominoes-1)*0.5)), ypos + 4);

    page++;
    column++;
    if (column == 2)
    {
      ypos += linehight;
      column = 0;
      linehight = SY;
    }
  }

  par.font = FNT_SMALL;
  par.box.x = (800-14*40)/2;
  par.box.w = 0;
  par.box.y = (Y()+H()-1)*gr.blockY();
  par.box.h = 0;
  par.color.r = HLP_COL_R; par.color.g = HLP_COL_G; par.color.b = HLP_COL_B;

  if (*(pages.rbegin()) > 0)
  {
    s.renderText(&par, "<<");
  }

  par.box.x = (800+14*40)/2;

  if (nextPage > 0)
  {
    s.renderText(&par, ">>");
  }
}

helpWindow_c::helpWindow_c(const std::string & m, const levelData_c & l, DominoType c, surface_c & su, graphicsN_c & gr) :
  window_c(1, 1, 18, 11, su, gr), level(l), carried(c), mission(m),
  s(su), g(gr)
{
  pages.push_back(0);
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
      if (pages.size() > 1)
      {
        pages.pop_back();
        displayCurrentPage();
        return true;
      }
    }
    if (event.key.keysym.sym == SDLK_RIGHT)
    {

      if (nextPage > 0)
      {
        pages.push_back(nextPage);
        displayCurrentPage();
        return true;
      }
    }
  }

  return false;
}


window_c * getHelpWindow(const std::string & mission, const levelData_c & level, DominoType carried, surface_c & surf, graphicsN_c & gr)
{
  return new helpWindow_c(mission, level, carried, surf, gr);
}

