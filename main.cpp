/*
 * @Author       : foregic
 * @Date         : 2021-08-28 12:33:09
 * @LastEditors  : foregic
 * @LastEditTime : 2021-12-30 01:39:46
 * @FilePath     : /httpserver/main.cpp
 * @Description  :
 */

#include "server.h"

std::mutex mx;
int sum = 0;


int main(int argv, char *argc[]) { // ./main 12100 5 100 设置端口号，listenNum,maxEvents

    std::unique_ptr<Server> server(new Server());
    server->start();

    // std::condition_variable cv;

    // threadPool *t = PoolFactory::create(5, 1000);
    // t->run();
    // for (int i = 0; i < 1000; i++) {
    //     t->submit([&] {
    //         std::unique_lock<std::mutex> lock(mx);
    //         sum += 1;
    //         if (sum == 1000) {
    //             cv.notify_all();
    //         }
    //     });
    // }
    // std::unique_lock<std::mutex> lock(mx);
    // cv.wait(lock, [&] { return sum == 1000; });

    // std::cout << "sum: " << sum << std::endl;

    return 0;
}
