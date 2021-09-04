#ifndef _UTILS_H_
#define _UTILS_H_

#include"header.h"

void send_file(int client,FILE*filename);
int start(u_short &port);
void response(int fd,char *buffer);

#endif