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
#include "sha1/sha1.hpp"
#include "graphics.h"
#include "soundsys.h"
#include "screen.h"
#include "ant.h"

#include <stdio.h>

#include <sstream>
#include <iomanip>

#include <assert.h>

#define LEVELSIZE_V1_X 20
#define LEVELSIZE_V1_Y 25

levelData_c::levelData_c(void)
{
  level.resize(LEVELSIZE_V1_Y);

  for (size_t y = 0; y < level.size(); y++)
  {
    level[y].resize(LEVELSIZE_V1_X);

    for (unsigned int x = 0; x < level[y].size(); x++) {
      level[y][x].bg.clear();
      level[y][x].ladder = false;
      level[y][x].platform = false;
      level[y][x].dominoType = DominoTypeEmpty;
      level[y][x].dominoState = 8;
      level[y][x].dominoDir = 0;
      level[y][x].dominoYOffset = 0;
      level[y][x].dominoExtra = 0;
    }
  }

  doorEntryState = 0;
  doorExitState = 0;

  doorEntryX = doorEntryY = doorExitX = doorExitY = 0;

  // invalid time...
  timeLeft = 60*60*18;

  numBg = 1;
  theme = "aztec";
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

  level.resize(LEVELSIZE_V1_Y);

  for (size_t i = 0; i < level.size(); i++)
    level[i].resize(LEVELSIZE_V1_X);

  /* Version section */
  unsigned int givenVersion;
  {
    std::istringstream versionStream(sections.getSingleLine("Version"));
    versionStream >> givenVersion;
    if (!versionStream.eof() || !versionStream)
      throw format_error("invalid level version");
  }
#ifdef DEBUG
  loadVersion = givenVersion;
#endif

  /* Name section */
  name = sections.getSingleLine("Name");

  /* Theme section */
  theme = sections.getSingleLine("Theme");

  /* Author section */
  if (sections.hasSection("Author"))
  {
    author = sections.getSingleSection("Author");
  }
  else
  {
    author.push_back(_("unknown"));
  }

  // optional tutorial section
  if (sections.hasSection("Tutorial"))
  {
    tutorial = sections.getSingleLine("Tutorial");
  }
  else
  {
    tutorial = "";
  }

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

  for (unsigned int y = 0; y < level.size(); y++) {
    for (unsigned int x = 0; x < level[y].size(); x++) {
      level[y][x].dominoType = DominoTypeEmpty;
      level[y][x].dominoState = 8;
      level[y][x].dominoDir = 0;
      level[y][x].dominoYOffset = 0;
      level[y][x].dominoExtra = 0;
      level[y][x].platform = false;
      level[y][x].ladder = false;

      level[y][x].bg.clear();
    }
  }

  doorEntryState = 0;
  doorExitState = 0;

  if (givenVersion == 1)
  {
    /* Level section */
    const std::vector<std::string> & givenLevelRows =
      sections.getSingleSection("Level");
    if (givenLevelRows.size() != 13)
      throw format_error("wrong number of level rows");
    std::string levelRows[13];
    for (unsigned int y = 0; y < 13; y++) {
      std::string::size_type len = givenLevelRows[y].size();
      if (len > LEVELSIZE_V1_X)
        throw format_error("level row is too long");
      /* padding with spaces to the whole width */
      levelRows[y] = givenLevelRows[y] + std::string(LEVELSIZE_V1_X-len, ' ');
    }

    bool doorEntryDefined = false;
    bool doorExitDefined = false;

    for (unsigned int y = 0; y < 13; y++) {
      for (unsigned int x = 0; x < LEVELSIZE_V1_X; x++) {

        switch (levelRows[y].c_str()[x]) {

          case '1':
            if (doorEntryDefined)
              throw format_error("duplicate entry door");
            doorEntryDefined = true;
            doorEntryX = x;
            doorEntryY = 2*y+2;
            doorEntryState = 0;

            break;

          case '2':
            if (doorExitDefined)
              throw format_error("duplicate exit door");
            doorExitDefined = true;
            doorExitX = x;
            doorExitY = 2*y+2;
            doorExitState = 0;
            break;

          case ' ':
          case '^':
            if (x > 0 && levelRows[y].c_str()[x-1] == '/')
            {
              level[y*2  ][x].platform = true;
            }
            break;

          case '$':
            level[2*y][x].dominoType = DominoTypeStandard;
            level[2*y][x].dominoExtra = 0x70;
            break;

          case '\\':
            if (y <= 0 || x <= 0)
              throw format_error("platform step '\\' has an invalid position");

            level[(y-1)*2+1][x-1].platform = true;
            level[y*2  ][x-1].platform = true;
            level[y*2  ][x].platform = true;
            level[y*2+1][x].platform = true;
            break;

          case '/':
            if (y <= 0 || x+1 >= LEVELSIZE_V1_X)
              throw format_error("platform step '/' has an invalid position");

            level[(y-1)*2+1][x+1].platform = true;
            level[y*2  ][x].platform = true;
            level[y*2+1][x].platform = true;
            break;

          case 'H':
          case 'V':
            level[y*2  ][x].ladder   = true;
            level[y*2+1][x].ladder   = true;
            break;

          case '.':
            level[y*2+1][x].platform = true;
            break;

          default:
            std::string::size_type dt = dominoChars.find_first_of(levelRows[y].c_str()[x]);
            if (dt == std::string::npos)
              throw format_error("invalid domino type");
            level[2*y][x].dominoType = (DominoType)dt;

            bool ladderAbove   = y > 0
              && levelRows[y-1].c_str()[x] == 'H';
            bool ladderBelow   = y+1 < 13
              && (levelRows[y+1].c_str()[x] == 'H'
                  || levelRows[y+1].c_str()[x] == 'V'
                  || levelRows[y+1].c_str()[x] == '^');
            if (ladderBelow)
            {
              level[y*2  ][x].ladder   = true;
              level[y*2+1][x].ladder   = true;
            }
            else if (ladderAbove)
            {
              level[y*2  ][x].ladder   = true;
            }

            if (y < 12)
              level[y*2+1][x].platform = true;
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
    for (unsigned char b = 0; b < numBg; b++) {
      if (bgSections[b].size() != 13)
        throw format_error("wrong number of background rows");
      for (unsigned int y = 0; y < 13; y++) {
        std::istringstream line(bgSections[b][y]);
        for (unsigned int x = 0; x < LEVELSIZE_V1_X; x++) {
          uint16_t val;
          line >> val;
          setBg(x, 2*y, b, 2*val);
          if (y < 12)
            setBg(x, 2*y+1, b, 2*val+1);

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
  else if (givenVersion == 2)
  {
    /* Size section (format XxY) */
    {
      std::istringstream sizeStream(sections.getSingleLine("Size"));
      unsigned int size;
      sizeStream >> size;
      if (size != LEVELSIZE_V1_X)
        throw format_error("invalid size, only 20 columns supported for now");

      if (sizeStream.get() != 'x')
        throw format_error("invalid size format");

      sizeStream >> size;
      if (!sizeStream.eof() || !sizeStream)
        throw format_error("invalid time format");

      if (size != LEVELSIZE_V1_Y)
        throw format_error("invalid size, only 25 rows supported for now");
    }

    /* doors section (format entryX;entryY exitX;exitY) */
    {
      std::istringstream doorsStream(sections.getSingleLine("Doors"));

      unsigned int pos;
      doorsStream >> pos;
      if (pos >= LEVELSIZE_V1_X)
        throw format_error("invalid x-position for entry door");

      doorEntryX = pos;

      if (doorsStream.get() != ';')
        throw format_error("invalid door format");

      doorsStream >> pos;

      if (pos >= LEVELSIZE_V1_Y)
        throw format_error("invalid y-position for entry door");

      doorEntryY = pos;

      if (doorsStream.get() != ' ')
        throw format_error("invalid door format");

      doorsStream >> pos;
      if (pos >= LEVELSIZE_V1_X)
        throw format_error("invalid x-position for exit door");

      doorExitX = pos;

      if (doorsStream.get() != ';')
        throw format_error("invalid door format");

      doorsStream >> pos;

      if (pos >= LEVELSIZE_V1_Y)
        throw format_error("invalid y-position for exit door");

      doorExitY = pos;

      if (!doorsStream.eof() || !doorsStream)
        throw format_error("invalid time format");
    }

    /* Level section */
    const std::vector<std::string> & givenLevelRows =
      sections.getSingleSection("Level");
    if (givenLevelRows.size() != LEVELSIZE_V1_Y)
      throw format_error("wrong number of level rows");
    std::string levelRows[LEVELSIZE_V1_Y];
    for (unsigned int y = 0; y < LEVELSIZE_V1_Y; y++) {
      std::string::size_type len = givenLevelRows[y].size();
      if (len > 2*LEVELSIZE_V1_X)
        throw format_error("level row is too long");
      /* padding with spaces to the whole width */
      levelRows[y] = givenLevelRows[y] + std::string(2*LEVELSIZE_V1_X-len, ' ');
    }

    for (unsigned int y = 0; y < LEVELSIZE_V1_Y; y++) {
      for (unsigned int x = 0; x < LEVELSIZE_V1_X; x++) {
        switch (levelRows[y][2*x])
        {
          case ' ': break;
          case 'H': level[y][x].ladder = true; break;
          case '-': level[y][x].platform = true; break;
          case 'B': level[y][x].platform = level[y][x].ladder = true; break;
          default: throw format_error("invalid platform element");
        }

        if (levelRows[y][2*x+1] != ' ')
        {
          std::string::size_type dt = dominoChars.find_first_of(levelRows[y].c_str()[2*x+1]);

          if (dt == std::string::npos)
            throw format_error("invalid domino type");

          level[y][x].dominoType = (DominoType)dt;
        }
      }
    }

    /* Background sections */
    const std::vector<std::vector<std::string> > & bgSections =
      sections.getMultiSection("Background");
    numBg = bgSections.size();
    if (numBg == 0)
      throw format_error("missing Background section");
    for (unsigned char b = 0; b < numBg; b++) {
      if (bgSections[b].size() != LEVELSIZE_V1_Y)
        throw format_error("wrong number of background rows");
      for (unsigned int y = 0; y < LEVELSIZE_V1_Y; y++) {
        std::istringstream line(bgSections[b][y]);
        for (unsigned int x = 0; x < LEVELSIZE_V1_X; x++)
        {
          uint16_t val;
          line >> val;
          setBg(x, y, b, val);

          if (!line)
            throw format_error("not enough background tiles in a row");
        }
        if (!line.eof())
          throw format_error("too many background tiles in a row");
      }
    }

    /* Calculate checksum */
    {
      // checksums are calculates over the level section and the doors section and the time

      SHA1 sha1;
      for (unsigned int y = 0; y < LEVELSIZE_V1_Y; y++) {
        sha1.update(levelRows[y]);
      }
      {
        std::ostringstream timeLeftStream;
        timeLeftStream << timeLeft;
        sha1.update(timeLeftStream.str());
      }
      {
        std::ostringstream doorsStream;
        doorsStream << doorEntryX << doorEntryY << doorExitX << doorExitY;
        sha1.update(doorsStream.str());
      }

      sha1.update(userString);
      checksum = sha1.final();
    }
    {
      SHA1 sha1;
      for (unsigned int y = 0; y < LEVELSIZE_V1_Y; y++) {
        sha1.update(levelRows[y]);
      }
      {
        std::ostringstream doorsStream;
        doorsStream << doorEntryX << doorEntryY << doorExitX << doorExitY;
        sha1.update(doorsStream.str());
      }
      sha1.update(userString);
      checksumNoTime = sha1.final();
    }
  }
  else
  {
    throw format_error("level version not supported");
  }

}

void levelData_c::save(std::ostream & stream) const
{
#ifdef DEBUG
  if (loadVersion == 1)
  {
    // we try to save in version 1 format

    std::ostringstream ss;
    if (save_v1(ss))
    {
      stream << ss.str();
      printf("successfully saved\n");
    }

    return;
  }
#endif

  unsigned int timeSeconds = timeLeft/18;

  std::string authors;

  for (size_t i = 0; i < author.size(); i++)
    authors += "| " + author[i] + "\n";

  stream <<
  textsections_c::firstLine << "\n"
  "\n"
  "Version\n"
  "| " << version << "\n"
  "\n"
  "Name\n"
  "| " << name << "\n"
  "\n"
  "Author\n"
  << authors <<
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
  "| " << hint << "\n"
  "\n"
  "Size\n"
  "| " << levelX() << "x" << levelY() << "\n"
  "\n"
  "Doors\n"
  "| " << (int)doorEntryX << ";" << (int)doorEntryY << " " << (int)doorExitX << ";" << (int)doorExitY << "\n";

  stream << '\n';
  stream << "Level\n";
  stream << '+' << std::string(2*levelX()+1, '-') << '\n';
  for (unsigned int y = 0; y < levelY(); y++)
  {
    std::string line = "| ";
    for (unsigned int x = 0; x < levelX(); x++)
    {
      if (getLadder(x, y))
      {
        if (getPlatform(x, y))
          line += 'B';
        else
          line += 'H';
      }
      else
      {
        if (getPlatform(x, y))
          line += '-';
        else
          line += ' ';
      }

      unsigned char dt = getDominoType(x, y);
      if (dt == DominoTypeEmpty)
        line += ' ';
      else if (dt < dominoChars.size())
        line += dominoChars[dt];
      else
        throw format_error("level contains a domino type which can't be saved");
    }
    stream << line.substr(0, line.find_last_not_of(' ')+1) << '\n';
  }
  stream << '+' << std::string(2*levelX()+1, '-') << '\n';

  for (unsigned char b = 0; b < numBg; b++) {
    stream << '\n';
    stream << "Background\n";
    stream << '+' << std::string((3+1)*20, '-') << '\n';
    for (unsigned int y = 0; y < levelY(); y++) {
      stream << '|';
      for (unsigned int x = 0; x < levelX(); x++)
        stream << ' ' << std::setfill(' ') << std::setw(3) << getBg(x, y, b);
      stream << '\n';
    }
    stream << '+' << std::string((3+1)*levelX(), '-') << '\n';
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

  if (name == other.name &&
      theme == other.theme &&
      hint == other.hint &&
      timeLeft == other.timeLeft &&
      numBg == other.numBg &&
      doorEntryX == other.doorEntryX &&
      doorEntryY == other.doorEntryY &&
      doorExitX == other.doorExitX &&
      doorExitY == other.doorExitY)
  {
    if (level.size() != other.level.size())
      return false;

    for (size_t y = 0; y < level.size(); y++)
    {
      if (level[y].size() != other.level[y].size())
        return false;

      for (size_t x = 0; x < level[y].size(); x++)
      {
        if (level[y][x].dominoType    != other.level[y][x].dominoType) return false;
        if (level[y][x].dominoState   != other.level[y][x].dominoState) return false;
        if (level[y][x].dominoDir     != other.level[y][x].dominoDir) return false;
        if (level[y][x].dominoYOffset != other.level[y][x].dominoYOffset) return false;
        if (level[y][x].dominoExtra   != other.level[y][x].dominoExtra) return false;

        if (level[y][x].platform != other.level[y][x].platform) return false;
        if (level[y][x].ladder   != other.level[y][x].ladder) return false;

        for (size_t bgi = 0; bgi < numBg; bgi++)
          if (level[y][x].bg[bgi] != other.level[y][x].bg[bgi]) return false;
      }
    }

    return true;
  }
  else
  {
    return false;
  }
}

void levelData_c::print(void) const {

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
      printf("%2x", level[y][x].dominoState);
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

unsigned short levelData_c::getBg(unsigned int x, unsigned int y, size_t layer) const {
  if (layer < level[y][x].bg.size())
    return level[y][x].bg[layer];
  else
    return 0;
}

bool levelData_c::getPlatform(unsigned int x, unsigned int y) const {
  return y < level.size() && x < level[y].size() && level[y][x].platform;
}

bool levelData_c::getLadder(unsigned int x, unsigned int y) const {
  return y < level.size() && x < level[y].size() && level[y][x].ladder;
}

void levelData_c::setPlatform(unsigned int x, unsigned int y, bool val)
{
  if (y < level.size() && x < level[y].size())
    level[y][x].platform = val;
}

void levelData_c::setLadder(unsigned int x, unsigned int y, bool val)
{
  if (y < level.size() && x < level[y].size())
    level[y][x].ladder = val;
}

unsigned char levelData_c::getDominoState(unsigned int x, unsigned int y) const
{
  if (y < level.size() && x < level[y].size())
    return level[y][x].dominoState;
  else
    assert(0);
}
signed char   levelData_c::getDominoDir(unsigned int x, unsigned int y) const
{
  if (y < level.size() && x < level[y].size())
    return level[y][x].dominoDir;
  else
    assert(0);
}
unsigned char levelData_c::getDominoExtra(unsigned int x, unsigned int y) const
{
  if (y < level.size() && x < level[y].size())
    return level[y][x].dominoExtra;
  else
    assert(0);
}
signed char levelData_c::getDominoYOffset(unsigned int x, unsigned int y) const
{
  if (y < level.size() && x < level[y].size())
    return level[y][x].dominoYOffset;
  else
    assert(0);
}

void levelData_c::setDominoType(unsigned int x, unsigned int y, DominoType val)
{
  if (y < level.size() && x < level[y].size())
    level[y][x].dominoType = val;
  else
    assert(0);
}
void levelData_c::setDominoState(unsigned int x, unsigned int y, int val)
{
  if (y < level.size() && x < level[y].size())
    level[y][x].dominoState = val;
  else
    assert(0);
}
void levelData_c::setDominoDir(unsigned int x, unsigned int y, int val)
{
  if (y < level.size() && x < level[y].size())
    level[y][x].dominoDir = val;
  else
    assert(0);
}
void levelData_c::setDominoExtra(unsigned int x, unsigned int y, int val)
{
  if (y < level.size() && x < level[y].size())
    level[y][x].dominoExtra = val;
  else
    assert(0);
}
void levelData_c::setDominoYOffset(unsigned int x, unsigned int y, int val)
{
  if (y < level.size() && x < level[y].size())
    level[y][x].dominoYOffset = val;
  else
    assert(0);
}

bool levelData_c::dominoLeansLeft(unsigned int x, unsigned int y) const
{
  if (getDominoType(x, y) == DominoTypeEmpty)
    return false;

  int st = getDominoState(x, y);

  if (st >= DO_ST_LEFT && st < DO_ST_UPRIGHT)
    return true;
  if (st >= DO_ST_SPLIT && st <= DO_ST_SPLIT_E)
    return true;
  if (st >= DO_ST_CRASH && st <= DO_ST_CRASH_E)
    return true;
  if (st >= DO_ST_ASCENDER && st < DO_ST_ASCENDER+7)
    return true;

  return false;
}

bool levelData_c::dominoLeansRight(unsigned int x, unsigned int y) const
{
  if (getDominoType(x, y) == DominoTypeEmpty)
    return false;

  int st = getDominoState(x, y);

  if (st > DO_ST_UPRIGHT && st <= DO_ST_RIGHT)
    return true;
  if (st >= DO_ST_SPLIT && st <= DO_ST_SPLIT_E)
    return true;
  if (st >= DO_ST_CRASH && st <= DO_ST_CRASH_E)
    return true;
  if (st > DO_ST_ASCENDER+7 && st <= DO_ST_ASCENDER_E)
    return true;

  return false;
}

bool levelContainsDomino(const levelData_c & l, DominoType d)
{
  for (size_t y = 0; y < l.levelY(); y++)
    for (size_t x = 0; x < l.levelX(); x++)
      if (l.getDominoType(x, y) == d) {
        return true;
      }
  return false;
}

bool levelForegroundEmpty(const levelData_c & l, int16_t x, int16_t y, uint16_t w, uint16_t h)
{
  for (size_t yp = 0; yp < h; yp++)
    for (size_t xp = 0; xp < w; xp++)
    {
      if (l.getPlatform(x+xp, y+yp) || l.getLadder(x+xp, y+yp) || l.getDominoType(x+xp, y+yp) != DominoTypeEmpty)
        return false;
      if (x+xp == l.getExitX() && y+yp == l.getExitY())
        return false;
      if (x+xp == l.getEntryX() && y+yp == l.getEntryY())
        return false;
      if (x+xp == l.getExitX() && y+yp+1 == l.getExitY())
        return false;
      if (x+xp == l.getEntryX() && y+yp+1 == l.getEntryY())
        return false;
    }
  return true;
}

void levelData_c::setBg(unsigned int x, unsigned int y, size_t layer, uint16_t tile)
{
  if (x < levelX() && y < levelY())
  {

    if (tile != 0 && tile != 1)
    {
      if (level[y][x].bg.size() <= layer)
      {
        level[y][x].bg.resize(layer+1);
        if (level[y][x].bg.size() > numBg)
        {
          numBg = level[y][x].bg.size();
          printf("num layer = %i\n", numBg);
        }
      }
      level[y][x].bg[layer] = tile;
    }
    else
    {
      if (layer < level[y][x].bg.size())
        level[y][x].bg[layer] = tile;
    }
  }
}

