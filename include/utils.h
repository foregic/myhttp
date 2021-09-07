#ifndef _UTILS_H_
#define _UTILS_H_

#include "header.h"
#include <bits/stat.h>
using std::string;

void send_file(int client, FILE *filename);
int start(u_short &port);
void response(int fd, char *buffer);
string decode(const string &str);


inline bool file_exist(const std::string &name)
{
    return (access(name.c_str(), F_OK) != -1);
}

#endif