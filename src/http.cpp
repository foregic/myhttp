/*
 * @Author       : foregic
 * @Date         : 2021-08-28 11:11:22
 * @LastEditors  : foregic
 * @LastEditTime : 2021-09-04 22:23:14
 * @FilePath     : /httpserver/src/http.cpp
 * @Description  : 
 */
#include "header.h"

/**
 * @description  : 解析请求的报文，将解析结果保存在传进来的地址
 * @param         {string} &http_request，请求报文
 * @param         {http_request_header} *phttphdr，解析结果
 * @return        {*}
 */
bool http_request_parse(const string &http_request, http_request_header *phttphdr)
{
    if (http_request.empty())
    {
        perror("http_request_parse: http_request is empty");
        return false;
    }
    if (phttphdr == nullptr)
    {
        perror("http_request_parse: phttphdr is NULL");

        return false;
    }

    string crlf("\r\n"), crlfcrlf("\r\n\r\n");
    size_t prev = 0, next = 0; //当前解析指针

    //解析http请求报文的起始行
    if ((next = http_request.find(crlf, prev)) != string::npos) //找到第一个\r\n的位置
    {
        string first_line(http_request.substr(prev, next - prev));
        prev = next; //移动指针到第一行末尾
        stringstream sstream(first_line);
        sstream >> (phttphdr->method);
        sstream >> (phttphdr->url);
        sstream >> (phttphdr->version);
    }
    else
    {
        perror("http_parser: http_request has not a \\r\\n");
        return false;
    }

    //查找"\r\n\r\n"的位置
    size_t pos_crlfcrlf = http_request.find(crlfcrlf, prev);
    if (pos_crlfcrlf == string::npos)
    {
        perror("http_request_parse: http_request has not a \"\r\n\r\n\"");
        return false;
    }

    //解析首部行
    string buff, key, value;
    while (1)
    {
        next = http_request.find(crlf, prev + 2);

        //如果找到的next不超过"\r\n\r\n"的位置
        if (next <= pos_crlfcrlf)
        {
            //buff保存了一行
            buff = http_request.substr(prev + 2, next - prev - 2);
            size_t end = 0;
            //跳过前置空白符，到达首部关键字的起始位置
            for (; isblank(buff[end]); ++end)
                ;
            int beg = end;
            //到达首部关键字的结束位置
            for (; buff[end] != ':' && !isblank(buff[end]); ++end)
                ;
            key = buff.substr(beg, end - beg);
            //跳至首部值的起始位置
            for (; (!isalpha(buff[end]) && !isdigit(buff[end])); ++end)
                ;
            beg = end;
            //到达首部值的结束位置
            for (; next != end; ++end)
                ;
            value = buff.substr(beg, end - beg);
            phttphdr->header.insert(make_tyhp_header(key, value));

            prev = next;
        }
        else
        {
            break;
        }
    }
    //获取http请求包的实体值
    phttphdr->body = http_request.substr(pos_crlfcrlf + 4, http_request.size() - pos_crlfcrlf - 4);
    return true;
}

/**
 * @description  : 根据key的值在phttphdr所指向的http_request_header中查找相对应的值
 * @param         {string} &key，key为关键字，header为首部行
 * @param         {http_request_header_line} &header
 * @return        返回空值表示查找失败，否则返回相应的值
 */
string get_value_from_http_request_header(const string &key, const http_request_header_line &header)
{
    if (header.empty())
        return "";
    http_request_header_line::const_iterator cit = header.find(key);
    if (cit == header.end())
        return "";
    return (*cit).second;
}

/**
 * @description  : 打印http_request_header里的header
 * @param         {http_request_header_line} &head，首部行
 * @return        {*}
 */
void print_http_request_header_line(const http_request_header_line &head)
{
    if (!head.empty())
    {
        http_request_header_line::const_iterator cit = head.begin();
        while (cit != head.end())
        {
            cout << cit->first << ":" << cit->second << endl;
            ++cit;
        }
    }
}

/**
 * @description  : 打印http_request_header请求头
 * @param         {http_request_header} *请求头指针
 * @return        {*}
 */
void print_http_request_header(http_request_header *phttphdr)
{
    if (NULL == phttphdr)
    {
        perror("phttphdr == NULL");
        return;
    }

    cout << phttphdr->method << " " << phttphdr->url << " " << phttphdr->version << endl;
    print_http_request_header_line(phttphdr->header);
    cout << endl
         << phttphdr->body << endl;
}

//发送HTTP头，解析成功，给客户端返回指定文件
void headers(int client, const char *file)
{
    printf("200 OK 发送文件\n");
    char buf[1024];

    memset(buf, 0, sizeof(buf));
    strcpy(buf, "HTTP/1.1 200 OK\r\n");
    // printf("%s", buf);

    send(client, buf, strlen(buf), 0);

    strcpy(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);

    FILE *filename = fopen(file, "r");
    send_file(client, filename);
    fclose(filename);

    strcpy(buf, "\r\n\r\n");
    send(client, buf, strlen(buf), 0);
}

//发送400，客户端请求的语法错误，服务器无法理解
void bad_request(int client)
{
    char buf[1024];
    sprintf(buf, "HTTP/1.1 400 BAD REQUEST\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "Content-type: text/html\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, sizeof(buf), 0);

    FILE *filename = fopen("../resources/http/400.html", "r");
    send_file(client, filename);
    fclose(filename);
    sprintf(buf, "\r\n\r\n");
    send(client, buf, strlen(buf), 0);
}

//发送404，服务器无法根据客户端请求找到资源
void not_found(int client)
{
    char buf[1024];
    sprintf(buf, "HTTP/1.1 404 NOT FOUND\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);

    FILE *filename = fopen("../resources/http/500.html", "r");
    send_file(client, filename);
    fclose(filename);
    sprintf(buf, "\r\n\r\n");
    send(client, buf, strlen(buf), 0);
}

//发送500，服务器内部错误，无法完成请求
void internal_server_error(int client)
{
    char buf[1024];
    //发送500
    sprintf(buf, "HTTP/1.1 500 Internal Server Error\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);

    FILE *filename = fopen("../resources/http/500.html", "r");
    send_file(client, filename);
    fclose(filename);
    sprintf(buf, "\r\n\r\n");
    send(client, buf, strlen(buf), 0);
}

//发送501，服务器不支持请求的功能，无法完成请求
void not_implemented(int client)
{
    char buf[1024];
    sprintf(buf, "HTTP/1.1 501 Not Implemented\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n\r\n");
    send(client, buf, strlen(buf), 0);

    FILE *filename = fopen("../resources/http/501.html", "r");
    send_file(client, filename);
    fclose(filename);
    sprintf(buf, "\r\n\r\n");
    send(client, buf, strlen(buf), 0);
}