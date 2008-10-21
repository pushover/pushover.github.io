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
