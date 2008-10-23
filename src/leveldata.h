#ifndef __LEVEL_DATA_H__
#define __LEVEL_DATA_H__

#include <string>

class textsections_c;

class levelData_c {

  private:

    static const unsigned int version = 1;
    static const std::string dominoChars;
    static bool isDominoChar(char ch);
    static const unsigned char maxBg = 8;


    std::string name;
    std::string theme;
    std::string hint;

    std::string checksum;



  protected:

    typedef struct levelEntry {
      unsigned short bg[maxBg];
      unsigned char fg;
      unsigned char dominoType;
      unsigned char dominoState;
      char dominoDir;
      char dominoYOffset;
      char dominoExtra;  // this field contains a lot of information:
        // 0-20 for delay domino is the number of ticks left until it falls
        // 0-9  for splitter is the domino that splits the splitter (for display)
        // 0x40 for all dominos means it is falling of the edge and still turning, so
        //      please fall slower
        // 0x60 riser rising
        // 0x70 falling domino, pile of rubbish...
    } levelEntry;

    levelEntry level[13][20];
    unsigned char numBg;

    // the number of 1/18 seconds that are left for solving the level
    int timeLeft;

    // the positions of the 2 doors
    unsigned char doorEntryX, doorEntryY, doorExitX, doorExitY;

  public:
    levelData_c(void);
    virtual ~levelData_c(void) {}

    virtual void load(const textsections_c & sections);
    void save(std::ostream & stream) const;
    bool operator==(const levelData_c & other) const;

    const std::string getName(void) const { return name; }
    const std::string getTheme(void) const { return theme; }
    const std::string getHint(void) const { return hint; }

    const std::string getChecksum(void) const { return checksum; }
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
      DominoTypeStopper,
      DominoTypeSplitter,
      DominoTypeExploder,
      DominoTypeDelay,
      DominoTypeTumbler,
      DominoTypeBridger,
      DominoTypeVanish,
      DominoTypeTrigger,
      DominoTypeAscender,
      DominoTypeCrash0,         // all yello big pile
      DominoTypeCrash1,         // mixed big pile
      DominoTypeCrash2,         // all red big pile
      DominoTypeCrash3,         // all yellow little pile
      DominoTypeCrash4,         // mixed little pile
      DominoTypeCrash5,         // all red little pile
      DominoTypeRiserCont,
      DominoTypeQuaver
    };
    unsigned char getFg(unsigned int x, unsigned int y) const { return level[y][x].fg; }
    unsigned char getDominoType(unsigned int x, unsigned int y) const { return level[y][x].dominoType; }
    unsigned char getDominoState(unsigned int x, unsigned int y) const { return level[y][x].dominoState; }
    signed char   getDominoDir(unsigned int x, unsigned int y) const { return level[y][x].dominoDir; }
    unsigned char getDominoExtra(unsigned int x, unsigned int y) const { return level[y][x].dominoExtra; }

    unsigned int getEntryDoorPosX(void) { return doorEntryX; }
    unsigned int getEntryDoorPosY(void) { return doorEntryY; }
    bool isEntryDoorOpen(void) { return getFg(doorEntryX, doorEntryY) == FgElementDoor3; }
    bool isExitDoorOpen(void) { return getFg(doorExitX, doorExitY) == FgElementDoor3; }
    bool isExitDoorClosed(void) { return getFg(doorExitX, doorExitY) == FgElementDoor0; }

    // query level information of certain places
    bool containsPlank(int x, int y);
    bool noGround(int x, int y, bool onLadder);  // returns true, if the ant can't stand
    bool isTherePlatform(int x, int y);

    void print(void);

    bool someTimeLeft(void) { return timeLeft > 0; }
};


#endif

