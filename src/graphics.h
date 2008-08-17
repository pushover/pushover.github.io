#include <SDL.h>
#include <string>
#include <vector>

/* this class contains all the informationf or all graphics */
class graphics_c {

  public:

    graphics_c(void);

    virtual void loadGraphics(void) = 0;

    /* load a new theme, and activate it, maybe that very theme is already
     * loaded and just a pointer is replaced
     */
    void setTheme(const char *name);

    /* to get the resolution that should be used */
    virtual unsigned int resolutionX(void) = 0;
    virtual unsigned int resolutionY(void) = 0;

    /* to get the blocksize of one block */
    virtual unsigned int blockX(void) = 0;
    virtual unsigned int blockY(void) = 0;

    SDL_Surface * getBgTile(unsigned int num) { return bgTiles[curTheme][num]; }
    SDL_Surface * getFgTile(unsigned int num) { return fgTiles[curTheme][num]; }

    SDL_Surface * getDomino(unsigned int domino, unsigned int image) { return dominos[domino][image]; }
    SDL_Surface * getAnt(unsigned int animation, unsigned int step) { return 0; }

    virtual void loadTheme(const char *name) = 0;

    static const unsigned char numDominoTypes;
    static const unsigned char numDominos[18];

  protected:

    /* some functions for the loaders to store the loaded images */
    void addBgTile(SDL_Surface * v);
    void addFgTile(SDL_Surface * v);

    void setDomino(unsigned int type, unsigned int num, SDL_Surface * v);

  private:

    std::vector<std::string> themeNames;

    std::vector<std::vector<SDL_Surface *> > bgTiles;
    std::vector<std::vector<SDL_Surface *> > fgTiles;

    std::vector<std::vector<SDL_Surface *> > dominos;

    unsigned int curTheme;

};
