#pragma once
#include "eventloop.h"
#include "theadpool.h"
struct Listener{
    int listen_fd;
    unsigned short port;
};
struct TcpServer{
    int threadnum;
    struct EventLoop* mainloop;
    struct TreadPool* thread_pool;
    struct Listener* listen;

};
//初始化
struct TcpServer* TcpServer_init(unsigned short port,int threadnum);
//初始化监听
struct Listener* Listener_init(unsigned short port);
//启动服务器
// void Listener_run(struct TcpServer *server);
void TCP_Server_run(struct TcpServer *server);
int acceptconnection(void *argv);