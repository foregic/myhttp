/*
 * @Author       : foregic
 * @Date         : 2021-12-20 17:30:42
 * @LastEditors  : foregic
 * @LastEditTime : 2022-01-02 13:41:04
 * @FilePath     : /httpserver/src/server.cpp
 * @Description  :
 */

#include "server.h"
std::string Server::resourcePath = "resources/http";
Server *Server::server = new Server();

void Server::BindAddress() {
    sockaddr_in http_addr;                         //本机网络地址，ip端口
    memset(&http_addr, 0, sizeof(http_addr));      //格式化
    http_addr.sin_family = AF_INET;                //设置协议格式
    http_addr.sin_port = htons(port);              //设置端口号
    http_addr.sin_addr.s_addr = htonl(INADDR_ANY); //设置本机IP

    if (bind(serverSocket, (sockaddr *)&http_addr, sizeof(http_addr)) == -1) { //绑定套接字
        perror("bind failed\n");
        exit(1);
    }
}

void Server::SetListenNUm() {
    if (listen(serverSocket, listenNum) == -1) { //等待连接队列的最大长度
        perror("listen failed\n");
        exit(1);
    }
}

void Server::SocketInit() {
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed\n");
        exit(1);
    }
    int option;
    socklen_t optlen;
    optlen = sizeof(option);
    option = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (void *)&option, optlen);
    assert(serverSocket >= 0);
}

void Server::SocketCreate() {

    SocketInit();
    BindAddress();
    SetListenNUm();

    {
        GetHostName();
        char addressString[INET_ADDRSTRLEN];
        struct ifaddrs *ifap;
        getifaddrs(&ifap);
        void *tmpAddrPtr;

        tmpAddrPtr = &((struct sockaddr_in *)ifap->ifa_addr)->sin_addr;

        inet_ntop(AF_INET, tmpAddrPtr, addressString, INET_ADDRSTRLEN);
        log->put("Server start at %s %s %s %s:%d\n", hostName, host->h_name, ifap->ifa_name, addressString, port);
    }
}

void Server::EpollCreate() {
    this->epollfd = epoll_create(maxEvents); //创建epoll文件描述符
    if (epollfd == -1) {
        perror("EpollCreate");
        exit(1);
    }
    epoll_event event;                // 添加监听描述符事件
    event.events = EPOLLIN | EPOLLET; // 可读事件, 边缘触发
    event.data.fd = this->serverSocket;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, this->serverSocket, &event)) { //将监听事件加入到epoll中
        perror("epol_ctl failed\n");
        exit(0);
    }
}

Server::~Server() {
    close(serverSocket);
}

bool Server::EpollRegisterFd() {

    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    int client_socket = accept(serverSocket, (struct sockaddr *)&client, &len);
    if (client_socket < 0) {
        return false;
    }

    if (client_socket < 0) {
        perror("accept() failed\n");
        return false;
    }
    fcntl(client_socket, F_SETFL, O_NONBLOCK | fcntl(client_socket, F_GETFL, 0)); //设置为非阻塞模式

    epoll_event ev;
    ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP; //可读事件,边缘触发模式,客户端关闭socket连接
    ev.data.fd = client_socket;

    epoll_ctl(epollfd, EPOLL_CTL_ADD, client_socket, &ev);
    return true;
}

void Server::CloseConnection(int fd) {
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0); // 删除epollfd里注册的fd
    close(fd);                                // 关闭客户端套接字
}

// std::string Server::ReadOnceive(int fd) {
//     std::string tmp;
//     char buffer[1024];
//     memset(buffer, 0, sizeof(buffer));
//     ssize_t data_size;

//     while (data_size = read(fd, buffer, sizeof(buffer))) {
//         if (data_size == 0) {
//             break;
//         }
//         if (data_size == -1) {
//             if (errno == EAGAIN || errno == EWOULDBLOCK) {
//                 break;
//             }
//             return false;
//         } else if (data_size == 0) {
//             return false;
//         }

//         tmp += buffer;
//     }
// }

void Server::start() {

    Log::write("MyHttp Server start");
    SocketCreate();
    EpollCreate();

    while (1) {
        epoll_event events[maxEvents]; //存放有事件发生的数组

        int infds = epoll_wait(epollfd, events, maxEvents, 0);
        if (infds == 0) {
            continue;
        }
        if (infds < 0 && errno != EINTR) { // 系统调用未被中断
            perror("epoll_wait()");
            continue;
        }
        for (int i = 0; i < infds; ++i) {
            int fd = events[i].data.fd;

            if (fd == serverSocket) { //发生事件的套接字为服务器套接字
                                      // while (1) {
                if (EpollRegisterFd() == false) {
                    break;
                }
                // }
                continue;
            } else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) { // 如果是客户端关闭连接，或者出错
                CloseConnection(fd);
            }

            else if (events[i].events & (EPOLLIN | EPOLLOUT)) { // 如果有读事件
                std::shared_ptr<HttpRequest> client(new HttpRequest(fd, this));
                auto f = std::function<void()>(
                    [=] {
                        client->response(fd);
                    });
                tp->submit(std::forward<decltype(f)>(f));
            }
            // else if (events[i].events & EPOLLOUT) {

            // }
        }
    }
}
