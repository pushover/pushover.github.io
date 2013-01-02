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

#include "levelset.h"
#include "leveldata.h"
#include "textsections.h"
#include "screen.h"
#include "tools.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zlib.h>

static std::string readCompressedFile(const std::string & path) {

  gzFile file = ::gzopen(path.c_str(), "rb");
  if (file == NULL)
    throw format_error("unable to open file: " + path);
  std::string result;
  char buf[1024];
  int len;
  while ((len = ::gzread(file, buf, sizeof(buf))) > 0)
    result.append(buf, len);
  if (len != 0 || !::gzeof(file) || ::gzclose(file) != Z_OK)
    throw format_error("error when decompressing file: " + path);
  return result;
}

levelset_c::levelset_c(const std::string & path, const std::string & userString) {

  /* Load all files */
  struct stat st;
  if (stat(path.c_str(), &st) != 0)
    throw format_error("file or directory does not exist: " + path);
  std::vector<textsections_c> fileSections;
  if (S_ISDIR(st.st_mode)) {
    const std::vector<std::string> entries = directoryEntries(path);
    for (std::vector<std::string>::const_iterator i = entries.begin(); i != entries.end(); i++) {
      const std::string & filename = *i;
      if (filename.size() <= 0 || filename[0] == '.')
        continue;
      std::ifstream file((path + '/' + filename).c_str());
      fileSections.push_back(textsections_c(file, true));
    }
  } else {
    std::istringstream stream(readCompressedFile(path));
    while (stream)
      fileSections.push_back(textsections_c(stream, false));
    if (!stream.eof())
      throw format_error("unexpected string stream error while loading file: " + path);
  }

  /* Parse all loaded files */
  levelData_c test_level;
  bool index_loaded = false;
  for (std::vector<textsections_c>::const_iterator i = fileSections.begin(); i != fileSections.end(); i++) {
    const textsections_c & sections = *i;
    try {

      /* Load level */
      test_level.load(sections, userString);

    } catch (format_error & level_e) {

      /* Load levelset index */
      if (index_loaded)
        throw;
      try {

        /* Version section */
        {
          std::istringstream versionStream(sections.getSingleLine("Version"));
          unsigned int givenVersion;
          versionStream >> givenVersion;
          if (!versionStream.eof() || !versionStream)
            throw format_error("invalid levelset index version");
        }

        /* Name section */
        name = sections.getSingleLine("Name");

        /* Priority section */
        {
          std::istringstream priorityStream(sections.getSingleLine("Priority"));
          priorityStream >> priority;
          if (!priorityStream.eof() || !priorityStream)
            throw format_error("invalid levelset priority number");
        }

        /* Levels section */
        levelNames = sections.getSingleSection("Levels");
        if (levelNames.empty())
          throw format_error("missing levels");

        index_loaded = true;

      } catch (format_error & index_e) {
        throw format_error("file is"
                           " neither a level (" + level_e.msg + ")"
                           " nor a levelset index (" + index_e.msg + ")");
      }
      continue;
    }
    const std::string & levelName = test_level.getName();
    if (!levels.insert(make_pair(levelName, sections)).second)
      throw format_error("duplicate level name: " + levelName);
    checksums.insert(make_pair(levelName, test_level.getChecksum()));
    checksumsNoTime.insert(make_pair(levelName, test_level.getChecksumNoTime()));
  }

  /* Check for consistency and completeness */
  if (!index_loaded)
    throw format_error("missing levelset index");
  std::map<std::string, bool> levelNamesMap;
  for (std::vector<std::string>::const_iterator i = levelNames.begin(); i != levelNames.end(); i++) {
    const std::string & levelName = *i;
    if (levels.find(levelName) == levels.end())
      throw format_error("missing level: " + levelName);
    if (!levelNamesMap.insert(make_pair(levelName, true)).second)
      throw format_error("duplicate entry in levelset index for level: " + levelName);
  }
  for (std::map<std::string, textsections_c>::const_iterator i = levels.begin(); i != levels.end(); i++) {
    const std::string & levelName = i->first;
    if (levelNamesMap.find(levelName) == levelNamesMap.end())
      throw format_error("missing entry in levelset index for level: " + levelName);
  }
}

const std::string & levelset_c::getChecksum(const std::string & levelName) const {
  if (checksums.find(levelName) == checksums.end())
    throw std::exception();
  return checksums.find(levelName)->second;
}

const std::string & levelset_c::getChecksumNoTime(const std::string & levelName) const {
  if (checksumsNoTime.find(levelName) == checksumsNoTime.end())
    throw std::exception();
  return checksumsNoTime.find(levelName)->second;
}

void levelset_c::loadLevel(levelData_c & level, const std::string & levelName, const std::string & userString) const {

  const std::map<std::string, textsections_c>::const_iterator i = levels.find(levelName);
  if (i == levels.end())
    throw format_error("unknown level name: " + levelName);
  level.load(i->second, userString);
}

void levelsetList_c::load(const std::string & path, const std::string & userString) {

  /* Load and add levelsets */
  const std::vector<std::string> entries = directoryEntries(path);
  for (std::vector<std::string>::const_iterator i = entries.begin(); i != entries.end(); i++) {

    const std::string & entryname = *i;
    if (entryname.size() <= 0 || entryname[0] == '.')
      continue;

    levelset_c levelset(path + '/' + entryname, userString);
    const std::string & levelsetName = levelset.getName();
    if (!levelsets.insert(make_pair(levelsetName, levelset)).second)
      throw format_error("duplicate levelset name: " + levelsetName);
    sortHelper.push_back(make_pair(levelset.getPriority(), levelsetName));
  }

  /* Re-sort all levelsets by priority and name */
  std::sort(sortHelper.begin(), sortHelper.end());
  levelsetNames.clear();
  for (std::vector<std::pair<unsigned int, std::string> >::const_iterator i = sortHelper.begin(); i != sortHelper.end(); i++)
    levelsetNames.push_back(i->second);
}

const levelset_c & levelsetList_c::getLevelset(const std::string & levelsetName) const {

  const std::map<std::string, levelset_c>::const_iterator i = levelsets.find(levelsetName);
  if (i == levelsets.end())
    throw format_error("unknown levelset name: " + levelsetName);
  return i->second;
}
