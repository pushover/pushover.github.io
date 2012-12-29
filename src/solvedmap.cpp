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

#include "solvedmap.h"
#include "tools.h"

#include <fstream>
#include <iostream>
#include <sstream>

solvedMap_c::solvedMap_c(void) {

  std::ifstream in((getHome()+"solved.txt").c_str());
  std::string line;

  // add default user
  users.push_back(userData_c());

  while (in.good()) {
    getline(in, line);
    if (line != "")
    {
      if (line[0] == 'U' and line[1] == ' ')
      {
        users.push_back(userData_c(line.substr(2)));
      }
      else
      {
        map.insert(line);
      }
    }
  }

  currentUser = 0;
}

solvedMap_c::~solvedMap_c(void) {  }

void solvedMap_c::save(void)
{
  std::ofstream out((getHome()+"solved.txt").c_str());

  // save the users first, don't save the default user
  for (size_t i = 1; i <users.size(); i++)
    out << "U " << users[i].getUserString() << std::endl;

  for (std::set<std::string>::iterator i = map.begin(); i != map.end(); i++)
    out << *i << std::endl;
}

void solvedMap_c::addLevel(const std::string & hash) {

  map.insert(hash);

  save();
}

void solvedMap_c::addUser(const std::string & name)
{
  users.push_back(userData_c(name, getTime()));

  currentUser = users.size()-1;

  save();
}

void solvedMap_c::deleteUser(size_t user)
{
  if (user < users.size())
  {
    users.erase(users.begin()+user);
  }

  save();
}


userData_c::userData_c(const std::string & n, uint64_t t)
{
  name = n;

  std::ostringstream str;

  str << n << '\x9' << t;

  userString = str.str();
}

// load a user from given string
userData_c::userData_c(const std::string & n)
{
  userString = n;

  name = n.substr(0, n.find('\x9'));
}
