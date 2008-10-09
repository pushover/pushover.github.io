#ifndef __GRAPHICS_N_H__
#define __GRAPHICS_N_H__

#include "graphics.h"

#include <string>

class pngLoader_c;

/* implementation for graphics class using the original graphics */
class graphicsN_c : public graphics_c {

  public:

    graphicsN_c(const char * path);
    ~graphicsN_c(void) {}

    void loadGraphics(void);

    void loadTheme(const std::string & name);

    virtual unsigned int resolutionX(void) { return 800; }
    virtual unsigned int resolutionY(void) { return 600; }

    /* to get the blocksize of one block */
    virtual unsigned int blockX(void) { return 40; }
    virtual unsigned int blockY(void) { return 48; }
    virtual unsigned int halveBlockDisplace(void) { return 8*3; }
    virtual unsigned int antDisplace(void) { return 6*3; }

    virtual signed int getCarryOffsetX(unsigned int animation, unsigned int image);
    virtual signed int getCarryOffsetY(unsigned int animation, unsigned int image);
    virtual signed int getMoveOffsetX(unsigned int animation, unsigned int image);
    virtual signed int getMoveOffsetY(unsigned int animation, unsigned int image);
    virtual signed int getMoveImage(unsigned int animation, unsigned int image);

    virtual int timeXPos(void) { return 5*18/2; }
    virtual int timeYPos(void) { return 3*186; }
    virtual int getDominoYStart(void) { return 3*4; }
    virtual int convertDominoX(int x) { return 5*x/2; }
    virtual int convertDominoY(int y) { return 3*y; }
    virtual int splitterY(void) { return 3*12; }

  private:

    std::string dataPath;

    void getAnimation(int anim, SDL_Surface * v, pngLoader_c * png);

};

#endif

