#pragma once
#include "httprequest.h"
#include "httpresponse.h"
#include "stdbool.h"
#include "eventloop.h"
#include "buffer.h"
#include "channel.h"

struct TcpConnection {
  struct EventLoop *evloop;
  struct Channel *channel;
  struct Buffer *write_buffer;
  struct Buffer *read_buffer;
  char name[32];
  struct HttpRequest *httprequset;
  struct HttpResponse *httpresponse;
};
struct TcpConnection* tcp_connection_init(int fd,struct EventLoop* evloop);
int tcp_connection_destroy(void *arg);