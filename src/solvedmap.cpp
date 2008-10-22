#include "solvedmap.h"

#include <fstream>
#include <iostream>

solvedMap_c::solvedMap_c(void) {

    std::ifstream in("solved.txt");
    std::string line;

    while (in) {
        in >> line;

        addLevel(line);
    }
}

solvedMap_c::~solvedMap_c(void) {

    std::ofstream out("solved.txt");

    for (std::set<std::string>::iterator i = map.begin(); i != map.end(); i++)
        out << *i << std::endl;
}

