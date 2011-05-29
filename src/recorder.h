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

#ifndef __RECORDER_H__
#define __RECORDER_H__

#include <vector>
#include <string>

class recorder_c {

  public:

    recorder_c(void) : playpos(0), levelsetName("unknown"), levelName("unknown") {};

    void load(const std::string & filename);
    void save(const std::string & prefix) const;

    const std::string & getLevelsetName(void) const { return levelsetName; }
    const std::string & getLevelName(void) const { return levelName; }
    void setLevel(const std::string & levelsetName, const std::string & levelName) {
        this->levelsetName = levelsetName;
        this->levelName = levelName;
    }

    void addEvent(int event) { record.push_back(event); }
    int getEvent(void) { return playpos < record.size() ? record[playpos++] : 0; }
    bool endOfRecord(void) const { return playpos >= record.size(); }

    // remove all events from the current position
    void truncate(void);

    // completely clear record
    void reset(void);

  private:

    std::vector<int> record;
    unsigned int playpos;
    std::string levelsetName;
    std::string levelName;

};

#endif
