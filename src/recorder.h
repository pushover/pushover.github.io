#ifndef __RECORDER_H__
#define __RECORDER_H__

#include <vector>
#include <string>

class recorder_c {

  public:

    recorder_c(std::string fname);
    recorder_c(void);

    void save(const std::string leve);

    void addEvent(int event) { record.push_back(event); }
    int getEvent(void) { return playpos < record.size() ? record[playpos++] : 0; }
    const std::string getLevel(void) const { return level; }
    bool endOfRecord(void) { return playpos >= record.size(); }

  private:

    std::vector<int> record;
    unsigned int playpos;
    std::string level;

};

#endif
