#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <stdbool.h>

bool http_server_start(const char *port, int poolTimeoutMs);

#endif  // HTTP_SERVER_H