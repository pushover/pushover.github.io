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

    /* functions to get and set variables */
    void setNumber(const std::string & name, lua_Number value);
    void setString(const std::string & name, const std::string & value);
    void setBool(const std::string & name, bool value);

    unsigned int getArraySize(const std::string & name);

    lua_Number getNumber(const std::string & name);
    lua_Number getNumberArray(const std::string & name, unsigned int idx);
    bool getBool(const std::string & name);

    /* functions to evaluate lua code */
    int doFile(const std::string & fname);
    int doString(const std::string & code);

    /* functions that allow calling lua functions
     *
     * I decided to leave out the generic interface and only provide some
     * overloaded functions to call a lua function with specific parameters
     * the return value is encoded within the name, the othe parameters
     * should be selected by the ...
     */
    void callV(const std::string & fname);
    void callV(const std::string & fname, lua_Number p1);
    lua_Number callN(const std::string & fname);
    lua_Number callN(const std::string & fname, lua_Number p1);

};

#endif
