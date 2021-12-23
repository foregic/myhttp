#include "src/server.h"

#include <vector>

int main(int argv, char *argc[]) { // ./main 12100 5 100 设置端口号，listenNum,maxEvents

    std::unique_ptr<Server> server(new Server());
    server->start();
    return 0;
}