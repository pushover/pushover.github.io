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

  private:

    std::vector<int> record;
    unsigned int playpos;
    std::string levelsetName;
    std::string levelName;

};

#endif
