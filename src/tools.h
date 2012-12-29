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

#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <string>
#include <vector>
#include <stdint.h>

// initialize the random number generator from the current time
// (implemented in a platform dependent way)
void srandFromTime(void);

// return the path to the home directory
// (implemented in a platform dependent way)
std::string getHome(void);

// return all non-hidden directory entries
std::vector<std::string> directoryEntries(const std::string & path);

// return current system time
uint64_t getTime(void);

#endif
