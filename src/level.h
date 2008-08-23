#include <SDL.h>

class graphics_c;

class level_c {

  private:

    typedef struct levelEntry {
      unsigned short bg;
      unsigned char fg;
      unsigned char dominoType;
      unsigned char dominoState;
      char dominoDir;
      char dominoYOffset;
    } levelEntry;

    levelEntry level[13][20];

    char theme[10];

    // the positions of the 2 doors
    unsigned char doorEntryX, doorEntryY, doorExitX, doorExitY;

    /* 2 bitmasks containing a bit for each block saying if it changed
     * there is one array for the static background and one for the dynamic
     * foreground with the dominos and the ant, the clock, ...
     */
    uint32_t staticDirty[13];
    uint32_t dynamicDirty[13];

    /* this surface contains the background. It is only updated when necessary
     * the content it used to restore stuff behind the sprites
     */
    SDL_Surface * background;

    // requested stated for the 2 doors
    bool openDoorExit;
    bool openDoorEntry;

  public:

    level_c(void);
    ~level_c(void);

    void load(const char * name);

    const char * getTheme(void) const { return theme; }

    /* Foreground elements */
    enum {
      FgElementEmpty,
      FgElementPlatformStart,
      FgElementPlatformMiddle,
      FgElementPlatformEnd,
      FgElementPlatformLadderDown,
      FgElementLadder,
      FgElementPlatformLadderUp,
      FgElementPlatformStep1,
      FgElementPlatformStep2,
      FgElementPlatformStep3,
      FgElementPlatformStep4,
      FgElementPlatformStep5,
      FgElementPlatformStep6,
      FgElementPlatformStep7,
      FgElementPlatformStep8,
      FgElementPlatformWrongDoor,
      FgElementPlatformStack,
      FgElementLadderMiddle,
      FgElementPlatformStrip,
      FgElementLadder2,
      FgElementDoor0,
      FgElementDoor1,
      FgElementDoor2,
      FgElementDoor3
    };

    unsigned short getBg(unsigned int x, unsigned int y) const { return level[y][x].bg; }
    unsigned char  getFg(unsigned int x, unsigned int y) const { return level[y][x].fg; }
    unsigned char  getDominoType(unsigned int x, unsigned int y) const { return level[y][x].dominoType; }
    unsigned char  getDominoState(unsigned int x, unsigned int y) const { return level[y][x].dominoState; }

    /* update the background where necessary */
    void updateBackground(graphics_c * gr);

    /* draw the changed stuff into the target surface */
    void drawDominos(SDL_Surface * target, graphics_c * gr);

    /* opens and closes doors */
    void performDoors(void);
    // allows you to open and close the 2 doors
    void openEntryDoor(bool open) { openDoorEntry = open; }
    void openExitDoor(bool open) { openDoorExit = open; }
    bool isEntryDoorOpen(void) { return getFg(doorEntryX, doorEntryY) == FgElementDoor3; }
    bool isExitDoorOpen(void) { return getFg(doorExitX, doorExitY) == FgElementDoor3; }
    unsigned int getEntryDoorPosX(void) { return doorEntryX; }
    unsigned int getEntryDoorPosY(void) { return doorEntryY; }

    // dirty marking of blocks
    void markDirty(int x, int y) { if (x >= 0 && x < 20 && y >= 0 && y < 13) dynamicDirty[y] |= (1 << x); }
};
