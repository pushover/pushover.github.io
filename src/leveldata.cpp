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

#include "leveldata.h"

#include "textsections.h"
#include "sha1.h"
#include "graphics.h"
#include "soundsys.h"
#include "screen.h"
#include "ant.h"

#include <stdio.h>

#include <sstream>
#include <iomanip>

#include <assert.h>

levelData_c::levelData_c(void) {
  for (unsigned int y = 0; y < 27; y++)
    for (unsigned int x = 0; x < 20; x++) {
      for (unsigned int i = 0; i < maxBg; i++)
        level[y][x].bg[i] = 0;
      level[y][x].ladder = false;
      level[y][x].platform = false;
      level[y][x].dominoType = DominoTypeEmpty;
      level[y][x].dominoState = 8;
      level[y][x].dominoDir = 0;
      level[y][x].dominoYOffset = 0;
      level[y][x].dominoExtra = 0;
    }

  // invalid time...
  timeLeft = 60*60*18;
}

const std::string levelData_c::dominoChars =
  "_" /* DominoTypeEmpty    */
  "I" /* DominoTypeStandard */
  "#" /* DominoTypeStopper  */
  "Y" /* DominoTypeSplitter */
  "*" /* DominoTypeExploder */
  "?" /* DominoTypeDelay    */
  "O" /* DominoTypeTumbler  */
  "=" /* DominoTypeBridger  */
  ":" /* DominoTypeVanish   */
  "!" /* DominoTypeTrigger  */
  "A" /* DominoTypeAscender */
  "X" /* DominoTypeConnectedA */
  "x" /* DominoTypeConnectedA */
  "a" /* DominoTypeCounter1 */
  "b" /* DominoTypeCounter1 */
  "c" /* DominoTypeCounter1 */
  ;

bool levelData_c::isDominoChar(char ch) {
  return dominoChars.find_first_of(ch) != std::string::npos;
}

void levelData_c::load(const textsections_c & sections, const std::string & userString) {

  memset(level, 0, sizeof(level));

  /* Version section */
  {
    std::istringstream versionStream(sections.getSingleLine("Version"));
    unsigned int givenVersion;
    versionStream >> givenVersion;
    if (!versionStream.eof() || !versionStream)
      throw format_error("invalid level version");
  }

  /* Name section */
  name = sections.getSingleLine("Name");

  /* Theme section */
  theme = sections.getSingleLine("Theme");

  /* Time section (time format is M:SS) */
  {
    std::istringstream timeStream(sections.getSingleLine("Time"));
    unsigned int timeMinutes;
    timeStream >> timeMinutes;
    if (timeStream.get() != ':')
      throw format_error("invalid time format");
    unsigned int timeSeconds;
    timeStream >> timeSeconds;
    if (!timeStream.eof() || !timeStream)
      throw format_error("invalid time format");
    timeLeft = (((timeMinutes * 60) + timeSeconds) * 18) + 17;
  }

  /* Hint section */
  hint = sections.getSingleLine("Hint");
  if (hint == "")
    hint = "No hint!";

  /* Level section */
  const std::vector<std::string> & givenLevelRows =
    sections.getSingleSection("Level");
  if (givenLevelRows.size() != 13)
    throw format_error("wrong number of level rows");
  std::string levelRows[13];
  for (unsigned int y = 0; y < 13; y++) {
    std::string::size_type len = givenLevelRows[y].size();
    if (len > 20)
      throw format_error("level row is too long");
    /* padding with spaces to the whole width */
    levelRows[y] = givenLevelRows[y] + std::string(20-len, ' ');
  }

  bool doorEntryDefined = false;
  bool doorExitDefined = false;
  for (unsigned int y = 0; y < 13; y++) {
    for (unsigned int x = 0; x < 20; x++) {

      level[2*y][x].dominoType = DominoTypeEmpty;
      level[2*y][x].dominoState = 8;
      level[2*y][x].dominoDir = 0;
      level[2*y][x].dominoYOffset = 0;
      level[2*y][x].dominoExtra = 0;

      switch (levelRows[y].c_str()[x]) {

        case '1':
          if (doorEntryDefined)
            throw format_error("duplicate entry door");
          doorEntryDefined = true;
          doorEntryX = x;
          doorEntryY = 2*y+1;
          doorEntryState = 0;
          setFg(x, y, FgElementEmpty);
          break;

        case '2':
          if (doorExitDefined)
            throw format_error("duplicate exit door");
          doorExitDefined = true;
          doorExitX = x;
          doorExitY = 2*y+1;
          doorExitState = 0;
          setFg(x, y, FgElementEmpty);
          break;

        case ' ':
        case '^':
          if (x > 0 && levelRows[y].c_str()[x-1] == '/')
              setFg(x, y, FgElementPlatformStep8);
          else
              setFg(x, y, FgElementEmpty);
          break;

        case '$':
          setFg(x, y, FgElementEmpty);
          level[2*y][x].dominoType = DominoTypeStandard;
          level[2*y][x].dominoExtra = 0x70;
          break;

        case '\\':
          if (y <= 0 || x <= 0)
            throw format_error("platform step '\\' has an invalid position");
          setFg(x-1, y-1, FgElementPlatformStep1);
          setFg(x,   y-1, FgElementPlatformStep2);
          setFg(x-1, y,   FgElementPlatformStep3);
          setFg(x,   y,   FgElementPlatformStep4);
          break;

        case '/':
          if (y <= 0 || x+1 >= 20)
            throw format_error("platform step '/' has an invalid position");
          setFg(x,   y-1, FgElementPlatformStep5);
          setFg(x+1, y-1, FgElementPlatformStep6);
          setFg(x,   y,   FgElementPlatformStep7);
          break;

        case 'H':
        case 'V':
          setFg(x, y, FgElementLadder);
          break;

        case '.':
          setFg(x, y, FgElementPlatformStrip);
          break;

        default:
          std::string::size_type dt = dominoChars.find_first_of(levelRows[y].c_str()[x]);
          if (dt == std::string::npos)
            throw format_error("invalid domino type");
          level[2*y][x].dominoType = dt;

          bool ladderAbove   = y > 0
                               && levelRows[y-1].c_str()[x] == 'H';
          bool ladderBelow   = y+1 < 13
                               && (levelRows[y+1].c_str()[x] == 'H'
                                   || levelRows[y+1].c_str()[x] == 'V'
                                   || levelRows[y+1].c_str()[x] == '^');
          bool platformLeft  = x <= 0
                               || isDominoChar(levelRows[y].c_str()[x-1])
                               || levelRows[y].c_str()[x-1] == '\\';
          bool platformRight = x+1 >= 20
                               || isDominoChar(levelRows[y].c_str()[x+1])
                               || levelRows[y].c_str()[x+1] == '/';
          if (ladderBelow)
            setFg(x, y, FgElementPlatformLadderDown);
          else if (ladderAbove)
            setFg(x, y, FgElementPlatformLadderUp);
          else if (!platformLeft && !platformRight)
            setFg(x, y, FgElementPlatformStrip);
          else if (!platformLeft)
            setFg(x, y, FgElementPlatformStart);
          else if (!platformRight)
            setFg(x, y, FgElementPlatformEnd);
          else
            setFg(x, y, FgElementPlatformMiddle);
      }
    }
  }

  if (!doorEntryDefined)
    throw format_error("missing entry door");
  if (!doorExitDefined)
    throw format_error("missing exit door");

  /* Background sections */
  const std::vector<std::vector<std::string> > & bgSections =
    sections.getMultiSection("Background");
  numBg = bgSections.size();
  if (numBg == 0)
    throw format_error("missing Background section");
  if (numBg > maxBg)
    throw format_error("too many Background sections");
  for (unsigned char b = 0; b < numBg; b++) {
    if (bgSections[b].size() != 13)
      throw format_error("wrong number of background rows");
    for (unsigned int y = 0; y < 13; y++) {
      std::istringstream line(bgSections[b][y]);
      for (unsigned int x = 0; x < 20; x++) {
        line >> level[2*y][x].bg[b];
        level[2*y][x].bg[b] *= 2;
        level[2*y+1][x].bg[b] = level[2*y][x].bg[b]+1;

        if (!line)
          throw format_error("not enough background tiles in a row");
      }
      if (!line.eof())
        throw format_error("too many background tiles in a row");
    }
  }

  /* Calculate checksum */
  {
    SHA1 sha1;
    for (unsigned int y = 0; y < 13; y++) {
      sha1.update(levelRows[y]);
    }
    {
      std::ostringstream timeLeftStream;
      timeLeftStream << timeLeft;
      sha1.update(timeLeftStream.str());
    }
    sha1.update(userString);
    checksum = sha1.final();
  }
  {
    SHA1 sha1;
    for (unsigned int y = 0; y < 13; y++) {
      sha1.update(levelRows[y]);
    }
    sha1.update(userString);
    checksumNoTime = sha1.final();
  }
}

void levelData_c::save(std::ostream & stream) const {

  unsigned int timeSeconds = timeLeft/18;
  stream <<
  textsections_c::firstLine << "\n"
  "\n"
  "Version\n"
  "| " << version << "\n"
  "\n"
  "Name\n"
  "| " << name << "\n"
  "\n"
  "Theme\n"
  "| " << theme << "\n"
  "\n"
  "Time\n"
  "| " << (timeSeconds / 60)
       << ':'
       << std::setfill('0') << std::setw(2) << (timeSeconds % 60)
       << "\n"
  "\n"
  "Hint\n"
  "| " << hint << "\n";

  stream << '\n';
  stream << "Level\n";
  stream << '+' << std::string(20+1, '-') << '\n';
  for (unsigned int y = 0; y < 13; y++) {
    std::string line = "| ";
    for (unsigned int x = 0; x < 20; x++) {
      if (x == doorEntryX && y == doorEntryY)
        line += '1';
      else if (x == doorExitX && y == doorExitY)
        line += '2';
      else switch (getFg(x, y)) {
        case FgElementEmpty:
        case FgElementPlatformStep2:
        case FgElementPlatformStep3:
        case FgElementPlatformStep5:
        case FgElementPlatformStep8:
        case FgElementDoor0:
        case FgElementDoor1:
        case FgElementDoor2:
        case FgElementDoor3:
          if (y > 0 && getFg(x, y-1) == FgElementPlatformLadderDown)
            line += '^';
          else if (getDominoType(x, y) != DominoTypeEmpty)
            line += '$';
          else
            line += ' ';
          break;
        case FgElementPlatformStep4:
          line += '\\';
          break;
        case FgElementPlatformStep7:
          line += '/';
          break;
        case FgElementLadder:
          if (y+1 >= 13
              || getFg(x, y+1) == FgElementPlatformLadderDown
              || getFg(x, y+1) == FgElementLadder
              || getFg(x, y+1) == FgElementPlatformLadderUp)
            line += 'H';
          else
            line += 'V';
          break;
        default:
          unsigned char dt = getDominoType(x, y);
          if (dt == DominoTypeEmpty
              && getFg(x, y) == FgElementPlatformStrip)
            line += '.';
          else if (dt < dominoChars.size())
            line += dominoChars[dt];
          else
            throw format_error("level contains a domino type which can't be saved");
      }
    }
    stream << line.substr(0, line.find_last_not_of(' ')+1) << '\n';
  }
  stream << '+' << std::string(20+1, '-') << '\n';

  for (unsigned char b = 0; b < numBg; b++) {
    stream << '\n';
    stream << "Background\n";
    stream << '+' << std::string((3+1)*20, '-') << '\n';
    for (unsigned int y = 0; y < 13; y++) {
      stream << '|';
      for (unsigned int x = 0; x < 20; x++)
        stream << ' ' << std::setfill(' ') << std::setw(3) << getBg(x, y, b);
      stream << '\n';
    }
    stream << '+' << std::string((3+1)*20, '-') << '\n';
  }
}

void levelData_c::removeDomino(int x, int y) {
  level[2*y][x].dominoType = DominoTypeEmpty;
  level[2*y][x].dominoState = 0;
  level[2*y][x].dominoDir = 0;
  level[2*y][x].dominoYOffset = 0;
  level[2*y][x].dominoExtra = 0;
}


bool levelData_c::operator==(const levelData_c & other) const {
  return
    name == other.name &&
    theme == other.theme &&
    hint == other.hint &&
    timeLeft == other.timeLeft &&
    memcmp(level, other.level, sizeof(level)) == 0 &&
    numBg == other.numBg &&
    doorEntryX == other.doorEntryX &&
    doorEntryY == other.doorEntryY &&
    doorExitX == other.doorExitX &&
    doorExitY == other.doorExitY;
}

void levelData_c::print(void) {

  char directions[] = "<0>";

  printf("------------\n");

  for (unsigned int y = 0; y < 25; y++) {
    for (unsigned int x = 0; x < 20; x++)
      if (level[y][x].platform)
        printf("-");
      else
        printf(" ");
    printf("\n");
  }
  printf("------------\n");

  for (unsigned int y = 0; y < 25; y++) {
    for (unsigned int x = 0; x < 20; x++)
      printf("%x", level[y][x].dominoType);
    printf("  ");
    for (unsigned int x = 0; x < 20; x++)
      printf("%x", level[y][x].dominoState);
    printf("  ");
    for (unsigned int x = 0; x < 20; x++)
      printf("%c", directions[level[y][x].dominoDir+1]);
    printf("  ");
    for (unsigned int x = 0; x < 20; x++)
      printf("%c", 'a'+level[y][x].dominoYOffset);
    printf("  ");
    for (unsigned int x = 0; x < 20; x++)
      printf("%02x", level[y][x].dominoExtra);
    printf("\n");
  }

  printf("\n");

}

unsigned short levelData_c::getBg(unsigned int x, unsigned int y, int layer) const {
    return level[y][x].bg[layer];
}

bool levelData_c::getPlatform(unsigned int x, unsigned int y) const {
  return level[y][x].platform;
}

bool levelData_c::getLadder(unsigned int x, unsigned int y) const {
  return level[y][x].ladder;
}

unsigned char levelData_c::getFg(unsigned int x, unsigned int y) const
{
    // check special case for doors
    if (x == doorEntryX && 2*y+1 == doorEntryY)
        return FgElementDoor0 + doorEntryState;

    if (x == doorExitX && 2*y+1 == doorExitY)
        return FgElementDoor0 + doorExitState;

    // create Bitmask of upper layer, lower layer and upper layer below with each 3 neighbors
    uint16_t platformBits = 0;
    if (x > 0  && level[2*y+0][x-1].platform) platformBits |= 0x100;
    if (          level[2*y+0][x+0].platform) platformBits |= 0x080;
    if (x < 20 && level[2*y+0][x+1].platform) platformBits |= 0x040;

    if (x > 0  && level[2*y+1][x-1].platform) platformBits |= 0x020;
    if (          level[2*y+1][x+0].platform) platformBits |= 0x010;
    if (x < 20 && level[2*y+1][x+1].platform) platformBits |= 0x008;

    if (x > 0  && level[2*y+2][x-1].platform) platformBits |= 0x004;
    if (          level[2*y+2][x+0].platform) platformBits |= 0x002;
    if (x < 20 && level[2*y+2][x+1].platform) platformBits |= 0x001;

    uint16_t ladderBits = 0;
    if (level[2*y+0][x].ladder) ladderBits |= 0x02;
    if (level[2*y+1][x].ladder) ladderBits |= 0x01;

    switch (platformBits)
    {
        case 0x000:
        case 0x001:
        case 0x004:
        case 0x008:
        case 0x020:
        case 0x028:
        case 0x40:
        case 0x100:
        case 0x140:
        case 0x108:
            switch (ladderBits)
            {
                case 0: return FgElementEmpty;
                case 2: return FgElementLadderMiddle;
                case 3: return FgElementLadder;
                default:
                        printf("ladder %i\n", ladderBits);
                        assert(false); break;
            }
            break;

        case 0x010:
            assert(ladderBits == 0);
            return FgElementPlatformStrip;

        case 0x038:
        case 0x138: //
        case 0x03C: //
        case 0x078: //
        case 0x07C: //
        case 0x039: //
        case 0x03D: //
            switch (ladderBits)
            {
                case 0: return FgElementPlatformMiddle;
                case 3: return FgElementPlatformLadderDown;
                case 2: return FgElementPlatformLadderUp;
                default:
                        printf("ladder %i\n", ladderBits);
                        assert(false); break;
            }
            break;

        case 0x018:
        case 0x019:
        case 0x058:
        case 0x118:
            // assert(ladderBits == 0); in level 99 this is required...
            return  FgElementPlatformStart;

        case 0x030:
        case 0x034:
        case 0x130:
            // assert(ladderBits == 0); in level 28 required
            return FgElementPlatformEnd;

        case 0x133: //
        case 0x033: //
            assert(ladderBits == 0);
            return FgElementPlatformStep1; // 7

        case 0x026: //
            assert(ladderBits == 0);
            return FgElementPlatformStep2; // 8

        case 0x0C8: //
            assert(ladderBits == 0);
            return FgElementPlatformStep3; // 9

        case 0x199: //
        case 0x198: //
            assert(ladderBits == 0);
            return FgElementPlatformStep4; // 10

        case 0x00b: //
            assert(ladderBits == 0);
            return FgElementPlatformStep5; // 11

        case 0x01E: //
        case 0x05E: //
            assert(ladderBits == 0);
            return FgElementPlatformStep6; // 12

        case 0x0F0: //
        case 0x0F4: //
            assert(ladderBits == 0);
            return FgElementPlatformStep7; // 13

        case 0x1A0: //
        case 0x080:
            assert(ladderBits == 0);
            return FgElementPlatformStep8; // 14

        default:
            printf("unknown platform bitmask pos %i %i  =  %x\n", x, y, platformBits);
            return FgElementDoor3;
            assert(false); break;
    }

    assert(false);

}

void levelData_c::setPlatform(unsigned int x, unsigned int y, bool val)
{
  level[y][x].platform = val;
}

void levelData_c::setLadder(unsigned int x, unsigned int y, bool val)
{
  level[y][x].ladder = val;
}

void levelData_c::setFg(unsigned int x, unsigned int y, int val)
{
    switch(val)
    {
        case FgElementEmpty             :  // 0
        case FgElementDoor0             :  // 18
        case FgElementDoor1             :  // 19
        case FgElementDoor2             :  // 20
        case FgElementDoor3             :  // 21
        case FgElementPlatformStep2     :  // 8
        case FgElementPlatformStep5     :  // 11
            level[y*2  ][x].platform = false;
            level[y*2+1][x].platform = false;
            level[y*2  ][x].ladder   = false;
            level[y*2+1][x].ladder   = false;
            break;

        case FgElementPlatformStart     :  // 1
        case FgElementPlatformMiddle    :  // 2
        case FgElementPlatformEnd       :  // 3
        case FgElementPlatformStrip     :  // 16
        case FgElementPlatformStep1     :  // 7
        case FgElementPlatformStep6     :  // 12
            level[y*2  ][x].platform = false;
            level[y*2+1][x].platform = true;
            level[y*2  ][x].ladder   = false;
            level[y*2+1][x].ladder   = false;
            break;

        case FgElementPlatformLadderDown:  // 4
            level[y*2  ][x].platform = false;
            level[y*2+1][x].platform = true;
            level[y*2  ][x].ladder   = true;
            level[y*2+1][x].ladder   = true;
            break;

        case FgElementLadder            :  // 5
        case FgElementLadder2           :  // 17
            level[y*2  ][x].platform = false;
            level[y*2+1][x].platform = false;
            level[y*2  ][x].ladder   = true;
            level[y*2+1][x].ladder   = true;
            break;

        case FgElementPlatformLadderUp  :  // 6
            level[y*2  ][x].platform = false;
            level[y*2+1][x].platform = true;
            level[y*2  ][x].ladder   = true;
            level[y*2+1][x].ladder   = false;
            break;

        case FgElementLadderMiddle      :  // 15
            level[y*2  ][x].platform = false;
            level[y*2+1][x].platform = false;
            level[y*2  ][x].ladder   = true;
            level[y*2+1][x].ladder   = false;
            break;

        case FgElementPlatformStep3     :  // 9
        case FgElementPlatformStep8     :  // 14
            level[y*2  ][x].platform = true;
            level[y*2+1][x].platform = false;
            level[y*2  ][x].ladder   = false;
            level[y*2+1][x].ladder   = false;
            break;

        case FgElementPlatformStep4     :  // 10
        case FgElementPlatformStep7     :  // 13
            level[y*2  ][x].platform = true;
            level[y*2+1][x].platform = true;
            level[y*2  ][x].ladder   = false;
            level[y*2+1][x].ladder   = false;
            break;

        default:
            assert(false);
    }
}



