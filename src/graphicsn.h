#ifndef __GRAPHICS_N_H__
#define __GRAPHICS_N_H__

#include "graphics.h"

#include <string>

class pngLoader_c;

/* implementation for graphics class using the original graphics */
class graphicsN_c : public graphics_c {

  public:

    graphicsN_c(const std::string & path);
    ~graphicsN_c(void) {}

    void loadGraphics(void);

    void loadTheme(const std::string & name);

    virtual unsigned int resolutionX(void) const { return 800; }
    virtual unsigned int resolutionY(void) const { return 600; }

    /* to get the blocksize of one block */
    virtual unsigned int blockX(void) const { return 40; }
    virtual unsigned int blockY(void) const { return 48; }
    virtual unsigned int halveBlockDisplace(void) const { return 8*3; }
    virtual unsigned int antDisplace(void) const { return 6*3; }

    virtual signed int getCarryOffsetX(unsigned int animation, unsigned int image) const;
    virtual signed int getCarryOffsetY(unsigned int animation, unsigned int image) const ;
    virtual signed int getMoveOffsetX(unsigned int animation, unsigned int image) const;
    virtual signed int getMoveOffsetY(unsigned int animation, unsigned int image) const;
    virtual signed int getMoveImage(unsigned int animation, unsigned int image) const;

    virtual int timeXPos(void) const { return 5*18/2; }
    virtual int timeYPos(void) const { return 3*186; }
    virtual int getDominoYStart(void) const { return 3*4; }
    virtual int convertDominoX(int x) const { return 5*x/2; }
    virtual int convertDominoY(int y) const { return 3*y; }
    virtual int splitterY(void) const { return 3*12; }

  private:

    std::string dataPath;

    void getAnimation(int anim, pngLoader_c * png);

};

#endif

