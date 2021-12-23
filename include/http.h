/*
 * @Author       : foregic
 * @Date         : 2021-12-20 17:24:11
 * @LastEditors  : foregic
 * @LastEditTime : 2021-12-23 21:02:53
 * @FilePath     : /httpserver/include/http.h
 * @Description  :
 */
#ifndef _HTTP_H
#define _HTTP_H

#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>

#define SERVER_STRING "Server: myhttp/1.0\r\n"
#define BUFFER_SIZE 1024

#include "luascript.h"

class Http;

class HttpResonse;

class Httpimpl;

class Http {

public:
    virtual void print() = 0;
    virtual void response(int client) = 0;
};
#include <fstream>
class Httpimpl : public Http {
    using string = std::string;
    using stringstream = std::stringstream;
    using requestHeaders = std::unordered_map<std::string, std::string>;

private:
    int client; // 客户端套接字

    string method;         // 请求方法
    string url;            // 请求url
    string version;        // http版本号
    requestHeaders header; // 请求头
    string body;           // 请求体

    std::unique_ptr<HttpResonse> resonse;

public:
    Httpimpl() = delete;
    virtual ~Httpimpl() = default;

    explicit Httpimpl(const string str);

    virtual void print();
    virtual void response(int fd);

    string getHeaderLine(const string &key) const;

    void http_request_parse(const string &http_request);

private:
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

    inline bool file_exist(const std::string &name) {
        return (access(name.c_str(), F_OK) == -1);
    }

    void headers(int client, const char *file);
    void post_response(requestHeaders dict, int client);
    void bad_request(int client);
    void not_found(int client);
    void internal_server_error(int client);
    void not_implemented(int client);

    void send_file(int client, FILE *filename);
};

class HttpResonse {
    using string = std::string;
    using responseHeaders = std::unordered_map<std::string, std::string>;

private:
    int statusCode;
    string statusMessage;
    responseHeaders headers; // 响应头
    string body;             // 响应体

    int length; // Content-length

    static std::unordered_map<int, string> codes;

public:
};

#endif /* _HTTP_H */
