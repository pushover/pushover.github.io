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

#ifndef __LUA_CLASS_H__
#define __LUA_CLASS_H__

#include <string>


extern "C" {
#include <lua.h>
}


class luaTypeException_c {
};

/* the lua class encapsulates one lua state with all necessary functions for interaction with it */
class luaClass_c {

  private:

    lua_State *L;

  public:

    luaClass_c(void);
    ~luaClass_c(void);

    unsigned int getArraySize(const std::string & name);

    lua_Number getNumber(const std::string & name);
    lua_Number getNumberArray(const std::string & name, unsigned int idx);
    bool getBool(const std::string & name);

    /* functions to evaluate lua code */
    int doFile(const std::string & fname);
    int doString(const std::string & code);

};

#endif
