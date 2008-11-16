#ifndef __SOUNDSYS_H__
#define __SOUNDSYS_H__

#include <SDL_mixer.h>

#include <vector>
#include <string>

class soundSystem_c {

  public:

    enum {
      SE_STANDARD,
      SE_STOPPER,
      SE_SPLITTER,
      SE_EXPLODER,
      SE_DELAY,
      SE_TUMBLER,
      SE_BRIDGER,
      SE_VANISH,
      SE_TRIGGER,
      SE_ASCENDER,
      SE_ANT_FALLING,
      SE_ANT_LANDING,
      SE_PICK_UP_DOMINO,
      SE_NU_WHAT,
      SE_SHRUGGING,
      SE_DOOR_CLOSE,
      SE_DOOR_OPEN,
      SE_VICTORY,
    };

    ~soundSystem_c(void);

    // play sounds, please use the enum above
    void startSound(unsigned int snd);

    /* tries to open and initialize the sound device */
    void openSound(const std::string & base);
    /* closes the sound device */
    void closeSound(void);

    /* singleton function, use this function to access the one and only
     * instance of this class
     */
    static soundSystem_c * instance(void);

    /* toggle quiet sound effects */
    void toggleOnOff(void) { quiet = !quiet; }

  private:

    struct soundDat {
      int id_num;  //unique ID # of this sound
      int volume;  //sound volume
      int channel;
      Mix_Chunk *sound; //sound data
    };

    void addsound(const std::string & fname, int vol);

    soundSystem_c(void);

    /* this var is only true, if we the user wants sound, and wa
     * can init it
     */
    bool useSound;

    bool quiet;  // used to toggle sound effects indepentently of sound

    std::vector<struct soundDat> sounds;

    static class soundSystem_c *inst;

};

#endif
