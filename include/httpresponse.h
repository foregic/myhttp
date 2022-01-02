#include <cstring>
#include <fstream>
#include <string>
#include <sys/socket.h>
#include <unordered_map>

#define BUFFER_SIZE 1024

class HttpResponse {
    using string = std::string;
    using KeyValue = std::unordered_map<std::string, std::string>;

public:
    HttpResponse(int client_) : client(client_) {}

private:
    int client; // 客户端套接字
    string path;

    int statusCode;       // HTTP status code
    string statusMessage; // HTTP status message
    string version;
    KeyValue headers; // 响应头
    string body;      // 响应体

public:
    void SetStatus(int status);

    void SetHeader(string key, string value); // 设置响应头信息
    void SetContentLength(); 
    void SetDate();
    void SetContentType(string &str);
    void SetContentType(const char *str);
    void SetConnection(bool isAlive);
    bool FileIsExist(const std::string &name);
    void NotFound();
    void SendResponse(int fd);

    void GetFileType(string &str);
public:
    static const std::unordered_map<int, string> codes;
    static const std::unordered_map<string, string> suffix;
};