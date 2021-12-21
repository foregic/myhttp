// #include "header.h"

// int start(u_short &port) {
//     int http_socket = 0, option;
//     http_socket = socket(AF_INET, SOCK_STREAM, 0); //套接字
//     if (http_socket == -1)
//         throw std::invalid_argument("socket建立失败");
//     socklen_t optlen;
//     optlen = sizeof(option);
//     option = 1;
//     setsockopt(http_socket, SOL_SOCKET, SO_REUSEADDR, (void *)&option, optlen);

//     sockaddr_in http_addr;                                                //本机网络地址，ip端口
//     memset(&http_addr, 0, sizeof(http_addr));                             //格式化
//     http_addr.sin_family = AF_INET;                                       //设置协议格式
//     http_addr.sin_port = htons(port);                                     //设置端口号
//     http_addr.sin_addr.s_addr = htonl(INADDR_ANY);                        //设置
//     if (bind(http_socket, (sockaddr *)&http_addr, sizeof(http_addr)) < 0) //绑定套接字
//         throw std::invalid_argument("无效的端口号");

//     listen(http_socket, 5); //等待连接队列的最大长度
//     return http_socket;
// }
// /**
//  * @description  : 发送文件给套接字
//  * @param         {int} client，客户端套接字
//  * @param         {FILE} *filename，文件指针
//  * @return        {*}
//  */
// void send_file(int client, FILE *filename) {
//     //发送文件的内容
//     char buf[1024];
//     while (!feof(filename)) {
//         // printf("%s\n", buf);
//         fgets(buf, sizeof(buf), filename);
//         send(client, buf, strlen(buf), 0);
//     }
// }

// /**
//  * @description  : 相应请求，发送对应文件
//  * @param         {int} fd
//  * @param         {char} *buffer
//  * @return        {*}
//  */
// void response(int fd, char *buffer) {
//     // printf("打印接受到的报文\n%s",buffer);
//     // printf("%s\n", buffer);
//     std::unique_ptr<http_request_header> header;
//     // http_request_header *header = new http_request_header;
//     string request(buffer);
//     // std::cout <<"request: " << request << std::endl;
//     http_request_parse(request, header);
//     // printf("打印解析后的报文\n");
//     // print_http_request_header(header);

//     if (header->method == "GET") {
//         if (header->url == "/") {
//             headers(fd, "resources/http/index.html");
//         } else {
//             string tmp("resources/http");
//             tmp += header->url;
//             // cout << tmp << endl;
//             if (!file_exist(tmp)) {
//                 not_found(fd);
//                 return;
//             }

//             headers(fd, tmp.c_str());
//         }
//     } else if (header->method == "POST") {
//         if (header->url == "/post.html") {
//             int prev = 0, pos_equal = 0, pos_and = 0;
//             key_value post_data;
//             int length = header->body.size();
//             while (pos_and < length - 1) {
//                 while (pos_and < length && header->body[pos_and] != '&') {
//                     pos_and++;
//                 }
//                 ++pos_and;
//                 while (header->body[pos_equal] != '=') {
//                     ++pos_equal;
//                 }
//                 ++pos_equal;
//                 string key = header->body.substr(prev, pos_equal - prev - 1);
//                 string value = header->body.substr(pos_equal, pos_and - pos_equal - 1);
//                 prev = pos_and;
//                 post_data[key] = decode(value);
//             }
//             cout << "打印post数据包" << endl;
//             for (auto begin = post_data.begin(); begin != post_data.end(); begin++) {
//                 cout << begin->first << ":" << begin->second << endl;
//             }

//             post_response(post_data, fd);
//         } else {
//             not_found(fd);
//         }
//     } else {
//         not_implemented(fd);
//     }
// }
