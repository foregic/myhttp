#ifndef _HEADER_H_
#define _HEADER_H_

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <unistd.h>

#include <pthread.h>

#include <string>
#include <cstring>
#include <thread>
#include <stdexcept>
#include <iostream>
#include <wait.h>

#include "utils.h"
#include "http.h"

#define SERVER_STRING "Server: myhttp/1.0\r\n"

#endif