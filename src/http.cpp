/*
 * @Author       : foregic
 * @Date         : 2021-08-28 11:11:22
 * @LastEditors  : foregic
 * @LastEditTime : 2022-01-01 23:15:25
 * @FilePath     : /httpserver/src/http.cpp
 * @Description  :
 */

#include "http.h"

HttpRequest::HttpRequest(const string &str, Server *server_):request(str),server(server_) {
    // printf("begin parser\n");

    std::cout << str << std::endl;
    http_request_parse(str);
}

/**
 * @description  : 打印http的内容
 * @param         {*}
 * @return        {*}
 */
void HttpRequest::print() {
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
bool HttpRequest::http_request_parse(const string &http_request) {
    if (http_request.size() == 0) {
        // TODO 修改异常处理
        perror("http_request_parse: http_request is empty");
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
        sstream >> this->method;
        sstream >> this->url;
        sstream >> this->version;
    } else {
        perror("http_parser: http_request has not a \\r\\n");
        return false;
    }

    //查找"\r\n\r\n"的位置
    size_t pos_crlfcrlf = http_request.find(crlfcrlf, prev);
    if (pos_crlfcrlf == string::npos) {
        perror("http_request: http_request has not a \"\r\n\r\n\"");
        return false;
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
    return true;
}

/**
 * @description  : 传入key获得对应首部值
 * @param         {string} &key
 * @return        {*}
 */
std::string HttpRequest::getHeaderLine(const string &key) const {
    auto iter = header.find(key);

    if (iter == header.end()) {
        return ""; // 如果请求头内没有该字段
    }
    return iter->second;
}

void HttpRequest::response(int fd) {
    // printf("begin response\n");

    if (this->method == "GET") {
        if (this->url == "/") {
            if (this->url.size() == 1) {
                headers(fd, "resources/http/index.html");
            } else {
                not_found(fd);
            }

        } else {

            string tmp("resources/http");
            tmp += this->url;
            // cout << tmp << endl;
            // printf("finding data\n");
            if (file_not_exist(tmp)) {
                fprintf(stderr, "%s file not found\n", this->url.data());
                // perror("file not found\n");
                not_found(fd);
                return;
            }
            // printf("sending data\n");
            headers(fd, tmp.c_str());
        }
    } else if (this->method == "POST") {
        printf("%s\n", this->url.data());
        auto ret = Api::api.find(this->url);
        if (ret != Api::api.end()) {
            char buf[1024];
            strcpy(buf, "HTTP/1.1 200 OK\r\n");
            send(fd, buf, strlen(buf), 0);
            strcpy(buf, SERVER_STRING);
            send(fd, buf, strlen(buf), 0);
            sprintf(buf, "Content-Type: text/html\r\n");
            send(fd, buf, strlen(buf), 0);

            sprintf(buf, "\r\n");
            send(fd, buf, strlen(buf), 0);

            auto response = "{" + ret->second + "}";

            printf("%s\n", response.c_str());
            send(fd, response.c_str(), response.size(), 0);
            close(fd);
            return;
        }
        if (this->url == "/post.html") {
            int prev = 0, pos_equal = 0, pos_and = 0;
            KeyValue post_data;
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
            // std::cout << "打印post数据包" << std::endl;
            // for (auto begin = post_data.begin(); begin != post_data.end(); begin++) {
            //     std::cout << begin->first << ":" << begin->second << std::endl;
            // }

            post_response(post_data, fd);
        } else {
            not_found(fd);
        }
    } else {
        not_implemented(fd);
    }
    close(fd);
}
#include <fstream>
//发送HTTP头，解析成功，给客户端返回指定文件
void HttpRequest::headers(int client, const char *file) {
    FILE *filename = fopen(file, "r");

    if (!filename) {
        not_found(client);
        return;
    }
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

    send_file(client, filename);
    fclose(filename);

    // printf("had sent file\n");

    strcpy(buf, "\r\n\r\n");
    send(client, buf, strlen(buf), 0);
}
//发送浏览器发送的post请求体的内容
void HttpRequest::post_response(KeyValue dict, int client) {
    // printf("发送post数据");
    char buf[1024];
    strcpy(buf, "HTTP/1.1 200 OK\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);

    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);

    string tmp = R"(
        <head>
    <meta charset="utf-8">
    <title>POST DATA</title>
    <style>
        .container {
            width: 60%;
            margin:auto;
            background-color: #f0f0f0;
            padding: 2% 5%;
            border-radius: 10px
        }
        ul {
            padding-left: 20px;
        }
        ul li {
            line-height: 2.3
        }
        a {
            color: #20a53a
        }
    </style>
</head>

<body>
    <div class="container">
    <h1>POST DATA</h1>
    <ul>
        )";

    strcpy(buf, tmp.c_str());
    send(client, buf, strlen(buf), 0);

    memset(buf, 0, sizeof(buf));

    for (auto begin = dict.begin(); begin != dict.end(); ++begin) {
        strcpy(buf, this->script.getPost(begin->first.c_str(), begin->second.c_str()).c_str());

        send(client, buf, strlen(buf), 0);
    }

    sprintf(buf, R"(
        </ul>
        </div>
        </body>
        </html>
    )");
    send(client, buf, strlen(buf), 0);
    // printf("%s\n", buf);

    sprintf(buf, "\r\n\r\n");
    send(client, buf, strlen(buf), 0);
    printf("%s\n", buf);
}

//发送400，客户端请求的语法错误，服务器无法理解
void HttpRequest::bad_request(int client) {
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
void HttpRequest::not_found(int client) {
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
void HttpRequest::internal_server_error(int client) {
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
void HttpRequest::not_implemented(int client) {
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

void HttpRequest::send_file(int client, FILE *filename) {
    if (!filename) {
        fprintf(stderr, "open file %s failed : %s\n", this->url.data(), strerror(errno));
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