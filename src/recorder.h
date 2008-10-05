#ifndef __RECORDER_H__
#define __RECORDER_H__

#include <vector>
#include <string>

class recorder_c {

  public:

    recorder_c(void) : playpos(0), levelName("unknown") {};

    void load(const std::string & filename);
    void save() const;

    const std::string & getLevelName(void) const { return levelName; }
    void setLevelName(const std::string & levelName) { this->levelName = levelName; }

    void addEvent(int event) { record.push_back(event); }
    int getEvent(void) { return playpos < record.size() ? record[playpos++] : 0; }
    bool endOfRecord(void) const { return playpos >= record.size(); }

  private:

    std::vector<int> record;
    unsigned int playpos;
    std::string levelName;

};

#endif
