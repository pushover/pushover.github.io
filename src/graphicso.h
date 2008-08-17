#include "graphics.h"

#include <string>

/* implementation for graphics class using the original graphics */
class graphicsO_c : public graphics_c {

  public:

    graphicsO_c(const char * path, unsigned int scale);

    void loadGraphics(void);

    void loadTheme(const char *name);

    virtual unsigned int resolutionX(void) { return 320*scale; }
    virtual unsigned int resolutionY(void) { return 200*scale; }

    /* to get the blocksize of one block */
    virtual unsigned int blockX(void) { return 16*scale; }
    virtual unsigned int blockY(void) { return 16*scale; }

  private:

    std::string dataPath;
    unsigned int scale;

};
