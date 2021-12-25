/*
 * @Author       : foregic
 * @Date         : 2021-08-28 12:33:09
 * @LastEditors  : foregic
 * @LastEditTime : 2021-12-25 15:56:07
 * @FilePath     : /httpserver/main.cpp
 * @Description  :
 */

#include "server.h"

int main(int argv, char *argc[]) { // ./main 12100 5 100 设置端口号，listenNum,maxEvents

    std::unique_ptr<Server> server(new Server());
    server->start();
    return 0;
}
