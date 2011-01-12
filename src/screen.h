#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <SDL.h>

#include <string>

/* this is the display class, there is only one instance fo this class and it contains
 * the complete screen
 * the background is always a level.
 * In front of that can be stacked windows
 */

class graphics_c;

/* the 3 vavailable fonts, they are a fixed size and are supposed to contain
 * all the required letters
 */
enum {
  FNT_SMALL,
  FNT_NORMAL,
  FNT_BIG
};

enum {
  ALN_TEXT,
  ALN_CENTER,
  ALN_TEXT_CENTER
};


typedef struct {
  int font;
  SDL_Rect box;
  int alignment;
  SDL_Color color;
  uint8_t shadow;
} fontParams_s;


void initText(std::string datadir);
void deinitText(void);
unsigned int getFontHeight(unsigned int font);
unsigned int getTextWidth(unsigned int font, const std::string & t);
unsigned int getTextHeight(const fontParams_s * par, const std::string & t);


class surface_c {

  protected:

    SDL_Surface * video;

  private:

    // the dirty blocks
    uint32_t dynamicDirty[13];

  public:

    SDL_Surface * getIdentical(void) const;

    surface_c(void) : video(0) {}
    surface_c(SDL_Surface * c) : video(c) {}
    ~surface_c(void);

    void markDirty(int x, int y) { if (x >= 0 && x < 20 && y >= 0 && y < 13) dynamicDirty[y] |= (1 << x); }
    bool isDirty(int x, int y) {
      if (x >= 0 && x < 20 && y >= 0 && y < 13)
        return (dynamicDirty[y] & (1 << x)) != 0;
      else
        return false;
    }
    void clearDirty(void);
    void markAllDirty(void);

    // blit the complete surface s so that the lower left corner of x is at x, y
    void blit(SDL_Surface * s, int x, int y);
    void blitBlock(SDL_Surface * s, int x, int y);
    void copy(surface_c & src, int x, int y, int w, int h);
    void fillRect(int x, int y, int w, int h, int r, int g, int b);

    // render a given UFT-8 encoded text to the surface v into the given box
    // the text is automatically broken into lines and newlines within the
    // text are considered to be new paragraphs
    // depending on the alignment mode the text is either:
    //   put as paragraph text starting with the upper corner, no block alignment, left or right
    //   alignment depending on language
    //   centered within the box
    //
    // the text will not leave the assign box, if it is too big it will be clipped
    // the given text will be put through gettext for translation before it is displayed
    // the function behaves a bit like printf in that you can format values into the
    // string with additional parameters
    //
    // return value is the number of lines that were output
    unsigned int renderText(const fontParams_s * par, const std::string & t);

    // apply the gradient within the givne area
    void gradient(int x, int y, int w, int h);
};

class pixelSurface_c : public surface_c {

  public:

    // creates a surface of ther same size and same format at the given surface
    pixelSurface_c(const surface_c & pre);
};

class screen_c : public surface_c {

  private:

    const graphics_c & gr;
    int animationState;

  public:

    // constructor, does nothing for the time beeing
    screen_c(const graphics_c & gr);
    ~screen_c(void);

    void flipComplete(void);  // flips the complete screen, not looking at the dirty blocks
    void flipDirty(void);     // updates only the dirty blocks
    bool flipAnimate(void);   // updates only the dirty blocks, but does that step by step resultin in an blending effetc return true, when done

    void toggleFullscreen(void);
};

// used for internationalisation to mark strings to translate
#define _(x) x

#endif

