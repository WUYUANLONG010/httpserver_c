#include "tcpserver.h"
#include <netinet/in.h>
#include <stdlib.h>
#include "channel.h"

struct TcpServer* TcpServer_init(unsigned short port,int threadnum){
    struct TcpServer* tcp=(struct TcpServer*)malloc(sizeof(struct TcpServer));
    tcp->listen=;
    tcp->mainloop=eventloop_init();
    tcp->thread_num=threadnum;
    tcp->thread_pool=thread_pool_init(tcp->mainloop,threadnum);

}

struct Listener* Listener_init(unsigned short port){
    struct Listener* listener=(struct Listener*)malloc(sizeof(struct Listener));
    int lfd=socket(AF_INET,SOCK_STREAM,0);
    if(lfd==-1){
        DEBUG("lfd craete fail");
        return -1;
    }
    int opt=1;
    int ret = setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    if(ret==-1){
        DEBUG("socket reuse set fail");
        return -1;
    }
    struct sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    addr.sin_addr.s_addr=INADDR_ANY;//本机所有ip地址
    ret=bind(lfd,(struct sockaddr_in*)&addr,sizeof(addr));
    if(ret==-1){
        DEBUG("bind set fail");
        return -1;
    }
    ret=listen(lfd,128);
        if(ret==-1){
        DEBUG("listend fail");
        return -1;
    }
    listener->port=port;
    listener->listen_fd=lfd;
    return listener;
    
}

void Listener_run(struct TcpServer* server){
    //启动线程池
    thread_pool_run(server->thread_pool);
    //初始化一个channel
    struct channel* ch= channel_init(server->listen->listen_fd,read_event,acceptconnection,NULL,server);
    //添加监听描述符到eventloop任务循环
    event_add_task(server->mainloop,channel,ADD);
    //启动反应堆模型
    eventloop_run(server->mainloop);

}
int acceptconnection(void* argv){
    struct TcpServer* server = (struct TcpServer*)argv;
    int cfd=accept(server->listen->listen_fd,NULL,NULL);
    //从线程池中取出一个子线程反应堆模型，将
    take_work_eventloop(server->thread_pool);
    return cfd;
}