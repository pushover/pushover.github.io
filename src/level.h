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
      char dominoExtra;  // this field contains a lot of information:
        // 0-20 for delay domino is the number of ticks left until it falls
        // 0-9  for splitter is the domino that splits the plitter (for display)
        // 0x40 for all dominos means it is falling of the edge and still turning, so
        //      please fall slower
        // 0x60 riser rising
        // 0x70 falling domino, pile of rubbish...
    } levelEntry;

    levelEntry level[13][20];

    char theme[10];

    // the positions of the 2 doors
    unsigned char doorEntryX, doorEntryY, doorExitX, doorExitY;

    // the number of 1/18 seconds that are left for solving the level
    int timeLeft;
    int Min, Sec;   // number of minutes and seconds shown in display

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

    // calls the different states of the dominos
    void callStateFunction(int type, int state, int x, int y);

    bool isTherePlatform(int x, int y);
    void DominoCrash(int x, int y, int type, int extra);

    void DTA_1(int x, int y);
    void DTA_2(int x, int y);
    void DTA_3(int x, int y);
    void DTA_4(int x, int y);
    void DTA_E(int x, int y);
    void DTA_I(int x, int y);
    void DTA_J(int x, int y);
    void DTA_K(int x, int y);
    void DTA_F(int x, int y);
    void DTA_C(int x, int y);
    void DTA_D(int x, int y);
    void DTA_5(int x, int y);
    void DTA_G(int x, int y);
    void DTA_6(int x, int y);
    void DTA_L(int x, int y);
    void DTA_7(int x, int y);
    void DTA_M(int x, int y);
    void DTA_8(int x, int y);
    void DTA_9(int x, int y);
    void DTA_N(int x, int y);
    void DTA_A(int x, int y);
    void DTA_H(int x, int y);
    void DTA_O(int x, int y);
    void DTA_B(int x, int y);

    bool triggerFalln;

  public:

    level_c(void);
    ~level_c(void);

    void load(const char * name);

    const char * getTheme(void) const { return theme; }

    /* Foreground elements */
    enum {
      FgElementEmpty,              // 0
      FgElementPlatformStart,
      FgElementPlatformMiddle,
      FgElementPlatformEnd,
      FgElementPlatformLadderDown,
      FgElementLadder,             // 5
      FgElementPlatformLadderUp,
      FgElementPlatformStep1,
      FgElementPlatformStep2,
      FgElementPlatformStep3,
      FgElementPlatformStep4,      // 10
      FgElementPlatformStep5,
      FgElementPlatformStep6,
      FgElementPlatformStep7,
      FgElementPlatformStep8,
      FgElementPlatformWrongDoor,  // 15  unused
      FgElementPlatformStack,      //     unused
      FgElementLadderMiddle,       //     used for ladder redraw but not in level
      FgElementPlatformStrip,
      FgElementLadder2,            //     used for ladder redraw but not in level
      FgElementDoor0,              // 20
      FgElementDoor1,              // used in door animation, but not in level
      FgElementDoor2,              // used in door animation, but not in level
      FgElementDoor3               // used in door animation, but not in level
    };

    enum {
      DominoTypeEmpty,
      DominoTypeStandard,
      DominoTypeBlocker,
      DominoTypeSplitter,
      DominoTypeExploder,
      DominoTypeDelay,
      DominoTypeTumbler,
      DominoTypeBridger,
      DominoTypeVanisher,
      DominoTypeTrigger,
      DominoTypeRiser,
      DominoTypeCrash0,
      DominoTypeCrash1,
      DominoTypeCrash2,
      DominoTypeCrash3,
      DominoTypeCrash4,
      DominoTypeCrash5,
      DominoTypeRiserCont,
      DominoTypeQuaver
    };

    unsigned short getBg(unsigned int x, unsigned int y) const { return level[y][x].bg; }
    unsigned char  getFg(unsigned int x, unsigned int y) const { return level[y][x].fg; }
    unsigned char  getDominoType(unsigned int x, unsigned int y) const { return level[y][x].dominoType; }
    unsigned char  getDominoState(unsigned int x, unsigned int y) const { return level[y][x].dominoState; }
    signed char  getDominoDir(unsigned int x, unsigned int y) const { return level[y][x].dominoDir; }
    unsigned char  getDominoExtra(unsigned int x, unsigned int y) const { return level[y][x].dominoExtra; }

    /* update the background where necessary */
    void updateBackground(graphics_c * gr);

    /* draw the changed stuff into the target surface */
    void drawDominos(SDL_Surface * target, graphics_c * gr, bool debug);

    /* opens and closes doors */
    void performDoors(void);
    // allows you to open and close the 2 doors
    void openEntryDoor(bool open) { openDoorEntry = open; }
    void openExitDoor(bool open) { openDoorExit = open; }
    bool isEntryDoorOpen(void) { return getFg(doorEntryX, doorEntryY) == FgElementDoor3; }
    bool isExitDoorOpen(void) { return getFg(doorExitX, doorExitY) == FgElementDoor3; }
    unsigned int getEntryDoorPosX(void) { return doorEntryX; }
    unsigned int getEntryDoorPosY(void) { return doorEntryY; }

    void performDominos(void);

    // query level information of certain places
    bool containsPlank(int x, int y);
    bool noGround(int x, int y, bool onLadder);  // returns true, if the ant can't stand

    int pickUpDomino(int x, int y);  // removes the domino from that position and returns the domino type
    void putDownDomino(int x, int y, int domino, bool pushin);
    void fallingDomino(int x, int y);

    bool pushDomino(int x, int y, int dir);
    void removeDomino(int x, int y) { level[y][x].dominoType = 0; }


    // dirty marking of blocks
    void markDirty(int x, int y) { if (x >= 0 && x < 20 && y >= 0 && y < 13) dynamicDirty[y] |= (1 << x); }
    bool isDirty(int x, int y) { return (dynamicDirty[y] & (1 << x)) != 0; }
    void clearDirty(void);


    void print(void);

    // check, if the level has been successfully solved
    // if not the reason for failure is in fail
    bool levelCompleted(int *fail);

    bool triggerIsFalln(void) { return triggerFalln; }

    bool someTimeLeft(void) { return timeLeft > 0; }

};
