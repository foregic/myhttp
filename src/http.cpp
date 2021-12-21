/*
 * @Author       : foregic
 * @Date         : 2021-08-28 11:11:22
 * @LastEditors  : foregic
 * @LastEditTime : 2021-12-21 19:48:20
 * @FilePath     : /httpserver/src/http.cpp
 * @Description  :
 */

#include "http.h"

std::unordered_map<int, std::string> HttpResonse::codes = {
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

Httpimpl::Httpimpl(const string &str) {
    http_request_parse(str);
}

/**
 * @description  : 打印http的内容
 * @param         {*}
 * @return        {*}
 */
void Httpimpl::print() {
    printf("%s %s %s\n", method.c_str(), url.c_str(), version.c_str());
    for (auto &h : header) {
        printf("%s: %s\n", h.first.c_str(), h.second.c_str());
    }
    printf("\n%s\n", body.c_str());
}

/**
 * @description  :传入字符串，解析出对应的url
 * @param         {string} &http_request
 * @return        {*}
 */
void Httpimpl::http_request_parse(const string &http_request) {
    if (http_request.size() == 0) {
        // TODO 修改异常处理
        perror("http_request_parse: http_request is empty");
        // return false;
    }

    string crlf("\r\n"), crlfcrlf("\r\n\r\n");
    size_t prev = 0, next = 0; //当前解析指针

    //解析http请求报文的起始行
    if ((next = http_request.find(crlf, prev)) != string::npos) //找到第一个\r\n的位置
    {
        string first_line(http_request.substr(prev, next - prev));
        prev = next; //移动指针到第一行末尾
        stringstream sstream(first_line);
        sstream >> this->method;
        sstream >> this->url;
        sstream >> this->version;
    } else {
        perror("http_parser: http_request has not a \\r\\n");
        // return false;
    }

    //查找"\r\n\r\n"的位置
    size_t pos_crlfcrlf = http_request.find(crlfcrlf, prev);
    if (pos_crlfcrlf == string::npos) {
        perror("http_request: http_request has not a \"\r\n\r\n\"");
        // return false;
    }

    //解析首部行
    string buff, key, value;
    while (1) {
        next = http_request.find(crlf, prev + 2);

        //如果找到的next不超过"\r\n\r\n"的位置
        if (next <= pos_crlfcrlf) {
            // buff保存了一行
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
            this->header.insert(std::make_pair(key, value));
            prev = next;
        } else {
            break;
        }
    }
    //获取http请求包的实体值
    this->body = http_request.substr(pos_crlfcrlf + 4, http_request.size() - pos_crlfcrlf - 4);
}

/**
 * @description  : 传入key获得对应首部值
 * @param         {string} &key
 * @return        {*}
 */
std::string Httpimpl::getHeaderLine(const string &key) const {
    auto iter = header.find(key);

    if (iter == header.end()) {
        return ""; // 如果请求头内没有该字段
    }
    return iter->second;
}

void Httpimpl::response(int fd) {

    if (this->method == "GET") {
        if (this->url == "/") {
            if (this->url.size() == 1) {
                headers(fd, "resources/http/index.html");
            }
            not_found(fd);
            return;

        } else {
            string tmp("resources/http");
            tmp += this->url;
            // cout << tmp << endl;
            // printf("finding data\n");
            if (!file_exist(tmp)) {
                fprintf(stderr, "%s file not found\n", this->url);
                // perror("file not found\n");
                not_found(fd);
                return;
            }
            // printf("sending data\n");
            headers(fd, tmp.c_str());
        }
    } else if (this->method == "POST") {
        if (this->url == "/post.html") {
            int prev = 0, pos_equal = 0, pos_and = 0;
            requestHeaders post_data;
            int length = this->body.size();
            while (pos_and < length - 1) {
                while (pos_and < length && this->body[pos_and] != '&') {
                    pos_and++;
                }
                ++pos_and;
                while (this->body[pos_equal] != '=') {
                    ++pos_equal;
                }
                ++pos_equal;
                string key = this->body.substr(prev, pos_equal - prev - 1);
                string value = this->body.substr(pos_equal, pos_and - pos_equal - 1);
                prev = pos_and;
                post_data[key] = decode(value);
            }
            std::cout << "打印post数据包" << std::endl;
            for (auto begin = post_data.begin(); begin != post_data.end(); begin++) {
                std::cout << begin->first << ":" << begin->second << std::endl;
            }

            post_response(post_data, fd);
        } else {
            not_found(fd);
        }
    } else {
        not_implemented(fd);
    }
}
#include <fstream>
//发送HTTP头，解析成功，给客户端返回指定文件
void Httpimpl::headers(int client, const char *file) {
    // printf("200 OK 发送文件\n");
    char buf[1024];

    memset(buf, 0, sizeof(buf));
    strcpy(buf, "HTTP/1.1 200 OK\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);

    // printf("open file %s and will send\n", file);

    // std::fstream ifile(file, std::ios::binary | std::ios::in);
    // sendFile(ifile, client);
    // ifile.close();
    FILE *filename = fopen(file, "r");

    send_file(client, filename);
    fclose(filename);

    // printf("had sent file\n");

    strcpy(buf, "\r\n\r\n");
    send(client, buf, strlen(buf), 0);
}

//发送浏览器发送的post请求体的内容
void Httpimpl::post_response(requestHeaders dict, int client) {
    printf("发送post数据");
    char buf[1024];
    strcpy(buf, "HTTP/1.1 200 OK\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);

    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);

    sprintf(buf,
            "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>POST</title><style> .container{width:60%;margin:10% auto 0;background-color:#f0f0f0;padding:2% 5%;border-radius:10px}ul{padding-left:20px;}ul li{line-height:2.3}a{color:#20a53a}</style></head><body><div class=\"container\"><h1>POSTDATA</h1><ul>\r\n");
    send(client, buf, strlen(buf), 0);

    memset(buf, 0, sizeof(buf));
    for (auto begin = dict.begin(); begin != dict.end(); ++begin) {
        sprintf(buf, "<li>%s=%s</li>\n", begin->first.c_str(), begin->second.c_str());
        // sprintf(buf, ("<li>" + begin->first + "=" + begin->second + "</li>").data());
        send(client, buf, strlen(buf), 0);
        printf("%s\n", buf);
    }

    sprintf(buf, "</ul></div></body></html>");
    send(client, buf, strlen(buf), 0);
    printf("%s\n", buf);

    sprintf(buf, "\r\n\r\n");
    send(client, buf, strlen(buf), 0);
    printf("%s\n", buf);
}

//发送400，客户端请求的语法错误，服务器无法理解
void Httpimpl::bad_request(int client) {
    char buf[1024];
    sprintf(buf, "HTTP/1.1 400 BAD REQUEST\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "Content-type: text/html\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, sizeof(buf), 0);

    FILE *filename = fopen("resources/http/400.html", "r");
    send_file(client, filename);
    fclose(filename);
    sprintf(buf, "\r\n\r\n");
    send(client, buf, strlen(buf), 0);
}

//发送404，服务器无法根据客户端请求找到资源
void Httpimpl::not_found(int client) {
    char buf[1024];
    sprintf(buf, "HTTP/1.1 404 NOT FOUND\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);

    //  sprintf(buf, "<h1>404 Not Found</h1>\r\n");
    // send(client, buf, strlen(buf), 0);
    FILE *filename = fopen("resources/http/404.html", "r");
    send_file(client, filename);
    fclose(filename);
    sprintf(buf, "\r\n\r\n");
    send(client, buf, strlen(buf), 0);
}

//发送500，服务器内部错误，无法完成请求
void Httpimpl::internal_server_error(int client) {
    char buf[1024];
    //发送500
    sprintf(buf, "HTTP/1.1 500 Internal Server Error\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);

    FILE *filename = fopen("resources/http/500.html", "r");
    send_file(client, filename);
    fclose(filename);
    sprintf(buf, "\r\n\r\n");
    send(client, buf, strlen(buf), 0);
}

//发送501，服务器不支持请求的功能，无法完成请求
void Httpimpl::not_implemented(int client) {
    char buf[1024];
    sprintf(buf, "HTTP/1.1 501 Not Implemented\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n\r\n");
    send(client, buf, strlen(buf), 0);

    FILE *filename = fopen("resources/http/501.html", "r");
    send_file(client, filename);
    fclose(filename);
    sprintf(buf, "\r\n\r\n");
    send(client, buf, strlen(buf), 0);
}

void Httpimpl::send_file(int client, FILE *filename) {
    if (!filename) {
        fprintf(stderr, "open file %s failed : %s\n", this->url, strerror(errno));
        not_found(client);
        return;
    }
    //发送文件的内容
    char buf[1024];
    while (!feof(filename)) {
        if (fgets(buf, sizeof(buf), filename) != NULL) {
            send(client, buf, strlen(buf), 0);
        }
    }
}