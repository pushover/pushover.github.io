#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <string>
#include <vector>

// return the path to the home directory
// this is implemented in a platform dependent way
std::string getHome(void);

// return all non-hidden directory entries
std::vector<std::string> directoryEntries(const std::string & path);

#endif
