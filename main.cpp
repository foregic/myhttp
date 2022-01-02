/*
 * @Author       : foregic
 * @Date         : 2021-08-28 12:33:09
 * @LastEditors  : foregic
 * @LastEditTime : 2022-01-02 12:55:45
 * @FilePath     : /httpserver/main.cpp
 * @Description  :
 */

#include "server.h"

int main(int argv, char *argc[]) { // ./main 12100 5 100 设置端口号，listenNum,maxEvents

    Server::GetInstance()->start();
    return 0;
}
