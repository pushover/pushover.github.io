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

#include "recorder.h"

#include "tools.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

void recorder_c::load(const std::string & filename) {
  std::ifstream stream(filename.c_str());

  if (!stream)
    throw std::runtime_error("Can't open record file: " + filename);

  if (!getline(stream, levelsetName))
    throw std::runtime_error("Error in record file: " + filename);
  if (!getline(stream, levelName))
    throw std::runtime_error("Error in record file: " + filename);

  playpos = 0;

  record.clear();
  while (stream)
  {
    int cnt, val;
    stream >> cnt;
    stream >> val;
    for (int i = 0; i < cnt; i++)
      record.push_back(val);
  }
}

void recorder_c::save(const std::string & prefix) const {
  /* find first unused filename */
  std::string filename;
  for (unsigned int num = 1; ; num++) {
    std::ostringstream s;
    s << getHome() << prefix << std::setfill('0') << std::setw(5) << num << ".rec";
    filename = s.str();
    std::ifstream stream(filename.c_str());
    if (!stream)
      break;
  }

  /* save to that filename */
  std::ofstream stream(filename.c_str());
  stream << levelsetName << '\n';
  stream << levelName << '\n';

  int val = record[0];
  int cnt = 1;
  unsigned int pos = 1;

  while (pos < record.size()) {

    if (record[pos] != val) {
      stream << cnt << ' ' << val << '\n';
      val = record[pos];
      cnt = 1;
    }
    else
    {
      cnt++;
    }
    pos++;
  }
  stream << cnt << ' ' << val << '\n';
}

void recorder_c::truncate(void) {
  record.resize(playpos);
}

void recorder_c::reset(void) {
  playpos = 0;
  record.resize(playpos);
}

