#ifndef _HTTP_H_
#define _HTTP_H_

#include "header.h"
#include "http.h"

using std::cout;
using std::endl;
using std::string;
using std::stringstream;

typedef std::map<string, string> http_request_header_line;

struct http_request_header
{
    string method;
    string url;
    string version;

    http_request_header_line header;

    string body;
};

void bad_request(int client);
void not_found(int client);
void internal_server_error(int client);
void not_implemented(int client);
void headers(int client, const char *file);

bool http_request_parse(const string &http_request, http_request_header *client_http);
void print_http_request_header_line(const http_request_header_line &head);
string get_value_from_http_request_header(const string &key, const http_request_header_line &header);
void print_http_request_header(http_request_header *client_http);

#endif
