#ifndef __LEVEL_PLAYER_H__
#define __LEVEL_PLAYER_H__

#include "leveldisplay.h"

class ant_c;

class levelPlayer_c : public levelDisplay_c {

  private:
    // has the level been chacked for completion, that is
    // only done once, once a trigger has falln
    bool finishCheckDone;

    // requested states for the 2 doors
    bool openDoorExit;
    bool openDoorEntry;

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
    void DTA_8(int x, int y);
    void DTA_9(int x, int y);
    void DTA_N(int x, int y);
    void DTA_A(int x, int y);
    void DTA_H(int x, int y);
    void DTA_O(int x, int y);
    void DTA_B(int x, int y);

    bool triggerFalln;

  public:

    levelPlayer_c(surface_c & screen, graphics_c & gr) : levelDisplay_c(screen, gr) {}

    void load(const textsections_c & sections);

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
    void removeDomino(int x, int y) { level[y][x].dominoType = levelData_c::DominoTypeEmpty; }

    // check, if the level has been successfully solved
    // if not the reason for failure is in fail
    bool levelCompleted(int & fail);

    bool triggerIsFalln(void) { return triggerFalln; }
};

#endif

