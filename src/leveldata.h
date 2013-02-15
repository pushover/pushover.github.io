/* Pushover
 *
 * Pushover is the legal property of its developers, whose
 * names are listed in the COPYRIGHT file, which is included
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
  DominoTypeLastNormal = DominoTypeCounter3,
  DominoTypeCrash0,         // all yellow big pile
  DominoTypeCrash1,         // mixed big pile
  DominoTypeCrash2,         // all red big pile
  DominoTypeCrash3,         // all yellow little pile
  DominoTypeCrash4,         // mixed little pile
  DominoTypeCrash5,         // all red little pile
  DominoNumber
} DominoType;


// the following defines put down some basic numbers for the
// possible domino states... each domino stat is supposed
// to have exactly one meaning.. even though some
// dominos might not use all of them.. e.g. the exploder
// will have no state except for the upright one and
// some explostions

// the following defines represent single states
#define DO_ST_INVALID  0
#define DO_ST_LEFT     1  // the state which represents the domino falln completely to the left
#define DO_ST_UPRIGHT  8  // domino standing completely vertical
#define DO_ST_RIGHT   15  // domino falln completely to the right
#define DO_ST_NUM     57  // number of different state

// the following defines represent groups of states, the _E appendix is always the last entry of the group
#define DO_ST_FALLING    1  // stands for the initial state of the normally falling dominos
#define DO_ST_FALLING_E 15  // final of the falling states

// TODO right now the special ascender states interfere here, so later on

#define DO_ST_EXPLODE   16  // exploder start state, representing the finished explosion
#define DO_ST_EXPLODE_E 22  // exploder end state, representing the starting explosion

#define DO_ST_SPLIT   23   // first of the non standard splitter states
#define DO_ST_SPLIT_E 35   // last of the non standard splitter states

#define DO_ST_ASCENDER   36
#define DO_ST_ASCENDER_E 50

#define DO_ST_CRASH   51
#define DO_ST_CRASH_E 56


class levelData_c {

  private:

    static const unsigned int version = 2;
    static const std::string dominoChars;
    static bool isDominoChar(char ch);
    static const unsigned char maxBg = 8;

    std::string name;
    std::string theme;
    std::string hint;
    std::vector<std::string> author;
    std::string tutorial;

    std::string checksum;
    std::string checksumNoTime;

    // the number of 1/18 seconds that are left for solving the level
    int timeLeft;

    // the positions of the 2 doors
    // y position is the _lower_ block of the door, so the door
    // extends one block above that number
    unsigned char doorEntryX, doorEntryY, doorExitX, doorExitY;
    // open and close animation for door
    unsigned char doorEntryState, doorExitState;

    typedef struct levelEntry {
      unsigned short bg[maxBg];
      bool platform;
      bool ladder;
      DominoType dominoType;
      unsigned char dominoState;
      char dominoDir;
      char dominoYOffset;
      char dominoExtra;  // this field contains a lot of information:
        // 0-20 for delay domino is the number of ticks left until it falls
        // 0-9  for splitter is the domino that splits the splitter (for display)
        // 0x40 for all dominos means it is falling of the edge and still turning, so
        //      please fall slower
        // 0x50 raiser is clinging to the ceiling
        // 0x60 riser rising
        // 0x70 falling domino, pile of rubbish...
    } levelEntry;

    std::vector< std::vector<levelEntry> > level;
    unsigned char numBg;

  public:
    levelData_c(void);
    virtual ~levelData_c(void) {}

    virtual void load(const textsections_c & sections, const std::string & userString);
    void save(std::ostream & stream) const;
    bool operator==(const levelData_c & other) const;

    const std::string getName(void) const { return name; }
    const std::string getTheme(void) const { return theme; }
    const std::string getHint(void) const { return hint; }
    const std::vector<std::string> & getAuthor(void) const { return author; }
    const std::string getTutorial(void) const { return tutorial; }

    int getTimeLeft(void) const { return timeLeft; }
    void timeTick(void) { timeLeft--; }
    bool someTimeLeft(void) const { return timeLeft > 0; }

    const std::string getChecksum(void) const { return checksum; }
    const std::string getChecksumNoTime(void) const { return checksumNoTime; }

    unsigned char getNumBgLayer(void) const { return numBg; }
    void setNumBgLayer(unsigned char n) { numBg = n; }


    // OLD INTERFACE, Deprecated


    // NEW INTERFACE please use this only

    unsigned short getBg(unsigned int x, unsigned int y, int layer) const;

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

    bool isEntryDoorOpen(void) { return doorEntryState == 3; }
    bool isEntryDoorClosed(void) { return doorEntryState == 0; }
    bool isExitDoorOpen(void) { return doorExitState == 3; }
    bool isExitDoorClosed(void) { return doorExitState == 0; }

    bool getPlatform(unsigned int x, unsigned int y) const;
    bool getLadder(unsigned int x, unsigned int y) const;

    void setPlatform(unsigned int x, unsigned int y, bool val);
    void setLadder(unsigned int x, unsigned int y, bool val);

    void print(void);

    void removeDomino(int x, int y);

    DominoType getDominoType(unsigned int x, unsigned int y) const;
    unsigned char getDominoState(unsigned int x, unsigned int y) const;
    signed char   getDominoDir(unsigned int x, unsigned int y) const;
    unsigned char getDominoExtra(unsigned int x, unsigned int y) const;
    signed char getDominoYOffset(unsigned int x, unsigned int y) const;

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

