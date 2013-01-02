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

levelData_c::levelData_c(void) {
  for (unsigned int y = 0; y < 13; y++)
    for (unsigned int x = 0; x < 20; x++) {
      for (unsigned int i = 0; i < maxBg; i++)
        level[y][x].bg[i] = 0;
      level[y][x].fg = 0;
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

      level[y][x].dominoType = DominoTypeEmpty;
      level[y][x].dominoState = 8;
      level[y][x].dominoDir = 0;
      level[y][x].dominoYOffset = 0;
      level[y][x].dominoExtra = 0;

      switch (levelRows[y].c_str()[x]) {

        case '1':
          if (doorEntryDefined)
            throw format_error("duplicate entry door");
          doorEntryDefined = true;
          doorEntryX = x;
          doorEntryY = y;
          level[y][x].fg = FgElementDoor0;
          break;

        case '2':
          if (doorExitDefined)
            throw format_error("duplicate exit door");
          doorExitDefined = true;
          doorExitX = x;
          doorExitY = y;
          level[y][x].fg = FgElementDoor0;
          break;

        case ' ':
        case '^':
          if (x > 0 && levelRows[y].c_str()[x-1] == '/')
            level[y][x].fg = FgElementPlatformStep8;
          else
            level[y][x].fg = FgElementEmpty;
          break;

        case '$':
          level[y][x].fg = FgElementEmpty;
          level[y][x].dominoType = DominoTypeStandard;
          level[y][x].dominoExtra = 0x70;
          break;

        case '\\':
          if (y <= 0 || x <= 0)
            throw format_error("platform step '\\' has an invalid position");
          level[y-1][x-1].fg = FgElementPlatformStep1;
          level[y-1][x  ].fg = FgElementPlatformStep2;
          level[y  ][x-1].fg = FgElementPlatformStep3;
          level[y  ][x  ].fg = FgElementPlatformStep4;
          break;

        case '/':
          if (y <= 0 || x+1 >= 20)
            throw format_error("platform step '/' has an invalid position");
          level[y-1][x  ].fg = FgElementPlatformStep5;
          level[y-1][x+1].fg = FgElementPlatformStep6;
          level[y  ][x  ].fg = FgElementPlatformStep7;
          break;

        case 'H':
        case 'V':
          level[y][x].fg = FgElementLadder;
          break;

        case '.':
          level[y][x].fg = FgElementPlatformStrip;
          break;

        default:
          std::string::size_type dt = dominoChars.find_first_of(levelRows[y].c_str()[x]);
          if (dt == std::string::npos)
            throw format_error("invalid domino type");
          level[y][x].dominoType = dt;

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
            level[y][x].fg = FgElementPlatformLadderDown;
          else if (ladderAbove)
            level[y][x].fg = FgElementPlatformLadderUp;
          else if (!platformLeft && !platformRight)
            level[y][x].fg = FgElementPlatformStrip;
          else if (!platformLeft)
            level[y][x].fg = FgElementPlatformStart;
          else if (!platformRight)
            level[y][x].fg = FgElementPlatformEnd;
          else
            level[y][x].fg = FgElementPlatformMiddle;
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
        line >> level[y][x].bg[b];
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
      else switch (level[y][x].fg) {
        case FgElementEmpty:
        case FgElementPlatformStep2:
        case FgElementPlatformStep3:
        case FgElementPlatformStep5:
        case FgElementPlatformStep8:
        case FgElementDoor0:
        case FgElementDoor1:
        case FgElementDoor2:
        case FgElementDoor3:
          if (y > 0 && level[y-1][x].fg == FgElementPlatformLadderDown)
            line += '^';
          else if (level[y][x].dominoType != DominoTypeEmpty)
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
              || level[y+1][x].fg == FgElementPlatformLadderDown
              || level[y+1][x].fg == FgElementLadder
              || level[y+1][x].fg == FgElementPlatformLadderUp)
            line += 'H';
          else
            line += 'V';
          break;
        default:
          unsigned char dt = level[y][x].dominoType;
          if (dt == DominoTypeEmpty
              && level[y][x].fg == FgElementPlatformStrip)
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
        stream << ' ' << std::setfill(' ') << std::setw(3) << level[y][x].bg[b];
      stream << '\n';
    }
    stream << '+' << std::string((3+1)*20, '-') << '\n';
  }
}

void levelData_c::removeDomino(int x, int y) {
  level[y][x].dominoType = DominoTypeEmpty;
  level[y][x].dominoState = 0;
  level[y][x].dominoDir = 0;
  level[y][x].dominoYOffset = 0;
  level[y][x].dominoExtra = 0;
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

  for (unsigned int y = 0; y < 13; y++) {
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

bool levelData_c::noGround(int x, int y, bool onLadder) {

  if (y >= 12) return true;

  if (getFg(x, y) == FgElementEmpty) return true;

  if (getFg(x, y) >= FgElementDoor0) return true;

  if (getFg(x, y) != FgElementLadder) return false;

  if (onLadder) return false;

  return true;
}

// return true, if there is a platform at the given position
bool levelData_c::isTherePlatform(int x, int y) {

  switch (level[y][x].fg)
  {
    case FgElementEmpty:              return false;
    case FgElementPlatformStart:      return true;
    case FgElementPlatformMiddle:     return true;
    case FgElementPlatformEnd:        return true;
    case FgElementPlatformLadderDown: return true;
    case FgElementLadder:             return false;
    case FgElementPlatformLadderUp:   return true;
    case FgElementPlatformStep1:      return true;
    case FgElementPlatformStep2:      return true;
    case FgElementPlatformStep3:      return false;
    case FgElementPlatformStep4:      return false;
    case FgElementPlatformStep5:      return true;
    case FgElementPlatformStep6:      return true;
    case FgElementPlatformStep7:      return false;
    case FgElementPlatformStep8:      return false;
    case FgElementLadderMiddle:       return false;
    case FgElementPlatformStrip:      return true;
    case FgElementLadder2:            return false;
    case FgElementDoor0:              return false;
    case FgElementDoor1:              return false;
    case FgElementDoor2:              return false;
    case FgElementDoor3:              return false;
    default:                          return false;
  }
}

bool levelData_c::levelCompleted(int & fail) const {

  for (int y = 0; y < 13; y++)
    for (int x = 0; x < 20; x++) {
      if (getDominoType(x, y) >= DominoTypeCrash0 &&
          getDominoType(x, y) <= DominoTypeCrash5)
      {
        fail = 3;
        return false;
      }

      if (getDominoType(x, y) != DominoTypeEmpty && getDominoType(x, y) != DominoTypeStopper)
      {
        if (getDominoType(x, y) == DominoTypeSplitter)
        { // for splitters is must have started to fall
          if (getDominoState(x, y) > 2 && getDominoState(x, y) <= 8)
          {
            fail = 6;
            return false;
          }
        }
        else if (getDominoType(x, y) == DominoTypeTrigger)
        { // triggers must as least have started to fall
          if (getDominoState(x, y) == 8 && !triggerIsFalln())
          {
            fail = 6;
            return false;
          }
        }
        else if (getDominoType(x, y) == DominoTypeTumbler)
        { // tumbler must lie on something or lean against a wall
          // not fallen far enough
          if (getDominoState(x, y) >= 3 && getDominoState(x, y) <= 13) {
            fail = 6;
            return false;
          }

          // fallen far enough but neighbor empty
          if (   getDominoState(x, y) <= 2
              && ((x < 1) || (getDominoType(x-1, y) == DominoTypeEmpty))
              && ((x < 2) || (getDominoType(x-2, y) == DominoTypeEmpty || getDominoState(x-2, y) < 14))
             )
          {
            fail = 6;
            return false;
          }
          if (   getDominoState(x, y) >= 14
              && ((x > 18) || (getDominoType(x+1, y) == DominoTypeEmpty))
              && ((x > 17) || (getDominoType(x+2, y) == DominoTypeEmpty || getDominoState(x+2, y) > 2))
             )
          {
            fail = 6;
            return false;
          }

        }
        else
        {
          if (getDominoState(x, y) == 8)
          {
            fail = 4;
            return false;
          }
          if (getDominoState(x, y) > 3 && getDominoState(x, y) < 13)
          {
            // here we certainly fail
            fail = 6;
            return false;
          }
          // in this case we might still succeed, when we lean against a block
          if (   getDominoState(x, y) == 3
              && (   getDominoType(x-1, y) != DominoTypeStopper
                  || getDominoType(x+1, y) == DominoTypeEmpty
                  || (getDominoDir(x+1, y) != -1 && getDominoType(x+1, y) != DominoTypeSplitter))
             )
          {
            // here we lean against a of a blocker and can't go back
            fail = 6;
            return false;
          }

          if (   getDominoState(x, y) == 13
              && (   getDominoType(x+1, y) != DominoTypeStopper
                  || getDominoType(x-1, y) == DominoTypeEmpty
                  || (getDominoDir(x-1, y) != 1 && getDominoType(x-1, y) != DominoTypeSplitter))
             )
          {
            // here we lean against a step
            fail = 6;
            return false;
          }
        }
      }
    }

  return true;
}

