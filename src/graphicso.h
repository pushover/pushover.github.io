#ifndef __GRAPHICS_O_H__
#define __GRAPHICS_O_H__

#include "graphics.h"

#include <string>

/* implementation for graphics class using the original graphics */
class graphicsO_c : public graphics_c {

  public:

    graphicsO_c(const char * path, unsigned int scale);
    ~graphicsO_c(void) {}

    void loadGraphics(void);

    void loadTheme(const std::string & name);

    virtual unsigned int resolutionX(void) { return 320*scale; }
    virtual unsigned int resolutionY(void) { return 200*scale; }

    /* to get the blocksize of one block */
    virtual unsigned int blockX(void) { return 16*scale; }
    virtual unsigned int blockY(void) { return 16*scale; }
    virtual unsigned int halveBlockDisplace(void) { return 8*scale; }
    virtual unsigned int antDisplace(void) { return 6*scale; }

    virtual signed int getCarryOffsetX(unsigned int animation, unsigned int image);
    virtual signed int getCarryOffsetY(unsigned int animation, unsigned int image);
    virtual signed int getMoveOffsetX(unsigned int animation, unsigned int image);
    virtual signed int getMoveOffsetY(unsigned int animation, unsigned int image);
    virtual signed int getMoveImage(unsigned int animation, unsigned int image);

    virtual int timeXPos(void) { return scale*18; }
    virtual int timeYPos(void) { return scale*186; }
    virtual int getDominoYStart(void) { return scale*4; }
    virtual int convertDominoX(int x) { return scale*x; }
    virtual int convertDominoY(int y) { return scale*y; }
    virtual int splitterY(void) { return scale*12; }

  private:

    std::string dataPath;
    unsigned int scale;

    unsigned int getAnimation(unsigned char * data, unsigned char anim, unsigned short * palette);

};

#endif

