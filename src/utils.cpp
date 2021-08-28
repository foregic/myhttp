#include"header.h"

void send_file(int client, FILE *filename)
{
    //发送文件的内容
    char buf[1024];
    while (!feof(filename))
    {
        fgets(buf, sizeof(buf), filename);
        send(client, buf, strlen(buf), 0);
        // fgets(buf, sizeof(buf), filename);
    }
}


