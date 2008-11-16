/* Tower Toppler - Nebulus
 * Copyright (C) 2000-2006  Andreas Röver
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "soundsys.h"

#include <SDL.h>

#include <iostream>

soundSystem_c::soundSystem_c(void)
{
  useSound = false;
  quiet = false;
}

soundSystem_c::~soundSystem_c(void)
{
  closeSound();

  for (unsigned int t = 0; t < sounds.size(); t++)
    if (sounds[t].sound)
      Mix_FreeChunk(sounds[t].sound);

}

void soundSystem_c::addsound(const std::string & fname, int vol)
{
  struct soundDat d;

  SDL_RWops *file = SDL_RWFromFile(fname.c_str(), "rb");
  d.sound = Mix_LoadWAV_RW(file, 1);

  if (d.sound) {

    d.channel = -1;
    d.volume = vol;

    sounds.push_back(d);
  }
  else
  {
    std::cout << "can not load sound from file " << fname << std::endl;
  }
}

void soundSystem_c::startSound(unsigned int snd)
{
  if (!useSound) return;
  if (quiet) return;

  if (snd >= 0 && snd < sounds.size())
  {
    sounds[snd].channel = Mix_PlayChannel(-1, sounds[snd].sound, 0);
    Mix_Volume(sounds[snd].channel, sounds[snd].volume);
  }
}

soundSystem_c * soundSystem_c::instance(void) {

  if (!inst)
    inst = new soundSystem_c();

  return inst;
}

class soundSystem_c *soundSystem_c::inst = 0;

void soundSystem_c::openSound(const std::string & base) {

  if(SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
    return;
  }

  if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    return;
  }

  if (sounds.size() == 0)
  {

    addsound(base+"/data/01_StandardFalling.ogg", MIX_MAX_VOLUME);     // SE_STANDARD,
    addsound(base+"/data/02_StopperHit.ogg", MIX_MAX_VOLUME);          // SE_STOPPER,
    addsound(base+"/data/03_Splitter.ogg", MIX_MAX_VOLUME);            // SE_SPLITTER,
    addsound(base+"/data/04_Exploder.ogg", MIX_MAX_VOLUME);            // SE_EXPLODER,
    addsound(base+"/data/05_Delay.ogg", MIX_MAX_VOLUME);               // SE_DELAY,
    addsound(base+"/data/06_TumblerFalling.ogg", MIX_MAX_VOLUME);      // SE_TUMBLER,
    addsound(base+"/data/07_BridgerFalling.ogg", MIX_MAX_VOLUME);      // SE_BRIDGER,
    addsound(base+"/data/06_TumblerFalling.ogg", MIX_MAX_VOLUME);      // SE_VANISH,
    addsound(base+"/data/09_TriggerFalling.ogg", MIX_MAX_VOLUME);      // SE_TRIGGER,
    addsound(base+"/data/0A_Ascender.ogg", MIX_MAX_VOLUME);            // SE_ASCENDER,
    addsound(base+"/data/0B_Falling.ogg", MIX_MAX_VOLUME);             // SE_ANT_FALLING,
    addsound(base+"/data/0C_Landing.ogg", MIX_MAX_VOLUME);             // SE_ANT_LANDING,
    addsound(base+"/data/0D_PickUpDomino.ogg", MIX_MAX_VOLUME);        // SE_PICK_UP_DOMINO,
    addsound(base+"/data/0E_Tapping.ogg", MIX_MAX_VOLUME);             // SE_NU_WHAT,
    addsound(base+"/data/0F_Schrugging.ogg", MIX_MAX_VOLUME);          // SE_SHRUGGING,
    addsound(base+"/data/10_DoorClose.ogg", MIX_MAX_VOLUME);           // SE_DOOR_CLOSE,
    addsound(base+"/data/11_DoorOpen.ogg", MIX_MAX_VOLUME);            // SE_DOOR_OPEN,
    addsound(base+"/data/13_Victory.ogg", MIX_MAX_VOLUME);             // SE_VICTORY,
  }

  useSound = true;
}

void soundSystem_c::closeSound(void) {

  if (!useSound) return;
  useSound = false;

  while (Mix_Playing(-1)) SDL_Delay(100);

  Mix_CloseAudio();
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

