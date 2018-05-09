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


#ifdef DEBUG

#include "leveldata.h"

#include "textsections.h"


#include <iomanip>
#include <stdio.h>


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
  FgElementDoor3,               // used in door animation, but not in level

  FgElementInvalid
};



bool levelData_c::save_v1(std::ostream & stream) const
{
  if (levelX() != 20 || levelY() != 25)
  {
    printf("unsuppored level size\n");
    return false;
  }

  std::string authors;

  for (size_t i = 0; i < author.size(); i++)
    authors += "| " + author[i] + "\n";

  unsigned int timeSeconds = timeLeft/18;
  stream <<
  textsections_c::firstLine << "\n"
  "\n"
  "Version\n"
  "| 1\n"
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
  "| " << hint << "\n";

  stream << '\n';
  stream << "Level\n";
  stream << '+' << std::string(20+1, '-') << '\n';
  for (unsigned int y = 0; y < 13; y++) {
    std::string line = "| ";
    for (unsigned int x = 0; x < 20; x++) {
      if (x == doorEntryX && 2*y+2 == doorEntryY)
        line += '1';
      else if (x == doorExitX && 2*y+2 == doorExitY)
        line += '2';
      else switch (getFg(x, y)) {
        case FgElementInvalid:
          printf("invalid levelconstruction at position %i %i\n", x, y);
          return false;
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
          else if (y < levelY() && getDominoType(x, 2*y) != DominoTypeEmpty)
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
          unsigned char dt = getDominoType(x, 2*y);
          if (y == 12)
          {
            dt = DominoTypeStandard;
          }

          if (dt == DominoTypeEmpty
              && getFg(x, y) == FgElementPlatformStrip)
            line += '.';
          else if (dt < dominoChars.size())
            line += dominoChars[dt];
          else
          {
            printf("level contains a domino type which can't be saved\n");
            return false;
          }

      }
    }
    stream << line.substr(0, line.find_last_not_of(' ')+1) << '\n';
  }
  stream << '+' << std::string(20+1, '-') << '\n';

  for (unsigned char b = 0; b < numBg; b++) {
    // save only non-empty layers
    bool empty = true;
    for (unsigned int y = 0; y < 25; y++)
      for (unsigned int x = 0; x < 20; x++)
        if (getBg(x, y, b) != 0 && getBg(x, y, b) != 1)
          empty = false;

    if (!empty)
    {
      stream << '\n';
      stream << "Background\n";
      stream << '+' << std::string((3+1)*20, '-') << '\n';

      for (unsigned int y = 0; y < 13; y++) {
        stream << '|';
        for (unsigned int x = 0; x < 20; x++)
        {
          if ((y < 12 && getBg(x, 2*y, b)+1 != getBg(x, 2*y+1, b) && getBg(x, 2*y+1, b) != 0) || (getBg(x, 2*y, b) % 2 != 0))
          {
            printf("invalid background tile at position %x %x in layer %x\n", x, y, b);
            return false;
          }
          stream << ' ' << std::setfill(' ') << std::setw(3) << getBg(x, 2*y, b)/2;
        }
        stream << '\n';
      }
      stream << '+' << std::string((3+1)*20, '-') << '\n';
    }
  }

  return true;
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

    if (2*y+0 < levelY())
    {
      if (x > 0  && level[2*y+0][x-1].platform) platformBits |= 0x100;
      if (          level[2*y+0][x+0].platform) platformBits |= 0x080;
      if (x < 19 && level[2*y+0][x+1].platform) platformBits |= 0x040;
    }

    if (2*y+1 < levelY())
    {
      if (x > 0  && level[2*y+1][x-1].platform) platformBits |= 0x020;
      if (          level[2*y+1][x+0].platform) platformBits |= 0x010;
      if (x < 19 && level[2*y+1][x+1].platform) platformBits |= 0x008;
    }

    if (2*y+2 < levelY())
    {
      if (x > 0  && level[2*y+2][x-1].platform) platformBits |= 0x004;
      if (          level[2*y+2][x+0].platform) platformBits |= 0x002;
      if (x < 19 && level[2*y+2][x+1].platform) platformBits |= 0x001;
    }

    uint16_t ladderBits = 0;
    if (2*y+0 < levelY() && level[2*y+0][x].ladder) ladderBits |= 0x02;
    if (2*y+1 < levelY() && level[2*y+1][x].ladder) ladderBits |= 0x01;

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
                        return FgElementInvalid;
            }
            break;

        case 0x010:
            if (ladderBits != 0)
              return FgElementInvalid;
            else
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
                        return FgElementInvalid;
            }
            break;

        case 0x018:
        case 0x019:
        case 0x058:
        case 0x118:
            return  FgElementPlatformStart;

        case 0x030:
        case 0x034:
        case 0x130:
            return FgElementPlatformEnd;

        case 0x133: //
        case 0x033: //
            if (ladderBits != 0)
              return FgElementInvalid;
            else
              return FgElementPlatformStep1; // 7

        case 0x026: //
            if (ladderBits != 0)
              return FgElementInvalid;
            else
              return FgElementPlatformStep2; // 8

        case 0x0C8: //
            if (ladderBits != 0)
              return FgElementInvalid;
            else
              return FgElementPlatformStep3; // 9

        case 0x199: //
        case 0x198: //
            if (ladderBits != 0)
              return FgElementInvalid;
            else
              return FgElementPlatformStep4; // 10

        case 0x00b: //
            if (ladderBits != 0)
              return FgElementInvalid;
            else
              return FgElementPlatformStep5; // 11

        case 0x01E: //
        case 0x05E: //
            if (ladderBits != 0)
              return FgElementInvalid;
            else
              return FgElementPlatformStep6; // 12

        case 0x0F0: //
        case 0x0F4: //
            if (ladderBits != 0)
              return FgElementInvalid;
            else
              return FgElementPlatformStep7; // 13

        case 0x1A0: //
        case 0x1A8:
        case 0x080:
            if (ladderBits != 0)
              return FgElementInvalid;
            else
              return FgElementPlatformStep8; // 14

        default:
            printf("unknown platform bitmask pos %i %i  =  %x\n", x, y, platformBits);
            return FgElementInvalid;
    }

    return FgElementInvalid;
}


#endif
