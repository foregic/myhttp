#include "header.h"

#define MAXEVENTS 5

int main(int argv, char *argc[])
{
    int server_socket = -1;
    u_short port = 12100; //监听端口号
    if (argv == 2)
    {
        port = atoi(argc[1]);
    }

    sockaddr_in client; //本机套接字
    socklen_t client_len = sizeof(client);
    try
    {
        server_socket = start(port); //绑定套接字,服务器启动
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        exit(-1);
    }

    std::cout << "http服务器套接字：" << server_socket << std::endl;
    std::cout << "http服务器启动端口号：" << port << std::endl;

    int epollfd = epoll_create(5); //创建epoll文件描述符

    epoll_event ev;      //添加监听描述符事件
    ev.events = EPOLLIN; //可读事件
    ev.data.fd = server_socket;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, server_socket, &ev); //将监听事件加入到epoll中

    while (1)
    {
        epoll_event events[MAXEVENTS]; //存放有事件发生的数组

        int infds = epoll_wait(epollfd, events, MAXEVENTS, -1);

        if (infds < 0)
        {
            perror("epoll_wait()");
            continue;
        }
        for (int ii = 0; ii < infds; ++ii)
        {
            if (events[ii].data.fd == server_socket)
            { //发生事件的套接字为服务器套接字
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int client_socket = accept(server_socket, (struct sockaddr *)&client, &len);
                printf("%s,%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
                if (client_socket < 0)
                {
                    perror("accept() failed\n");
                    continue;
                }
                fcntl(client_socket, F_SETFL, fcntl(client_socket, F_GETFL, 0));

                ev.events = EPOLLIN; //设置读事件
                ev.data.fd = client_socket;

                epoll_ctl(epollfd, EPOLL_CTL_ADD, client_socket, &ev);
            }
            else
            {
                char buffer[1024];
                memset(buffer, 0, sizeof(buffer));

                ssize_t data_size = read(events[ii].data.fd, buffer, sizeof(buffer)); //buffer里存了请求报文
                if (data_size <= 0)
                {
                    //如果客户端关闭请求
                    printf("client disconnetced\n");
                    memset(&ev, 0, sizeof(ev));
                    ev.events = EPOLLIN;
                    ev.data.fd = events[ii].data.fd; //客户端的请求套接字
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, events[ii].data.fd, &ev);
                    close(events[ii].data.fd);
                    continue;
                }

                response(events[ii].data.fd, buffer);
                close(events[ii].data.fd);
            }
        }
    }
    close(server_socket);
    return 0;
}