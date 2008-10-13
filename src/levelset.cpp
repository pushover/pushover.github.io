#include "levelset.h"
#include "level.h"
#include "textsections.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <dirent.h>

class directory_c {

  private:

    DIR * dir;

  public:

    directory_c(const std::string & path): dir(::opendir(path.c_str())) {
      if (dir == NULL)
        throw format_error("unable to open directory: " + path);
    }

    ~directory_c() {
      if (::closedir(dir) != 0)
        throw format_error("unable to close directory");
    }

    struct dirent * readdir(void) {
      return ::readdir(dir);
    }
};

levelset_c::levelset_c(const std::string & path) {

  directory_c dir(path);
  level_c test_level;
  bool index_loaded = false;
  for (struct dirent * i = dir.readdir(); i != NULL; i = dir.readdir()) {

    const std::string filename(i->d_name);
    if (filename.size() <= 0 || filename[0] == '.')
      continue;

    std::ifstream file((path + '/' + filename).c_str());
    const textsections_c sections(file, true);
    try {

      /* Load level */
      test_level.load(sections);

    } catch (format_error & level_e) {

      /* Load levelset index */
      if (index_loaded)
        throw;
      try {

        /* Version section */
        std::istringstream versionStream(sections.getSingleLine("Version"));
        unsigned int givenVersion;
        versionStream >> givenVersion;
        if (!versionStream.eof() || !versionStream)
          throw format_error("invalid levelset index version");

        /* Name section */
        name = sections.getSingleLine("Name");

        /* Levels section */
        levelNames = sections.getSingleSection("Levels");

        index_loaded = true;

      } catch (format_error & index_e) {
        throw format_error("file " + filename + " is"
                           " neither a level (" + level_e.msg + ")"
                           " nor a levelset index (" + index_e.msg + ")");
      }
      continue;
    }
    const std::string & levelName = test_level.getName();
    if (!levels.insert(make_pair(levelName, sections)).second)
      throw format_error("duplicate level name: " + levelName);
  }
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

void levelset_c::loadLevel(level_c & level, const std::string & levelName) const {

  std::map<std::string, textsections_c>::const_iterator i = levels.find(levelName);
  if (i == levels.end())
    throw format_error("unknown level name: " + levelName);
  level.load(i->second);
}
