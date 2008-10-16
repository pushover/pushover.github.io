#include "luaclass.h"

extern "C" {
#include "lauxlib.h"
}

#include <string.h>

luaClass_c::luaClass_c(void) {

  L = luaL_newstate();

}
luaClass_c::~luaClass_c(void) {

  lua_close(L);

}

unsigned int luaClass_c::getArraySize(const std::string & name) {
  lua_getglobal(L, name.c_str());
  return lua_objlen(L, -1);
}

/* functions to get and set variables */
void luaClass_c::setNumber(const std::string & name, lua_Number value) {
}
void luaClass_c::setString(const std::string & name, const std::string & value) {
}
void luaClass_c::setBool(const std::string & name, bool value) {
}

lua_Number luaClass_c::getNumberArray(const std::string & name, unsigned int idx) {
  lua_getglobal(L, name.c_str());
  lua_pushnumber(L, idx);
  lua_gettable(L, -2);

  lua_Number result = lua_tonumber(L, -1);
  lua_pop(L, 2);

  return result;
}

lua_Number luaClass_c::getNumber(const std::string & name) {
  lua_getglobal(L, name.c_str());
  if (!lua_isnumber(L, -1)) throw new luaTypeException_c();
  return lua_tointeger(L, -1);
}
bool luaClass_c::getBool(const std::string & name) {
  lua_getglobal(L, name.c_str());
  return lua_toboolean(L, -1) != 0;
}

/* functions to evaluate lua code */
int luaClass_c::doFile(const std::string & fname) {
  luaL_loadfile(L, fname.c_str()) || lua_pcall(L, 0, 0, 0);
}
int luaClass_c::doString(const std::string & code) {
  luaL_loadbuffer(L, code.c_str(), code.length(), "line") || lua_pcall(L, 0, 0, 0);
}

/* functions that allow calling lua functions
 *
 * I decided to leave out the generic interface and only provide some
 * overloaded functions to call a lua function with specific parameters
 * the return value is encoded within the name, the othe parameters
 * should be selected by the ...
 */
void luaClass_c::callV(const std::string & fname) {
}
void luaClass_c::callV(const std::string & fname, lua_Number p1) {
}
lua_Number luaClass_c::callN(const std::string & fname) {
}
lua_Number luaClass_c::callN(const std::string & fname, lua_Number p1) {
}

