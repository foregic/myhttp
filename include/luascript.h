
#ifndef _LUASCRIPT_H
#define _LUASCRIPT_H

#include <lua.hpp>
#include <string>

class Lua {
public:
    Lua() : L(luaL_newstate()) {
        luaopen_base(L);
        // 导入lua基础库
        luaL_openlibs(L);
        //加载lua文件
        luaL_dofile(L, "script/post.lua");
    }
    ~Lua() {
        lua_close(L);
    }

    template <typename... Args>
    std::string callFunc(const char *str, Args... args) {
        lua_getglobal(L, str);
        pushArgs(args...);
        lua_pcall(L, sizeof...(Args), 1, 0);
        auto result = lua_tostring(L, -1);
        lua_pop(L, 1);
        return result;
    }

    template <typename T, typename... Args>
    void pushArgs(T t, Args... args) {
        lua_pushstring(L, t); // 参数压栈
        pushArgs(args...);
    }

    void pushArgs() {
    }

private:
    lua_State *L;
};

class Script {

private:
    Lua lua;

public:
    template <typename... Args>
    std::string getPost(Args... args) {
        return lua.callFunc("getPostResponse", args...);
    }
};

#endif /* _LUASCRIPT_H */