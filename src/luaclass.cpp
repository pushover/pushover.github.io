#include "luaclass.h"

extern "C" {
#include <lauxlib.h>
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
  return luaL_dofile(L, fname.c_str());
}
int luaClass_c::doString(const std::string & code) {
  return luaL_dostring(L, code.c_str());
}

