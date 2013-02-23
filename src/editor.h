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

#ifndef __EDIT_H__
#define __EDIT_H__

#include <SDL.h>

#include <string>

class graphicsN_c;
class screen_c;
class levelPlayer_c;

void leaveEditor(void);
void startEditor(graphicsN_c & g, screen_c & s, levelPlayer_c & lp, const std::string & user);

// return true, when the level editor wants to leave
bool eventEditor(const SDL_Event & event);

void stepEditor(void);

#endif
