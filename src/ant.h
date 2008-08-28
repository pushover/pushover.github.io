#include <SDL.h>

// this class contains all necessary code for the ant animation


class level_c;
class graphics_c;

// the following defines are used for the keymask
#define KEY_LEFT 1
#define KEY_UP 2
#define KEY_RIGHT 4
#define KEY_DOWN 8
#define KEY_ACTION 16

class ant_c {

  private:

    unsigned int state;
    unsigned int animation;
    unsigned int animationImage;
    unsigned int carriedDomino;
    unsigned int animationTimer;

    int blockX, blockY;
    int subBlock, screenBlock;

    level_c * level;
    graphics_c * gr;

    unsigned int keyMask;
    unsigned int inactiveTimer;
    unsigned int fallingHight;
    signed int direction;
    unsigned int pushDelay;
    unsigned int pushAnimation;

  public:

    ant_c(void) {}

    // init the ant state for level entering
    // the level is saved and used later on for dirty block
    // marking, and level modification
    void init(level_c * l, graphics_c * gr);

    // do one animation step for the ant
    void performAnimation(void);

    void draw(SDL_Surface * video);

    void setKeyStates(unsigned int keyMask);

  private:

    unsigned int callStateFunction(unsigned int state);
    bool animateAnt(unsigned int delay);


    unsigned int SFLeaveDoor(void);
    unsigned int SFStepAside(void);
    unsigned int SFWalkLeft(void);
    unsigned int SFWalkRight(void);
    unsigned int SFJumpUpLeft(void);
    unsigned int SFJumpUpRight(void);
    unsigned int SFJumpDownLeft(void);
    unsigned int SFJumpDownRight(void);
    unsigned int SFInFrontOfExploder(void);
    unsigned int SFInactive(void);
    unsigned int SFLazying(void);
    unsigned int SFFlailing(void);
    unsigned int SFStartFallingLeft(void);
    unsigned int SFStartFallingRight(void);
    unsigned int SFFalling(void);
    unsigned int SFLanding(void);
    unsigned int SFLadder1(void);
    unsigned int SFLadder2(void);
    unsigned int SFLadder3(void);
    unsigned int SFPullOutLeft(void);
    unsigned int SFPullOutRight(void);
    unsigned int SFPushInLeft(void);
    unsigned int SFPushInRight(void);
    unsigned int SFLeaveLadderRight(void);
    unsigned int SFLeaveLadderLeft(void);
    unsigned int SFEnterLadder(void);
    unsigned int SFLooseRight(void);
    unsigned int SFLooseLeft(void);
    unsigned int SFXXX7(void);   // TODO whats this state????
    unsigned int SFEnterDominosLeft(void);
    unsigned int SFEnterDominosRight(void);
    unsigned int SFPushLeft(void);
    unsigned int SFPushRight(void);
    unsigned int SFPushSpecialLeft(void);
    unsigned int SFPushSpecialRight(void);
    unsigned int SFXXX5(void);   // TODO whats this state????
    unsigned int SFXXX6(void);   // TODO whats this state????
    unsigned int SFGhost1(void);
    unsigned int SFGhost2(void);
    unsigned int SFEnterDoor(void);
    unsigned int SFXXX9(void);   // TODO whats this state????
    unsigned int SFNoNo(void);
    unsigned int SFVictory(void);
    unsigned int SFShrugging(void);
    unsigned int SFStruck(void);


    unsigned int SFNextAction(void);

};
