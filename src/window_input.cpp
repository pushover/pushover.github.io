#include "window.h"

#include "colors.h"
#include "screen.h"
#include "graphicsn.h"


class InputWindow_c : public window_c {

  private:
    std::string input;

    unsigned int cursorPosition;

    std::string title;

    void redraw(void);

  public:

    InputWindow_c(int x, int y, int w, int h, surface_c & s, graphicsN_c & gr,
        const std::string & title);

    // the the user has selected something
    const std::string getText(void) { return input; } // which list entry was selected

    virtual bool handleEvent(const SDL_Event & event);
};


InputWindow_c::InputWindow_c(int x, int y, int w, int h, surface_c & s, graphicsN_c & gr,
        const std::string & ti) : window_c(x, y, w, h, s, gr)
{
  title = ti;
  input = "";
  cursorPosition = 0;
  escape = false;
  redraw();
}


static size_t utf8EncodeOne(uint16_t uni, char *utf8, size_t bufspace)
{
  size_t bytes, si;

  if (uni <= 0x7F) {
    /* one byte, simple */
    if (bufspace < 1) return 0;
    utf8[0] = (char) uni;
    return 1;

  } else if (uni <= 0x7FF) { /* 0b110ddddd 10dddddd */
    /* two bytes */
    if (bufspace < 2) return 0;
    bytes = 2;
    utf8[0] = 0xC0 | (uni >> 6);

  } else  { /* 0b1110dddd 10dddddd 10dddddd */
    /* three bytes */
    if (bufspace < 3) return 0;
    bytes = 3;
    utf8[0] = 0xE0 | ((uni >> 12) & 0x0F);

  }

  for (si = bytes - 1; si > 0; si--) {
    utf8[si] = 0x80 | (uni & 0x3F);
    uni >>= 6;
  }

  return bytes;
}

bool InputWindow_c::handleEvent(const SDL_Event & event)
{
  if (window_c::handleEvent(event)) return true;

  if (event.type == SDL_KEYDOWN)
  {
    if (event.key.keysym.sym == SDLK_ESCAPE)
    {
      escape = true;
      done = true;
      return true;
    }
    else if (event.key.keysym.sym == SDLK_RETURN)
    {
      done = true;
      return true;
    }
    else if (event.key.keysym.sym == SDLK_LEFT)
    {
      if (cursorPosition > 0) {
        cursorPosition--;
        while (   (cursorPosition > 0)
               && ((input[cursorPosition] & 0xC0) == 0x80)
              )
        {
          cursorPosition--;
        }
        redraw();
      }
      return true;
    }
    else if (event.key.keysym.sym == SDLK_RIGHT)
    {
      if (cursorPosition+1 <= input.length()) {
        cursorPosition++;
        while (   (cursorPosition < input.length())
               && ((input[cursorPosition] & 0xC0) == 0x80)
              )
        {
          cursorPosition++;
        }
        redraw();
      }
      return true;
    }
    else if (event.key.keysym.sym == SDLK_HOME)
    {
        if (cursorPosition > 0)
        {
          cursorPosition = 0;
          redraw();
        }
    }
    else if (event.key.keysym.sym == SDLK_END)
    {
        if (cursorPosition+1 < input.length())
        {
          cursorPosition = input.length();
          redraw();
        }
    }
    else if (event.key.keysym.sym == SDLK_BACKSPACE)
    {
      while (   (cursorPosition > 0)
             && ((input[cursorPosition-1] & 0xC0) == 0x80)
            )
      {
        input.erase(cursorPosition-1, 1);
        cursorPosition--;
      }
      if (cursorPosition > 0)
      {
        input.erase(cursorPosition-1, 1);
        cursorPosition--;
      }
      redraw();
    }
    else if (event.key.keysym.sym == SDLK_DELETE)
    {
      if (cursorPosition < input.length())
      {
        input.erase(cursorPosition, 1);

        while (   (cursorPosition < input.length())
               && ((input[cursorPosition] & 0xC0) == 0x80)
              )
        {
          input.erase(cursorPosition, 1);
        }
        redraw();
      }
    }
    else if (event.key.keysym.unicode >= 32)
    {
      if (getTextWidth(FNT_NORMAL, input+(char)event.key.keysym.unicode) < gr.blockX()*(w-2))
      {
        char utf8[10];
        size_t s = utf8EncodeOne(event.key.keysym.unicode, utf8, 10);

        for (size_t i = 0; i < s; i++)
        {
          input.insert(cursorPosition, 1, utf8[i]);
          cursorPosition++;
        }
        redraw();
      }
    }
    else
    {
    }
  }

  return false;
}

void InputWindow_c::redraw(void)
{
  clearInside();

  fontParams_s par;

  par.font = FNT_BIG;
  par.alignment = ALN_CENTER;
  par.color.r = TXT_COL_R; par.color.g = TXT_COL_G; par.color.b = TXT_COL_B;
  par.shadow = 2;
  par.box.x = gr.blockX()*(x+1);
  par.box.y = gr.blockY()*(y+1);
  par.box.w = gr.blockX()*(w-2);
  par.box.h = getFontHeight(FNT_BIG);

  surf.renderText(&par, title);

  int ypos = gr.blockY()*(y+1) + getFontHeight(FNT_BIG);

  surf.fillRect(gr.blockX()*(x+1)+1, ypos+1, gr.blockX()*(w-2), 2, 0, 0, 0);
  surf.fillRect(gr.blockX()*(x+1), ypos, gr.blockX()*(w-2), 2, TXT_COL_R, TXT_COL_G, TXT_COL_B);

  ypos += 20;

  par.alignment = ALN_TEXT;
  par.font = FNT_NORMAL;
  par.shadow = 0;

  unsigned int wi = getTextWidth(FNT_NORMAL, input.substr(0, cursorPosition));

  surf.fillRect(gr.blockX()*(x+1)+wi, ypos, 4, getFontHeight(FNT_NORMAL), 0, 0, 0);

  par.box.y = ypos;
  surf.renderText(&par, input);
}


window_c * getProfileInputWindow(surface_c & surf, graphicsN_c & gr)
{
  return new InputWindow_c(4,2,12,5, surf, gr, _("Enter new profile name"));
}

window_c * getNewLevelWindow(surface_c & surf, graphicsN_c & gr)
{
  return new InputWindow_c(4,2,12,5, surf, gr, _("Enter name for the new level"));
}

window_c * getAuthorsAddWindow(surface_c & surf, graphicsN_c & gr)
{
  return new InputWindow_c(4,2,12,5, surf, gr, _("Enter new authors name"));
}

