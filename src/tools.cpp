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
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <dirent.h>

#include <stdexcept>

std::string getHome(void) {

#ifdef WIN32

  static char userHome[MAX_PATH];

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
  userHome[size] = '\\';
  userHome[size+1] = '\0';

  std::string home = std::string(userHome) + "\\pushover\\";


#else

  std::string home = std::string(getenv("HOME"))+"/.pushover/";

#endif

  DIR * dir = ::opendir(home.c_str());

  if (!dir)
  {
    // create it
    if (::mkdir(home.c_str(), S_IRWXU) != 0)
      throw std::runtime_error("Can not create home directory\n");
  }
  else
  {
    if (::closedir(dir) != 0)
      throw std::runtime_error("Can not close dir in find home\n");
  }

  return home;
}


