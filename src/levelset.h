#ifndef __LEVELSET_H__
#define __LEVELSET_H__

#include <string>
#include <vector>
#include <map>

class levelData_c;
class textsections_c;

class levelset_c {

  private:

    std::string name;
    unsigned int priority;
    std::vector<std::string> levelNames;
    std::map<std::string, std::string> checksums;
    std::map<std::string, textsections_c> levels;

  public:

    levelset_c(const std::string & path);
    const std::string getName(void) const { return name; }
    const unsigned int getPriority(void) const { return priority; }

    const std::vector<std::string> & getLevelNames(void) const { return levelNames; }
    const std::string & getChecksum(const std::string & levelName) const;
    void loadLevel(levelData_c & level, const std::string & levelName) const;
};

class levelsetList_c {

  private:

    std::map<std::string, levelset_c> levelsets;
    std::vector<std::string> levelsetNames;
    std::vector<std::pair<unsigned int, std::string> > sortHelper;

  public:

    levelsetList_c() {};
    void load(const std::string & path);
    const std::vector<std::string> & getLevelsetNames(void) const { return levelsetNames; }
    const levelset_c & getLevelset(const std::string & levelsetName) const;
};

#endif
