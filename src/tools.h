#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <string>
#include <vector>

// initialize the random number generator from the current time
// (implemented in a platform dependent way)
void srandFromTime(void);

// return the path to the home directory
// (implemented in a platform dependent way)
std::string getHome(void);

// return all non-hidden directory entries
std::vector<std::string> directoryEntries(const std::string & path);

#endif
