
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

        // 导入lua基础库
        luaL_openlibs(L);

        //加载lua文件
        luaL_dofile(L, "script/post.lua");

        string a = "123", b = "456", c = "789", d = "147", e = "258", f = "369";

        //调用lua函数
        lua_getglobal(L, "getPostResponse");

        lua_pushstring(L, a.c_str());
        lua_pushstring(L, b.c_str());
        lua_pushstring(L, c.c_str());
        lua_pushstring(L, d.c_str());
        lua_pushstring(L, e.c_str());
        lua_pushstring(L, f.c_str());

        // 调用函数
        lua_pcall(L, 6, 1, 0);

        string result = lua_tostring(L, -1);

        cout << result << endl;

        //弹出结果
        lua_pop(L, 1);

        // 释放Lua运行时环境.
        lua_close(L);
    }
};
