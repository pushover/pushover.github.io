/* Pushover
 *
 * Pushover is the legal property of its developers, whose
 * names are listed in the COPYRIGHT file, which is included
 * within the source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA
 */

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

#include <boost/filesystem.hpp>

uint64_t getTime(void)
{
#ifdef WIN32
  FILETIME currentTime;
  GetSystemTimeAsFileTime(&currentTime);
  return currentTime.dwLowDateTime;
#else
  return time(0);
#endif
}


void srandFromTime(void) {

  srand(getTime());
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

  std::string lev = home+"levels";

  dir = ::opendir(lev.c_str());

  if (dir)
  {
    closedir(dir);
  }
  else
  {
    // create it
#ifdef WIN32
    if (::mkdir(lev.c_str()) != 0)
#else
    if (::mkdir(lev.c_str(), S_IRWXU) != 0)
#endif
      throw std::runtime_error("Can't create home directory: " + lev);
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


std::string findFileInDirectory(const std::string & path, const std::string name, uint16_t depth)
{
  boost::filesystem::path p (path);   // p reads clearer than argv[1] in the following code

  if (depth > 0)
  {
    if (boost::filesystem::exists(p))
    {
      if (boost::filesystem::is_regular_file(p) && p.filename() == name)
      {
        return p.string();
      }
      else if (boost::filesystem::is_directory(p))
      {
        for (boost::filesystem::directory_iterator i(p); i != boost::filesystem::directory_iterator(); ++i)
        {
          std::string f = findFileInDirectory(i->path().string(), name, depth-1);

          if (f != "") return f;
        }
      }
    }
  }

  return "";
}

// splits a string along a set of characters, attention, splitting characters must be single byte utf-8 encodable
std::vector<std::string> splitString(const std::string & text, const std::string & splitter)
{
  std::string current;
  std::vector<std::string> res;

  for (unsigned int i = 0; i < text.length(); i++)
  {
    if (splitter.find_first_of(text[i]) != splitter.npos)
    {
      if (current.length())
      {
        res.push_back(current);
      }
      current = "";
    }
    else
    {
      current += text[i];
    }
  }

  if (current.length())
  {
    res.push_back(current);
  }

  return res;
}

