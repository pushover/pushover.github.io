/* Pushover
 *
 * Pushover is the legal property of its developers, whose
 * names are listed in the AUTHORS file, which is included
 * within the source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA
 */

#include "check.h"

#include "levelset.h"
#include "ant.h"
#include "recorder.h"
#include "tools.h"

#include <stdexcept>
#include <set>
#include <string>

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


static void check_record(const std::string & rec_path, levelsetList_c & levelsetList,
                         unsigned int & count, unsigned int & failed,
                         std::set<std::pair<std::string, std::string> > & uncheckedLevels,
                         std::string checker(const ant_c & a, const levelPlayer_c & l)) {
  recorder_c rec;
  rec.load(rec_path);

  uncheckedLevels.erase(make_pair(rec.getLevelsetName(), rec.getLevelName()));

  levelPlayer_c l;
  levelsetList.getLevelset(rec.getLevelsetName()).loadLevel(l, rec.getLevelName(), "");
  ant_c a(l);

  while (!rec.endOfRecord()) {
    a.performAnimation(rec.getEvent());
  }

  // add a few more iterations at the end to make sure the ant has left the level
  for (unsigned int j = 0; j < 100; j++)
  {
    a.performAnimation(0);
  }

  const std::string error = checker(a, l);
  if (!error.empty()) {
    std::cout << rec_path << " (" << rec.getLevelsetName() << " / " << rec.getLevelName() << "): " << error << std::endl;
    failed++;
  }
  count++;
}

static bool check(const std::string &levelsdir, int argn, char * argv[], std::string checker(const ant_c & a, const levelPlayer_c & l), bool allowUncheckedLevels) {
  levelsetList_c levelsetList;
  levelsetList.load(levelsdir, "");

  unsigned int count = 0;
  unsigned int failed = 0;

  std::set<std::pair<std::string, std::string> > uncheckedLevels;
  for (unsigned int i = 0; i < levelsetList.getLevelsetNames().size(); i++) {
    const levelset_c & levelset = levelsetList.getLevelset(levelsetList.getLevelsetNames()[i]);
    for (unsigned int j = 0; j < levelset.getLevelNames().size(); j++) {
        uncheckedLevels.insert(make_pair(levelset.getName(), levelset.getLevelNames()[j]));
    }
  }

  for (int i = 0; i < argn; i++)
  {
    const std::string path(argv[i]);
    struct stat st;
    if (stat(path.c_str(), &st) != 0)
      throw std::runtime_error("file or directory does not exist: " + path);
    if (S_ISDIR(st.st_mode))
    {
      const std::vector<std::string> entries = directoryEntries(path);
      for (std::vector<std::string>::const_iterator j = entries.begin(); j != entries.end(); j++) {
        const std::string & filename = *j;
        if (filename.size() > 0 && filename[0] != '.')
          check_record(path + '/' + filename, levelsetList, count, failed, uncheckedLevels, checker);
      }
    }
    else
    {
      check_record(path, levelsetList, count, failed, uncheckedLevels, checker);
    }
  }

  if (!allowUncheckedLevels) {
    for (std::set<std::pair<std::string, std::string> >::const_iterator i = uncheckedLevels.begin(); i != uncheckedLevels.end(); i++) {
      std::cout << "Missing recording for level: " << i->first << " / " << i->second << "\n";
    }
  }

  std::cout << failed << " out of " << count << " tests failed, " << uncheckedLevels.size() << " levels unchecked\n";
  return failed == 0 && (allowUncheckedLevels || uncheckedLevels.empty());
}

static std::string checkerFinish(const ant_c & a, const levelPlayer_c & l)
{
    if (a.isVisible())
      return "Ant still visible";


    if (!l.dominoesFalln())
      return "Not all dominoes falln";

    if (l.rubblePile())
      return "Crashes happened";

    return "";
}

static std::string checkerFail(const ant_c & a, const levelPlayer_c & l) {

    if (a.isVisible() == true || !l.dominoesFalln() || l.rubblePile())
      return "";
    else
      return "Level not Failed";
}

static std::string checkerCrash(const ant_c & /*a*/, const levelPlayer_c & l) {
  // we succeeded, when the ant has vanished, then it went out of the door

  if (l.rubblePile())
    return "";
  else
    return "Crashes didn't happen";
}


void check(const std::string &levelsdir, int argc, char * argv[]) {

  // filter out the no graphic cases, they are special and will be treated
  // separately
  if (argc >= 3 && strcmp(argv[1], "-c") == 0)   // the "must finish" tests
  {
    exit(check(levelsdir, argc-2, argv+2, checkerFinish, false) ? 0 : 1);
  }

  if (argc >= 3 && strcmp(argv[1], "-y") == 0)   // the "must fail" tests
  {
    exit(check(levelsdir, argc-2, argv+2, checkerFail, true) ? 0 : 1);
  }

  if (argc >= 3 && strcmp(argv[1], "-x") == 0)   // the "must crash" tests
  {
    exit(check(levelsdir, argc-2, argv+2, checkerCrash, true) ? 0 : 1);
  }
}
