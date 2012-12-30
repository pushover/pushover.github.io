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
    void toggleSound(void) { playSoundSwitch = !playSoundSwitch; }

    void playMusic(const std::string & fname);

    void toggleMusic(void) {
      playMusicSwitch = !playMusicSwitch;
      playMusic("");
    }

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

    bool playSoundSwitch;  // used to toggle sound effects independently of sound
    bool playMusicSwitch; // toggle music on and off

    std::vector<struct soundDat> sounds;

    static class soundSystem_c *inst;

    Mix_Music * music;

};

#endif
