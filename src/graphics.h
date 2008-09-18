#include <SDL.h>
#include <string>
#include <vector>
#include <map>

/* this class contains all the informationf or all graphics */
class graphics_c {

  public:

    graphics_c(void);
    virtual ~graphics_c(void);

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
    virtual unsigned int halveBlockDisplace(void) = 0;  // return and noffset to actually place the objects
    virtual unsigned int antDisplace(void) = 0;  // return and noffset to actually place the objects
    virtual unsigned int dominoDisplace(void) = 0;  // return and noffset to actually place the objects

    SDL_Surface * getBgTile(unsigned int num) { return bgTiles[curTheme][num]; }
    SDL_Surface * getFgTile(unsigned int num) { return fgTiles[curTheme][num]; }

    SDL_Surface * getDomino(unsigned int domino, unsigned int image) { return dominos[domino][image]; }
    SDL_Surface * getAnt(unsigned int animation, unsigned int step) { return ant[animation][step].v; }
    int getAntOffset(unsigned int animation, unsigned int step) { return step<ant[animation].size()?ant[animation][step].ofs:0; }
    unsigned int getAntImages(unsigned int animation) { return ant[animation].size(); }
    SDL_Surface * getCarriedDomino(unsigned int domino, unsigned int image) { return carriedDominos[domino][image]; }

    // these are offsets that are used together with carried domino to displace
    // that domino
    virtual signed int getCarryOffsetX(unsigned int animation, unsigned int image) = 0;
    virtual signed int getCarryOffsetY(unsigned int animation, unsigned int image) = 0;

    virtual signed int getMoveOffsetX(unsigned int animation, unsigned int image) = 0;
    virtual signed int getMoveOffsetY(unsigned int animation, unsigned int image) = 0;
    virtual signed int getMoveImage(unsigned int animation, unsigned int image) = 0;

    virtual void loadTheme(const char *name) = 0;

    static const unsigned char numDominoTypes;
    static const unsigned char numDominos[18];

    static const unsigned char numAntAnimations;

    void putText(SDL_Surface * v, int x, int y, const wchar_t * text, Uint8 r, Uint8 g, Uint8 b, bool shadow);
    unsigned int textLen(const wchar_t * text, bool shadow);

    // the position of the time in the level
    virtual int timeXPos(void) = 0;
    virtual int timeYPos(void) = 0;

    virtual int getDominoYStart(void) = 0;
    virtual int convertDominoX(int x) = 0;
    virtual int convertDominoY(int y) = 0;
    virtual int splitterY(void) = 0;

  protected:

    /* some functions for the loaders to store the loaded images */
    void addBgTile(SDL_Surface * v);
    void addFgTile(SDL_Surface * v);

    // sets a specific domino for a specific domino type and animation state
    void setDomino(unsigned int type, unsigned int num, SDL_Surface * v);
    void setCarriedDomino(unsigned int type, unsigned int num, SDL_Surface * v);

    // add an image to a specific ant animation, the new image is added at the end
    // you must also provide an y-offset used when animating to displace the image
    // if free is false it is assumed that the Suface is used elsewhere and not freed
    // on deletion of object
    void addAnt(unsigned int anim, unsigned int img, signed char yOffset, SDL_Surface * v, bool free = true);

    void addBigGlyph(wchar_t c, SDL_Surface *);

  private:

    std::vector<std::string> themeNames;

    std::vector<std::vector<SDL_Surface *> > bgTiles;
    std::vector<std::vector<SDL_Surface *> > fgTiles;

    std::vector<std::vector<SDL_Surface *> > dominos;
    std::vector<std::vector<SDL_Surface *> > carriedDominos;

    typedef struct {
      SDL_Surface * v;
      int ofs;
      bool free;
    } antSprite;

    std::vector<std::vector<antSprite> > ant;

    unsigned int curTheme;

    std::map<wchar_t, SDL_Surface *> bigFont;

};
