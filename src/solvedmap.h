#ifndef __SOLVED_MAP_H__
#define __SOLVED_MAP_H__

// this set contains all the checksums of all solved levels
// with this it is easy to find out if a certain level has been
// solved by the user

#include <string>
#include <set>

class solvedMap_c {

    private:

        std::set<std::string> map;

    public:

        // try to load the map
        solvedMap_c(void);
        ~solvedMap_c(void);

        void addLevel(const std::string & hash) { map.insert(hash); }

        bool solved(const std::string & hash) const {
            return map.find(hash) != map.end();
        }
};

#endif

