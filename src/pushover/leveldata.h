/* Pushover
 *
 * Pushover is the legal property of its developers, whose
 * names are listed in the AUTHORS file, which is included
 * within the source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA
 */

#ifndef __LEVEL_DATA_H__
#define __LEVEL_DATA_H__

#include <string>
#include <vector>

#include <stdint.h>

class textsections_c;

// This enumeration contains all the available dominoes that can be whithin the level
typedef enum {
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
  DominoTypeConnectedA,
  DominoTypeConnectedB,
  DominoTypeCounter1,
  DominoTypeCounter2,
  DominoTypeCounter3,
  DominoTypeLastNormal = DominoTypeCounter3, // dominoes up to this domino are within the level files
  DominoTypeCrash0,         // all yellow big pile
  DominoTypeCrash1,         // mixed big pile
  DominoTypeCrash2,         // all red big pile
  DominoTypeCrash3,         // all yellow little pile
  DominoTypeCrash4,         // mixed little pile
  DominoTypeCrash5,         // all red little pile
  DominoNumber
} DominoType;


// Each of the dominoes has a state, initially DO_ST_UPRIGHT
// the following defines put down some basic numbers for the
// possible domino states... each domino stat is supposed
// to have exactly one meaning.. even though some
// dominoes might not use all of them.. e.g. the exploder
// will have no state except for the upright one and
// some explostions

// the following defines represent single states
#define DO_ST_INVALID  0
#define DO_ST_LEFT     1  // the state which represents the domino falln completely to the left
#define DO_ST_UPRIGHT  8  // domino standing completely vertical
#define DO_ST_RIGHT   15  // domino falln completely to the right
#define DO_ST_LEFT_AS 36   // domino floating at the ceiling "falln" completely to the left
#define DO_ST_DOWNRIGHT 43 // domino at the ceiling and standing
#define DO_ST_RIGHT_AS 50  // domino at the ceiling falln right

#define DO_ST_NUM     57  // number of different state

// the following defines represent groups of states, the _E appendix is always the last entry of the group
#define DO_ST_FALLING    1  // stands for the initial state of the normally falling dominoes
#define DO_ST_FALLING_E 15  // final of the falling states

#define DO_ST_EXPLODE   16  // exploder start state, representing the finished explosion
#define DO_ST_EXPLODE_E 22  // exploder end state, representing the starting explosion

#define DO_ST_SPLIT   23   // first of the non standard splitter states
#define DO_ST_SPLIT_E 35   // last of the non standard splitter states

// the states, where the domino clings to the ceiling
#define DO_ST_ASCENDER   36
#define DO_ST_ASCENDER_E 50

// crash images, starting with dust cloud... that is dissolving and at the end only the
// pile of rubbish is left
#define DO_ST_CRASH   51
#define DO_ST_CRASH_E 56

// this class represents a compete level
class levelData_c {

  private:

    // the current file version for level
    static const unsigned int version = 2;
    // a string containing the characters to use for the different dominoes
    static const std::string dominoChars;
    // a function that returns true, if the given character is a valid one for a domino
    static bool isDominoChar(char ch);

    std::string name;    // name of level
    std::string theme;   // theme of level represented as a string
    std::string hint;    // the hint for this level
    std::vector<std::string> author;
    std::string tutorial; // tutorial string, may be empty

    std::string checksum;       // the checksum for this level... this is user dependent for the user given initially
    std::string checksumNoTime; // same as above but sparing out the time of the level

    // the number of 1/18 seconds that are left for solving the level
    int timeLeft;

    // the positions of the 2 doors
    // y position is the _lower_ block of the door, so the door
    // extends one block above that number
    unsigned char doorEntryX, doorEntryY, doorExitX, doorExitY;
    // open and close animation for door
    unsigned char doorEntryState, doorExitState;

    // content of one cell of the level
    typedef struct levelEntry {
      std::vector <uint16_t> bg;
      bool platform;             // is there a platform in this cell
      bool ladder;               // is there a ladder in this cell
      DominoType dominoType;     // which domino is in this cell, the platform the domino stands on is one row below
      unsigned char dominoState; // the state the domino is in (see ebove for possible values
      char dominoDir;            // direction the domino is fallin -1: left, 0: not falling, 1: right
      char dominoYOffset;        // positive values lower the domino
      char dominoExtra;  // this field contains a lot of information:
        // 0-20 for delay domino is the number of ticks left until it falls
        // 0-9  for splitter is the domino that splits the splitter (for display)
        // 0x40 for all dominoes means it is falling of the edge and still turning, so
        //      please fall slower
        // 0x50 raiser is clinging to the ceiling
        // 0x60 riser rising
        // 0x70 falling domino, pile of rubbish...
    } levelEntry;

    std::vector< std::vector<levelEntry> > level;

    uint16_t numBg; // current maximum number of background layers


#ifdef DEBUG
    // functions to save a level in the old format
    uint16_t loadVersion;
    bool save_v1(std::ostream & stream) const;
    unsigned char getFg(unsigned int x, unsigned int y) const;
#endif


  public:

    // create an empty level with no background, no foreground and no dominoes
    levelData_c(void);
    virtual ~levelData_c(void) {}

    // load the level from the textsection and use the userString to calculate the checksums
    virtual void load(const textsections_c & sections, const std::string & userString);
    void save(std::ostream & stream) const;
    bool operator==(const levelData_c & other) const;

    const std::string getName(void) const { return name; }
    const std::string getTheme(void) const { return theme; }
    void setTheme(const std::string & th) { theme = th; }
    const std::string getHint(void) const { return hint; }
    void setHint(const std::string & h) { hint = h; }
    const std::vector<std::string> & getAuthor(void) const { return author; }
    std::vector<std::string> & getAuthor(void) { return author; }
    const std::string getTutorial(void) const { return tutorial; }

    void setName(const std::string & n) { name = n; }
    void setAuthor(const std::string & n) { author.push_back(n); }

    int getTimeLeft(void) const { return timeLeft; }
    void timeTick(void) { timeLeft--; }
    bool someTimeLeft(void) const { return timeLeft > 0; }
    void setTimeLeft(int tm) { timeLeft = tm; }

    const std::string getChecksum(void) const { return checksum; }
    const std::string getChecksumNoTime(void) const { return checksumNoTime; }

    unsigned char getNumBgLayer(void) const { return numBg; }
    unsigned short getBg(unsigned int x, unsigned int y, size_t layer) const;

    void setBg(unsigned int x, unsigned int y, size_t layer, uint16_t tile);

    void openEntryDoorStep(void) { doorEntryState++; }
    void closeEntryDoorStep(void) { doorEntryState--; }
    void openExitDoorStep(void) { doorExitState++; }
    void closeExitDoorStep(void) { doorExitState--; }

    // the y position defines the BASE of the door, so the other
    // halv if above it
    unsigned char getEntryX(void) const { return doorEntryX; }
    unsigned char getEntryY(void) const { return doorEntryY; }
    unsigned char getExitX(void) const { return doorExitX; }
    unsigned char getExitY(void) const { return doorExitY; }
    unsigned char getEntryState(void) const { return doorEntryState; }
    unsigned char getExitState(void) const { return doorExitState; }

    void setEntry(uint8_t x, uint8_t y) { doorEntryX = x; doorEntryY = y; }
    void setExit(uint8_t x, uint8_t y) { doorExitX = x; doorExitY = y; }

    bool isEntryDoorOpen(void) const { return doorEntryState == 3; }
    bool isEntryDoorClosed(void) const { return doorEntryState == 0; }
    bool isExitDoorOpen(void) const { return doorExitState == 3; }
    bool isExitDoorClosed(void) const { return doorExitState == 0; }

    bool getPlatform(unsigned int x, unsigned int y) const;
    bool getLadder(unsigned int x, unsigned int y) const;

    void setPlatform(unsigned int x, unsigned int y, bool val);
    void setLadder(unsigned int x, unsigned int y, bool val);

    // outputs most of the information for the level on stdout
    void print(void) const;

    // remove a domino from the level
    void removeDomino(int x, int y);

    DominoType getDominoType(unsigned int x, unsigned int y) const {
      if (y < level.size() && x < level[y].size())
        return level[y][x].dominoType;
      else
        return DominoTypeEmpty;
    }
    unsigned char getDominoState(unsigned int x, unsigned int y) const;
    signed char   getDominoDir(unsigned int x, unsigned int y) const;
    unsigned char getDominoExtra(unsigned int x, unsigned int y) const;
    signed char getDominoYOffset(unsigned int x, unsigned int y) const;
    bool dominoLeansLeft(unsigned int x, unsigned int y) const;
    bool dominoLeansRight(unsigned int x, unsigned int y) const;

    void setDominoType(unsigned int x, unsigned int y, DominoType val);
    void setDominoState(unsigned int x, unsigned int y, int val);
    void setDominoDir(unsigned int x, unsigned int y, int val);
    void setDominoExtra(unsigned int x, unsigned int y, int val);
    void setDominoYOffset(unsigned int x, unsigned int y, int val);

    size_t levelY(void) const { return level.size(); }
    size_t levelX(void) const { if (level.size() == 0) return 0; else return level[0].size(); }
};

bool levelContainsDomino(const levelData_c & l, DominoType d);
bool levelForegroundEmpty(const levelData_c & l, int16_t x, int16_t y, uint16_t w, uint16_t h);

#endif

