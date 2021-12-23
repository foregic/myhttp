//
// Created by LD on 2021/12/20.
//

#include "server.h"

void Server::socketCreate() {
    int option;
    this->serverSocket = socket(AF_INET, SOCK_STREAM, 0); //套接字
    if (serverSocket == -1) {
        perror("socket failed\n");
        exit(0);
    }
    socklen_t optlen;
    optlen = sizeof(option);
    option = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (void *)&option, optlen);

    sockaddr_in http_addr;                                                   //本机网络地址，ip端口
    memset(&http_addr, 0, sizeof(http_addr));                                //格式化
    http_addr.sin_family = AF_INET;                                          //设置协议格式
    http_addr.sin_port = htons(port);                                        //设置端口号
    http_addr.sin_addr.s_addr = htonl(INADDR_ANY);                           //设置
    if (bind(serverSocket, (sockaddr *)&http_addr, sizeof(http_addr)) < 0) { //绑定套接字
        perror("bind failed\n");
        exit(0);
    }
    if (listen(serverSocket, listenNum) < 0) { //等待连接队列的最大长度
        perror("listen failed\n");
        exit(0);
    }
    // 打印本机启动ip信息
    {
        getHostName();
        char addressString[INET_ADDRSTRLEN];
        struct ifaddrs *ifap;
        getifaddrs(&ifap);
        void *tmpAddrPtr;

        tmpAddrPtr = &((struct sockaddr_in *)ifap->ifa_addr)->sin_addr;

        inet_ntop(AF_INET, tmpAddrPtr, addressString, INET_ADDRSTRLEN);

        printf("[%s]\tServer start at %s %s %s %s:%d\n", getTime(), hostName, host->h_name, ifap->ifa_name, addressString, port);
    }
}

void Server::epollCreate() {
    this->epollfd = epoll_create(maxEvents); //创建epoll文件描述符

    epoll_event ev;      //添加监听描述符事件
    ev.events = EPOLLIN; //可读事件
    ev.data.fd = this->serverSocket;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, this->serverSocket, &ev)) { //将监听事件加入到epoll中
        perror("epol_ctl failed\n");
        exit(0);
    }
}

const char *Server::getTime() {
    std::time_t result = std::time(NULL);
    this->time = std::move(asctime(std::localtime(&result)));
    time.pop_back();
    return time.c_str();
}

Server::~Server() {
    close(serverSocket);
}

void Server::start() {
    printf("[%s]\tMyHttp Server start\n", this->getTime());
    tp.run();
    socketCreate();
    epollCreate();

    while (1) {
        epoll_event events[maxEvents]; //存放有事件发生的数组

        int infds = epoll_wait(epollfd, events, maxEvents, -1);

        if (infds < 0) {
            perror("epoll_wait()");
            continue;
        }
        for (int ii = 0; ii < infds; ++ii) {
            epoll_event ev;

            if (events[ii].data.fd == this->serverSocket) { //发生事件的套接字为服务器套接字
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int client_socket = accept(this->serverSocket, (struct sockaddr *)&client, &len);

                int nNetTimeout = 1000; // 1秒
                //发送时限
                setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&nNetTimeout, sizeof(int));
                //接收时限
                setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout, sizeof(int));
                // setsockopt(client_socket, );

                printf("[%s]\t%s:%d build connect\n", getTime(), inet_ntoa(client.sin_addr), ntohs(client.sin_port));

                if (client_socket < 0) {
                    perror("accept() failed\n");
                    continue;
                }
                fcntl(client_socket, F_SETFL, fcntl(client_socket, F_GETFL, 0)); //设置为非阻塞模式

                ev.events = EPOLLIN; //设置读事件
                ev.data.fd = client_socket;

                epoll_ctl(epollfd, EPOLL_CTL_ADD, client_socket, &ev);
            } else {
                char buffer[1024];
                memset(buffer, 0, sizeof(buffer));

                struct sockaddr_in client;
                socklen_t len = sizeof(client);

                getpeername(events[ii].data.fd, (struct sockaddr *)&client, &len);    // 获得套接字对应的对方的sockaddr
                ssize_t data_size = read(events[ii].data.fd, buffer, sizeof(buffer)); // buffer里存了请求报文
                if (data_size <= 0) {
                    //如果客户端关闭请求

                    printf("[%s]\tclient disconnetced %s:%d\n", getTime(), inet_ntoa(client.sin_addr), ntohs(client.sin_port));

                    memset(&ev, 0, sizeof(ev));
                    ev.events = EPOLLIN;
                    ev.data.fd = events[ii].data.fd; //客户端的请求套接字
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, events[ii].data.fd, &ev);
                    close(events[ii].data.fd);
                    continue;
                }
                if (strnlen(buffer, 1024) <= 0) {
                    printf("[%s]\tget data from %s:%d is None\n", getTime(), inet_ntoa(client.sin_addr), ntohs(client.sin_port));
                    continue;
                }

                printf("[%s]\tget data from %s:%d\n", getTime(), inet_ntoa(client.sin_addr), ntohs(client.sin_port));
                std::unique_ptr<Http> clientHttp(new Httpimpl(buffer));
                auto f = std::function<void()>([&] { clientHttp->response(events[ii].data.fd); });
                tp.submit(f);
                close(events[ii].data.fd);
            }
        }
    }
}