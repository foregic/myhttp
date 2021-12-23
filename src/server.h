/*
 * @Author       : foregic
 * @Date         : 2021-12-20 14:25:13
 * @LastEditors  : foregic
 * @LastEditTime : 2021-12-23 14:49:28
 * @FilePath     : /httpserver/src/server.h
 * @Description  :
 */

#ifndef _SERVER_H_
#define _SERVER_H_

#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <memory>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "http.h"
#include "threadPool.h"

#define BUFFER_SIZE 1024

class Server {

private:
    u_short port;
    int serverSocket;
    int listenNum;

    int maxEvents;
    int epollfd; // 获取epoll描述符

    std::string time;

    char hostName[128];
    struct hostent *host;

    threadPool tp;

public:
    void getHostName() {
        gethostname(hostName, sizeof(hostName));
        host = gethostbyname(hostName);
    }
    Server(u_short _port = 12100, int _listenNum = 5, int _maxevents = 100, int threadPoolSize = 4)
        : port(_port), listenNum(_listenNum), maxEvents(_maxevents), tp(threadPoolSize) {
        tp.run();
    }
    ~Server();

    void socketCreate();
    void epollCreate();

    const char *getTime();

    void start();
};

#endif /* _SERVER_H_ */
