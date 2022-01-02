/*
 * @Author       : foregic
 * @Date         : 2021-12-20 17:24:11
 * @LastEditors  : foregic
 * @LastEditTime : 2022-01-02 13:37:09
 * @FilePath     : /httpserver/include/http.h
 * @Description  :
 */
#ifndef _HTTP_H
#define _HTTP_H

#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>

#include "api.h"
#include "dao.h"
#include "httpresponse.h"
#include "luascript.h"

#define SERVER_STRING "Server: myhttp/1.0\r\n"
#define BUFFER_SIZE 1024

class Http;

class HttpRequest;

class Http {

public:
    virtual void print() = 0;
    virtual void response(int client) = 0;
};

class Server;

#include <fstream>
class HttpRequest {
    using string = std::string;
    using stringstream = std::stringstream;
    using KeyValue = std::unordered_map<std::string, std::string>;

private:
    int client; // 客户端套接字

    string request; // 请求内容

    Script script; // lua脚本

    string method;   // 请求方法
    string url;      // 请求url
    string version;  // http版本号
    KeyValue header; // 请求头
    string body;     // 请求体

    KeyValue parameters; // url里携带的参数

    Server *server;

    std::unique_ptr<HttpResponse> httpResponse;
    bool isAlive;
    sockaddr_in addr;

public:
    HttpRequest() = delete;
    virtual ~HttpRequest() = default;
    HttpRequest(int clientfd, Server *server_) : client(clientfd), server(server_) {
        socklen_t len = sizeof(addr);
        getpeername(client, (sockaddr *)&addr, &len);
        isAlive = requestRead();
        http_request_parse(request);
        ParseParameters();
        PrintParameters();
    }
    explicit HttpRequest(const string &str, Server *server_);

    void print();
    void response(int fd);

    string getHeaderLine(const string &key) const;

    bool http_request_parse(const string &http_request);

private:
    bool requestRead() {
        int bytesRead;
        while (true) {
            char buffer[BUFFER_SIZE];
            bytesRead = recv(client, buffer, BUFFER_SIZE, 0);
            if (bytesRead == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                return false;
            } else if (bytesRead == 0) {
                return false;
            }
            request += buffer;
        }
        printf("数据读取完毕\n:%s", request.c_str());
        return true;
    }

    string decode(const string &str) {
        string ans;
        for (size_t i = 0; i < str.size(); ++i) {
            if (str[i] == '%') {
                i += 2;
            } else {
                ans.push_back(str[i]);
            }
        }
        return ans;
    }

    bool file_not_exist(const std::string &name) {
        return (access(name.c_str(), F_OK) == -1);
    }

    int GetFd() {
        return client;
    }

    char *GetIP() {
        return inet_ntoa(addr.sin_addr);
    }
    int GetPort() {
        return ntohs(addr.sin_port);
    }

    bool IsKeepAlive() {
        if (header.count("Connection") == 1) {
            return header.find("Connection")->second == "keep-alive" && version == "HTTP/1.1";
        }
        return false;
    }

    void ParseParameters() {
        auto mark = url.find('?');
        if (mark == std::string::npos) {
            return;
        }

        int prev = mark + 1, pos_equal = mark + 1, pos_and = mark + 1;

        int length = this->url.size();
        while (pos_and < length - 1) {
            while (pos_and < length && this->url[pos_and] != '&') {
                pos_and++;
            }
            ++pos_and;
            while (this->url[pos_equal] != '=') {
                ++pos_equal;
            }
            ++pos_equal;
            string key = this->url.substr(prev, pos_equal - prev - 1);
            string value = this->url.substr(pos_equal, pos_and - pos_equal - 1);
            prev = pos_and;
            parameters[key] = value;
        }
        url = url.substr(0, mark);
    }

    void PrintParameters() {
        for (auto &item : parameters) {
            std::cout << item.first << " = " << item.second << std::endl;
        }
        std::cout << url << std::endl;
    }

    void headers(int client, const char *file);
    void post_response(KeyValue dict, int client);
    void bad_request(int client);
    void not_found(int client);
    void internal_server_error(int client);
    void not_implemented(int client);
    void send_file(int client, FILE *filename);
};

#endif /* _HTTP_H */
