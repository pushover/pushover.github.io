#ifndef __TEXT_H__
#define __TEXT_H__

/* all kinds of text ouput... font handling, ... whatever stuff text related, including
 * internationalisation
 */

#include <SDL.h>

#include <string>

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
  bool shadow;
} fontParams_s;


void initText(std::string datadir);
void deinitText(void);

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
void renderText(SDL_Surface * v, const fontParams_s * par, const std::string & t);

unsigned int getFontHeight(unsigned int font);

#endif
