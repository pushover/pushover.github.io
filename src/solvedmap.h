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
#include <vector>

#include <stdint.h>

class userData_c {

   private:
       std::string name;
       std::string userString;  // the userstring, containing the name and creation-date

   public:

       // create new user with given timestamp
       userData_c(const std::string & n, uint64_t t);

       // load a user from given string
       userData_c(const std::string & n);

       // create default user (no name, no time, no addition to
       // level hash
       userData_c(void) : name("(default)"), userString("") {}

       // save the user into the given string
       std::string getUserString(void) const { return userString; }
       std::string getUserName(void) const { return name; }
};

class solvedMap_c {

    private:

        std::set<std::string> map;
        std::vector<userData_c> users;

        size_t currentUser;

        void save(void);

    public:

        // try to load the map
        solvedMap_c(void);
        ~solvedMap_c(void);

        void addLevel(const std::string & hash);

        bool solved(const std::string & hash) const {
            return map.find(hash) != map.end();
        }

        // adds a new user and selects it
        void addUser(const std::string & name);
        void deleteUser(size_t user);
        size_t getNumberOfUsers(void) const { return users.size(); }
        void selectUser(size_t user) { if (user < users.size()) currentUser = user; }
        size_t getCurrentUser(void) const { return currentUser; }
        const std::string getUserName(size_t user) const { return users[user].getUserName(); }
        const std::string getUserString(void) const { return users[currentUser].getUserString(); }
};

#endif

