
#pragma once

#include <bits/stdc++.h>
#include <lua.hpp>
#include <string>

using namespace std;

class Script {
    // using namespace std;

public:
    void test() {

        lua_State *L = luaL_newstate();
        luaopen_base(L);

        // load Lua base libraries
        luaL_openlibs(L);

        //加载1.lua文件，之前的版本luaL_dofile（）函数的第二个参数要写绝对路径，否则执行后会提示“unprotected error in call to lua api(attampt to call a nil value)”错误
        luaL_dofile(L, "script/post.lua");

        string a = "123", b = "456", c = "789", d = "147", e = "258", f = "369";

        lua_getglobal(L, "getPostResponse");

        lua_pushstring(L, a.c_str());
        lua_pushstring(L, b.c_str());
        lua_pushstring(L, c.c_str());
        lua_pushstring(L, d.c_str());
        lua_pushstring(L, e.c_str());
        lua_pushstring(L, f.c_str());

        lua_pcall(L, 6, 1, 0);

        string result = lua_tostring(L, -1);

        cout << result << endl;

        lua_pop(L, 1);

        // 释放Lua运行时环境.
        lua_close(L);
    }
};
