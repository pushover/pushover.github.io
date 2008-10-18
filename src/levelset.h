#ifndef __LEVELSET_H__
#define __LEVELSET_H__

#include <string>
#include <vector>
#include <map>

class level_c;
class textsections_c;

class levelset_c {

  private:

    std::string name;
    unsigned int priority;
    std::vector<std::string> levelNames;
    std::map<std::string, textsections_c> levels;

  public:

    levelset_c(const std::string & path);
    const std::string getName(void) const { return name; }
    const unsigned int getPriority(void) const { return priority; }

    const std::vector<std::string> & getLevelNames(void) const { return levelNames; }
    void loadLevel(level_c & level, const std::string & levelName) const;
};

#endif
