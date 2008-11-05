#include "solvedmap.h"
#include "tools.h"

#include <fstream>
#include <iostream>

solvedMap_c::solvedMap_c(void) {

  std::ifstream in((getHome()+"solved.txt").c_str());
  std::string line;

  while (in.good()) {
    getline(in, line);
    if (line != "")
      map.insert(line);
  }
}

solvedMap_c::~solvedMap_c(void) {  }

void solvedMap_c::addLevel(const std::string & hash) {

  map.insert(hash);

  std::ofstream out((getHome()+"solved.txt").c_str());

  for (std::set<std::string>::iterator i = map.begin(); i != map.end(); i++)
    out << *i << std::endl;
}

