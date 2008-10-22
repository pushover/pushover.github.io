#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <SDL.h>

/* this is the display class, there is only one instance fo this class and it contains
 * the complete screen
 * the background is always a level.
 * In front of that can be stacked windows
 */

class graphics_c;


class surface_c {

  protected:

    SDL_Surface * video;

  private:

    // the dirty blocks
    uint32_t dynamicDirty[13];

  public:

    // maybe temporarily??? return the video surface to draw something
    SDL_Surface * getVideo(void) { return video; }

    void markDirty(int x, int y) { if (x >= 0 && x < 20 && y >= 0 && y < 13) dynamicDirty[y] |= (1 << x); }
    bool isDirty(int x, int y) { if (x >= 0 && x < 20 && y >= 0 && y < 13) return (dynamicDirty[y] & (1 << x)) != 0; else return false; }
    void clearDirty(void);
    void markAllDirty(void);
};

class screen_c : public surface_c {

  private:

    const graphics_c & gr;
    int animationState;
    bool fullscreen;

  public:

    // constructor, does nothing for the time beeing
    screen_c(const graphics_c & gr);
    ~screen_c(void);

    // initialized the screen as long as this is not done, all the actions will do nothing
    void init(void);

    void flipComplete(void);  // flips the complete screen, not looking at the dirty blocks
    void flipDirty(void);     // updates only the dirty blocks
    bool flipAnimate(void);   // updates only the dirty blocks, but does that step by step resultin in an blending effetc return true, when done

    void toggleFullscreen(void);
};

#endif

