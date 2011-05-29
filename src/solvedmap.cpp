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

solvedMap_c::solvedMap_c(void) {

  std::ifstream in((getHome()+"solved.txt").c_str());
  std::string line;

  while (in.good()) {
    getline(in, line);
    if (line != "")
      map.insert(line);
  }
}

solvedMap_c::~solvedMap_c(void) {  }

void solvedMap_c::addLevel(const std::string & hash) {

  map.insert(hash);

  std::ofstream out((getHome()+"solved.txt").c_str());

  for (std::set<std::string>::iterator i = map.begin(); i != map.end(); i++)
    out << *i << std::endl;
}

