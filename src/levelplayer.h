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

#ifndef __LEVEL_PLAYER_H__
#define __LEVEL_PLAYER_H__

#include "leveldisplay.h"

class ant_c;

class levelPlayer_c : public levelDisplay_c {

  private:
    // has the level been checked for completion, that is
    // only done once, once a trigger has fallen
    bool finishCheckDone;

    // requested states for the 2 doors
    bool openDoorExit;
    bool openDoorEntry;

    // returns true, if there are still counter stoppers
    // of num or bigger (num = DominoTypeCounter1, 2 or 3) that are not falling
    bool CounterStopper(int num);

    // calls the different states of the dominos
    void callStateFunction(int type, int state, int x, int y);

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
    void DTA_P(int x, int y);
    void DTA_8(int x, int y);
    void DTA_9(int x, int y);
    void DTA_N(int x, int y);
    void DTA_A(int x, int y);
    void DTA_H(int x, int y);
    void DTA_O(int x, int y);
    void DTA_B(int x, int y);

    // a counter used to see, if nothing happens within the level
    int inactive;

  public:

    levelPlayer_c(surface_c & screen, graphics_c & gr) : levelDisplay_c(screen, gr), inactive(0) {}

    void load(const textsections_c & sections, const std::string & userString);

    /* opens and closes doors */
    void performDoors(void);
    // allows you to open and close the 2 doors
    void openEntryDoor(bool open) { openDoorEntry = open; }
    void openExitDoor(bool open) { openDoorExit = open; }

    int performDominos(ant_c & ant);

    int pickUpDomino(int x, int y);  // removes the domino from that position and returns the domino type
    void putDownDomino(int x, int y, int domino, bool pushin);
    void fallingDomino(int x, int y);

    bool pushDomino(int x, int y, int dir);

    bool levelInactive(void) { return inactive > 3; }
};

#endif

