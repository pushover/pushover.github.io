#include "tools.h"

#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <lmcons.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#endif

#include <sys/stat.h>
#include <dirent.h>

#include <stdexcept>

void srandFromTime(void) {

#ifdef WIN32
  FILETIME currentTime;
  GetSystemTimeAsFileTime(&currentTime);
  srand(currentTime.dwLowDateTime);
#else
  srand(time(0));
#endif

}

std::string getHome(void) {

#ifdef WIN32

  static char userHome[MAX_PATH+1];

  HKEY key;
  DWORD size = MAX_PATH;

  if (RegOpenKeyEx(HKEY_CURRENT_USER,
                   "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
                   0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS)
    userHome[0] = '\0';

  else if (RegQueryValueEx(key, "Personal", 0, 0, (LPBYTE)userHome, &size ) != ERROR_SUCCESS)
    userHome[0] = '\0';

  else
    RegCloseKey(key);

  size = strlen(userHome);
  userHome[size] = '\0';

  std::string home = std::string(userHome) + "\\pushover\\";

#else

  std::string home = std::string(getenv("HOME"))+"/.pushover/";

#endif

  DIR * dir = ::opendir(home.c_str());

  if (dir)
  {
    closedir(dir);
  }
  else
  {
    // create it
#ifdef WIN32
    if (::mkdir(home.c_str()) != 0)
#else
    if (::mkdir(home.c_str(), S_IRWXU) != 0)
#endif
      throw std::runtime_error("Can't create home directory: " + home);
  }

  return home;
}

std::vector<std::string> directoryEntries(const std::string & path) {
  DIR * dir = ::opendir(path.c_str());
  if (dir == NULL)
    throw std::runtime_error("Can't open directory: " + path);
  std::vector<std::string> entries;
  for (struct dirent * i = ::readdir(dir); i != NULL; i = ::readdir(dir))
    entries.push_back(i->d_name);
  if (::closedir(dir) != 0)
    throw std::runtime_error("Can't close directory: " + path);
  return entries;
}
