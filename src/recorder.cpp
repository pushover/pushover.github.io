#include "recorder.h"

#include "tools.h"

#include <fstream>
#include <sstream>
#include <iomanip>

void recorder_c::load(const std::string & filename) {
  std::ifstream stream(filename.c_str());

  if (!stream) throw std::exception();

  if (!getline(stream, levelsetName))
    throw std::exception();
  if (!getline(stream, levelName))
    throw std::exception();

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

