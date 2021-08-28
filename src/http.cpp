#include "header.h"


//发送400，客户端请求的语法错误，服务器无法理解
void bad_request(int& client)
{
    char buf[1024];
    sprintf(buf, "HTTP/1.1 400 BAD REQUEST\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "Content-type: text/html\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, sizeof(buf), 0);


    FILE *filename=fopen("../resources/http/400.html","r");
    send_file(client,filename);
    fclose(filename);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
}

//发送404，服务器无法根据客户端请求找到资源
void not_found(int& client)
{
    char buf[1024];
    FILE *filename;

    sprintf(buf, "HTTP/1.1 404 NOT FOUND\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);


    FILE *filename=fopen("../resources/http/500.html","r");
    send_file(client,filename);
    fclose(filename);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);


}

//发送500，服务器内部错误，无法完成请求
void internal_server_error(int& client)
{
    char buf[1024];
    //发送500
    sprintf(buf, "HTTP/1.1 500 Internal Server Error\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);

    FILE *filename=fopen("../resources/http/500.html","r");
    send_file(client,filename);
    fclose(filename);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);



}

//发送501，服务器不支持请求的功能，无法完成请求
void not_implemented(int& client)
{
    char buf[1024];
    sprintf(buf, "HTTP/1.1 501 Not Implemented\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    
    FILE *filename=fopen("../resources/http/501.html","r");
    send_file(client,filename);
    fclose(filename);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);

}