#include "dispatch.h"
#include <sys/epoll.h>
#include "server.h"
#include "channel.h"

#define MAX 520
struct epoll_data{
    int epfd;
    struct epoll_event* events;
}
//init 初始化epoll （epoll event） poll:pollfd select:fdset其中一个,返回值是epoll poll select中的一个
static void* epoll_init();
static int epoll_add(stuct channel* ch,struct eventloop* evloop);
static int epoll_remove(stuct channel* ch,struct eventloop* evloop);
static int epoll_modify(stuct channel* ch,struct eventloop* evloop);
static int epoll_dispatch(struct eventloop* evloop,int timeout);
static int epoll_clear(struct eventloop* evloop);
static int epollctl(stuct channel* ch,struct eventloop* evloop,int option);
struct dispather epoll_dispather={
    epoll_init,
    epoll_add,
    epoll_remove,
    epoll_modify,
    epoll_dispatch,
    epoll_clear
};
static int epollctl(stuct channel* ch,struct eventloop* evloop,int option){
    struct epoll_data* data=(struct epoll_data*)evloop->dpt_data;
    struct epoll_event ev;
    ev.data.fd=channel->fd;
    int events=0;
    if(channel->events&read_event){//是读事件
        events|=EPOLLIN;
    }
    if(channel->events&write_event){
        events|=EPOLLOUT;
    }
    ev.events=events;
    int ret=epoll_ctl(data->epfd,option,ch->fd,&ev);
    return ret;
}

static void* epoll_init(){
    struct epoll_data* data=(struct epoll_data*)malloc(sizeof(epoll_data));
    data->epfd=epoll_create(10);
    if(data->epfd==-1){
        DEBUG("dispather_epoll create error");
        return -1;
    }else{
        DEBUG("dispather_epoll create success");
    }
    data->events=(struct epoll_event* )calloc(MAX,sizeof(struct epoll_event));
    if(data->events==NULL){
        DEBUG("dispather_epoLL_EVENT create error");
        return -1;
    }else{
        DEBUG("dispather_epoLL_EVENT  create success");
    }
    return data;
}
static int epoll_add(stuct channel* ch,struct eventloop* evloop){
    int ret=epollctl(ch,evloop,EPOLL_CTL_ADD);
    if(ret==-1){
        DEBUG("epoll_add fail!");
    }
    return ret;
}
static int epoll_remove(struct channel* ch,struct eventloop* evloop){
    int ret=epollctl(ch,evloop,EPOLL_CTL_DEL);
    if(ret==-1){
        DEBUG("epoll_add fail!");
    }
    return ret;
    return ret;
}
static int epoll_modify(stuct channel* ch,struct eventloop* evloop){
    int ret=epollctl(ch,evloop,EPOLL_CTL_MOD);
    if(ret==-1){
        DEBUG("epoll_add fail!");
    }
    return ret;
}
static int epoll_dispatch(struct eventloop* evloop,int timeout){
    struct epoll_data* data=(struct epoll_data*)evloop->dpt_data;
    int count=epoll_wait(data->epfd,data->events,MAX,timeout*1000);
    for(int i=0;i<count;i++){
        int event= data->events[i].events;
        int fd=data->events[i].data.fd;
        if(events&EPOLLERR||events&EPOLLHUP){
            //断开连接  删除其fd
            //epoll_remove(,evloop);
            continue;
        }
        if(event&EPOLLIN){
            //读事件
            event_activate(evloop,fd,read_event);
        }
        if(event&EPOLLOUT){
            //写事件
            event_activate(evloop,fd,write_event);
        }

    }
    return 0;
    
}
static int epoll_clear(struct eventloop* evloop){
    struct epoll_data* data=(struct epoll_data*)evloop->dpt_data;
    free(data->events);
    close(daata->epfd);
    free(data);
}