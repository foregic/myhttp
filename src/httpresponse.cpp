/*
 * @Author       : foregic
 * @Date         : 2022-01-01 02:38:10
 * @LastEditors  : foregic
 * @LastEditTime : 2022-01-02 12:46:44
 * @FilePath     : /httpserver/src/httpresponse.cpp
 * @Description  :
 */
#include "httpresponse.h"

const std::unordered_map<int, std::string> HttpResponse::codes = {
    {200, "OK"},

    {400, "Bad Request"},        // 客户端请求的语法错误，服务器无法理解
    {401, "Unauthorized"},       // 请求要求用户的身份认证
    {403, "Forbidden"},          // 服务器理解请求客户端的请求，但是拒绝执行此请求
    {404, "Not Found"},          // 服务器无法根据客户端的请求找到资源（网页）
    {405, "Method Not Allowed"}, // 客户端请求中的方法被禁止
    {408, "Request Time-out"},   // 服务器等待客户端发送的请求时间过长，超时

    {500, "Internal Server Error"},     // 服务器内部错误，无法完成请求
    {501, "Not Implemented"},           // 服务器不支持请求的功能，无法完成请求
    {502, "Bad Gateway"},               // 作为网关或者代理工作的服务器尝试执行请求时，从远程服务器接收到了一个无效的响应
    {503, "Service Unavailable"},       // 由于超载或系统维护，服务器暂时的无法处理客户端的请求。延时的长度可包含在服务器的Retry-After头信息中
    {504, "Gateway Time-out"},          // 充当网关或代理的服务器，未及时从远端服务器获取请求
    {505, "HTTP Version not supported"} // 服务器不支持请求的HTTP协议的版本，无法完成处理
};
const std::unordered_map<std::string, std::string> HttpResponse::suffix = {
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/nsword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css "},
    {".js", "text/javascript "},
};

void HttpResponse::SetStatus(int status) {
    statusCode = status;
    statusMessage = codes.find(status)->second;
}
void HttpResponse::SetHeader(string key, string value) {
    headers[key] = value;
}

void HttpResponse::SetContentLength() {
    SetHeader("Content-Length", std::to_string(body.size()));
}

void HttpResponse::SetDate() {
}

void HttpResponse::SetContentType(string &str) {
    SetHeader("Content-Type", suffix.find(str)->second);
}
void HttpResponse::SetContentType(const char *str) {
    SetHeader("Content-Type", suffix.find(str)->second);
}

void HttpResponse::SetConnection(bool isAlive) {

    if (isAlive) {
        SetHeader("Connection", "keep-alive");
        SetHeader("keep-alive", "timeout=5");
    } else {
        SetHeader("Connection", "close");
    }
}

void HttpResponse::GetFileType(string &str) {
    auto point = path.find('.');
}

bool HttpResponse::FileIsExist(const std::string &name) {
    std::ifstream f(name.c_str());
    return f.good();
    // return (access(name.c_str(), F_OK) == -1);
}

void HttpResponse::NotFound() {
    std::ifstream ifs(path, std::ios::binary);
    while (ifs >> body) {
    }
    ifs.close();
    SetStatus(404);
    SetContentType(".html");
    SetConnection(false);
}

void HttpResponse::SendResponse(int fd) {
    char buf[BUFFER_SIZE];

    sprintf(buf, "%s %d %s\r\n", version.c_str(), statusCode, statusMessage.c_str());
    send(fd, buf, strlen(buf), 0);

    for (auto &item : headers) {
        sprintf(buf, "%s: %s\r\n", item.first.c_str(), item.second.c_str());
        send(fd, buf, strlen(buf), 0);
    }
    send(fd, "\r\n", 2, 0);
    int size = 0;
    while (size < body.size()) {
        sprintf(buf, "%s", body.substr(size).c_str());
        size += send(fd, buf, strlen(buf), 0);
    }
    send(fd, "\r\n", 2, 0);
}