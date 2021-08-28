#include "header.h"

#include <string>
#include <cstring>
#include <thread>
#include <stdexcept>
#include <iostream>
#include <wait.h>
#include <pthread.h>

#define ISspace(x) isspace((int)(x))

void *accept_request(void *clien);
void execute_cgi(int client, const char *path, const char *method, const char *query_string);
void serve_file(int client, const char *filename);
int get_line(int sock, char *buf, int size);
void headers(int client, const char *filename);
int start(u_short &port);

void *accept_request(void *clien)
{
    int client_socket = *(int *)clien;
    char buf[1024];
    int numchars;
    char method[255];
    char url[255];
    char path[512];
    size_t i, j;
    struct stat st;
    int cgi = 0;
    char *query_string = NULL;

    numchars = get_line(client_socket, buf, sizeof(buf));
    i = 0;
    j = 0;
    while (ISspace(buf[j]) && (i < sizeof(method) - 1))
    {
        //提取其中的请求方式
        method[i] = buf[j];
        i++;
        j++;
    }
    method[i] = '\0';

    if (strcasecmp(method, "GET") && strcasecmp(method, "POST"))
    {
        not_implemented(client_socket);
        return NULL;
    }

    if (strcasecmp(method, "POST") == 0)
        cgi = 1;

    i = 0;
    while (ISspace(buf[j]) && (j < sizeof(buf)))
        j++;

    while (!ISspace(buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf)))
    {
        url[i] = buf[j];
        i++;
        j++;
    }
    url[i] = '\0';

    //GET请求url可能会带有?,有查询参数
    if (strcasecmp(method, "GET") == 0)
    {

        query_string = url;
        while ((*query_string != '?') && (*query_string != '\0'))
            query_string++;

        /* 如果有?表明是动态请求, 开启cgi */
        if (*query_string == '?')
        {
            cgi = 1;
            *query_string = '\0';
            query_string++;
        }
    }

    sprintf(path, "httpdocs%s", url);

    if (path[strlen(path) - 1] == '/')
    {
        strcat(path, "test.html");
    }

    if (stat(path, &st) == -1)
    {
        while ((numchars > 0) && strcmp("\n", buf))
            numchars = get_line(client_socket, buf, sizeof(buf));

        not_found(client_socket);
    }
    else
    {

        if ((st.st_mode & S_IFMT) == S_IFDIR) //S_IFDIR代表目录
                                              //如果请求参数为目录, 自动打开test.html
        {
            strcat(path, "/test.html");
        }

        //文件可执行
        if ((st.st_mode & S_IXUSR) ||
            (st.st_mode & S_IXGRP) ||
            (st.st_mode & S_IXOTH))
            //S_IXUSR:文件所有者具可执行权限
            //S_IXGRP:用户组具可执行权限
            //S_IXOTH:其他用户具可读取权限
            cgi = 1;

        if (!cgi)

            serve_file(client_socket, path);
        else
            execute_cgi(client_socket, path, method, query_string);
    }

    close(client_socket);
    //printf("connection close....client: %d \n",client);
    return NULL;
}

//执行cgi动态解析
void execute_cgi(int client, const char *path,
                 const char *method, const char *query_string)
{

    char buf[1024];
    int cgi_output[2];
    int cgi_input[2];

    pid_t pid;
    int status;

    int i;
    char c;

    int numchars = 1;
    int content_length = -1;
    //默认字符
    buf[0] = 'A';
    buf[1] = '\0';
    if (strcasecmp(method, "GET") == 0)

        while ((numchars > 0) && strcmp("\n", buf))
        {
            numchars = get_line(client, buf, sizeof(buf));
        }
    else
    {

        numchars = get_line(client, buf, sizeof(buf));
        while ((numchars > 0) && strcmp("\n", buf))
        {
            buf[15] = '\0';
            if (strcasecmp(buf, "Content-Length:") == 0)
                content_length = atoi(&(buf[16]));

            numchars = get_line(client, buf, sizeof(buf));
        }

        if (content_length == -1)
        {
            bad_request(client);
            return;
        }
    }

    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    send(client, buf, strlen(buf), 0);
    if (pipe(cgi_output) < 0)
    {
        internal_server_error(client);
        return;
    }
    if (pipe(cgi_input) < 0)
    {
        internal_server_error(client);
        return;
    }

    if ((pid = fork()) < 0)
    {
        internal_server_error(client);
        return;
    }
    if (pid == 0) /* 子进程: 运行CGI 脚本 */
    {
        char meth_env[255];
        char query_env[255];
        char length_env[255];

        dup2(cgi_output[1], 1);
        dup2(cgi_input[0], 0);

        close(cgi_output[0]); //关闭了cgi_output中的读通道
        close(cgi_input[1]);  //关闭了cgi_input中的写通道

        sprintf(meth_env, "REQUEST_METHOD=%s", method);
        putenv(meth_env);

        if (strcasecmp(method, "GET") == 0)
        {
            //存储QUERY_STRING
            sprintf(query_env, "QUERY_STRING=%s", query_string);
            putenv(query_env);
        }
        else
        { /* POST */
            //存储CONTENT_LENGTH
            sprintf(length_env, "CONTENT_LENGTH=%d", content_length);
            putenv(length_env);
        }

        execl(path, path, NULL); //执行CGI脚本
        exit(0);
    }
    else
    {
        close(cgi_output[1]);
        close(cgi_input[0]);
        if (strcasecmp(method, "POST") == 0)

            for (i = 0; i < content_length; i++)
            {

                recv(client, &c, 1, 0);

                write(cgi_input[1], &c, 1);
            }

        //读取cgi脚本返回数据

        while (read(cgi_output[0], &c, 1) > 0)
        //发送给浏览器
        {
            send(client, &c, 1, 0);
        }

        //运行结束关闭
        close(cgi_output[0]);
        close(cgi_input[1]);

        waitpid(pid, &status, 0);
    }
}

void serve_file(int client, const char *filename)
{
    FILE *resource = NULL;
    int numchars = 1;
    char buf[1024];
    buf[0] = 'A';
    buf[1] = '\0';
    while ((numchars > 0) && strcmp("\n", buf))
    {
        numchars = get_line(client, buf, sizeof(buf));
    }

    //打开文件
    resource = fopen(filename, "r");
    if (resource == NULL)
        not_found(client);
    else
    {
        headers(client, filename);
        send_file(client, resource);
    }
    fclose(resource); //关闭文件句柄
}

//解析一行http报文
int get_line(int sock, char *buf, int size)
{
    int i = 0;
    char c = '\0';
    int n;

    while ((i < size - 1) && (c != '\n'))
    {
        n = recv(sock, &c, 1, 0);

        if (n > 0)
        {
            if (c == '\r')
            {

                n = recv(sock, &c, 1, MSG_PEEK);
                if ((n > 0) && (c == '\n'))
                    recv(sock, &c, 1, 0);
                else
                    c = '\n';
            }
            buf[i] = c;
            i++;
        }
        else
            c = '\n';
    }
    buf[i] = '\0';
    return (i);
}

void headers(int client, const char *filename)
{

    char buf[1024];

    (void)filename; /* could use filename to determine file type */
                    //发送HTTP头
    strcpy(buf, "HTTP/1.0 200 OK\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
}

int start(u_short &port)
{
    int http_socket = 0, option;
    sockaddr_in name;
    http_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (http_socket == -1)
        throw std::invalid_argument("socket建立失败");
    socklen_t optlen;
    optlen = sizeof(option);

    option = 1;
    setsockopt(http_socket, SOL_SOCKET, SO_REUSEADDR, (void *)&option, optlen);
    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(http_socket, (sockaddr *)&name, sizeof(name)) < 0)
        throw std::invalid_argument("无效的端口号");

    listen(http_socket, 5);
    return http_socket;
}

int main(int argv, char *argc[])
{
    int server_socket = -1;
    // u_short port = std::stoi(12100); //监听端口号
    u_short port = 12100; //监听端口号

    sockaddr_in client;
    socklen_t client_len = sizeof(client);
    try
    {
      server_socket = start(port);

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        exit(-1);
    }
    
    pthread_t newthread;

    std::cout << "http服务器套接字：" << server_socket << std::endl;
    std::cout << "http服务器启动端口号：" << port << std::endl;
    while (1)
    {
        int client_socket = accept(server_socket,
                                   (sockaddr *)&client,
                                   &client_len);
        std::cout << "新的socket连接，ip：" << inet_ntoa(client.sin_addr)
                  << "\t端口：" << ntohs(client.sin_port) << std::endl;
        if (client_socket == -1)
        {
            throw std::invalid_argument("错误的连接客户端");
        }
        if (pthread_create(&newthread, NULL, accept_request, (void *)&client_socket) != 0)
            perror("pthread_create");
    }
    close(server_socket);
    return 0;
}