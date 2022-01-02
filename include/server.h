/*
 * @Author       : foregic
 * @Date         : 2021-12-20 14:25:13
 * @LastEditors  : foregic
 * @LastEditTime : 2022-01-02 12:56:06
 * @FilePath     : /httpserver/include/server.h
 * @Description  :
 */

#ifndef _SERVER_H
#define _SERVER_H

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
    static Server *server;
    u_short port;     // 服务器端口号
    int serverSocket; // 套接字
    int listenNum;

    int maxEvents;
    int epollfd; // 获取epoll描述符

    std::string time;

    char hostName[128];
    struct hostent *host;

    std::unique_ptr<ThreadPool> tp;
    std::unique_ptr<Log> log;

public:
    void GetHostName() {
        gethostname(hostName, sizeof(hostName));
        host = gethostbyname(hostName);
    }

private:
    Server(u_short _port = 12100, int _listenNum = 5, int _maxevents = 100, int threadPoolSize = 4)
        : port(_port), listenNum(_listenNum), maxEvents(_maxevents), tp(PoolFactory::create(threadPoolSize)), log(Log::getInstance()) {
        tp->run();
    }

public:
    ~Server();

    static Server *GetInstance() { return server; }

    void SocketCreate();
    void EpollCreate();

    void SocketInit();
    void BindAddress();
    void SetListenNUm();

    bool EpollRegisterFd();

    void CloseConnection(int fd);

    char *GetIP(sockaddr_in client) {
        return inet_ntoa(client.sin_addr);
    }
    int GetPort(sockaddr_in client) {
        return ntohs(client.sin_port);
    }

    void start();

    static std::string resourcePath;
};

#endif /* _SERVER_H */
