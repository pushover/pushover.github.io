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

#ifndef __SOLVED_MAP_H__
#define __SOLVED_MAP_H__

// this set contains all the checksums of all solved levels
// with this it is easy to find out if a certain level has been
// solved by the user

#include <string>
#include <set>

class solvedMap_c {

    private:

        std::set<std::string> map;

    public:

        // try to load the map
        solvedMap_c(void);
        ~solvedMap_c(void);

        void addLevel(const std::string & hash);

        bool solved(const std::string & hash) const {
            return map.find(hash) != map.end();
        }
};

#endif

