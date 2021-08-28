#ifndef _HTTP_H_
#define _HTTP_H_

#include "header.h"
void bad_request(int &client);
void not_found(int &client);
void internal_server_error(int &client);
void not_implemented(int &client);

#endif
