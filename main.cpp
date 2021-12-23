/*
 * @Author       : foregic
 * @Date         : 2021-08-28 12:33:09
 * @LastEditors  : foregic
 * @LastEditTime : 2021-12-23 23:44:18
 * @FilePath     : /httpserver/main.cpp
 * @Description  :
 */

// #include "server.h"

#include "include/luascript.h"

int main(int argv, char *argc[]) { // ./main 12100 5 100 设置端口号，listenNum,maxEvents

    Script test;
    test.test();

    // std::unique_ptr<Server> server(new Server());
    // server->start();
    return 0;
}