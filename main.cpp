#include "src/server.h"

#include <vector>

int main(int argv, char *argc[]) { // ./main 12100 5 100 设置端口号，listenNum,maxEvents

    // std::mutex mx;
    // std::unique_lock<std::mutex> lock(mx);
    std::unique_ptr<Server> server(new Server());
    server->start();
    return 0;
}