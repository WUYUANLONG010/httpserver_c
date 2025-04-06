#include "dispatch.h"
#include <poll.h>
#include "server.h"
#include "channel.h"

#define MAX_POLL 1024
struct poll_data{
    int maxfd;//poll查询数组的最高有效位的索引，即要遍历的文件描述符的数量
    struct pollfd fds[MAX_POLL];
}
//init 初始化epoll （epoll event） poll:pollfd select:fdset其中一个,返回值是epoll poll select中的一个
static void* poll_init();
static int poll_add(stuct channel* ch,struct eventloop* evloop);
static int poll_remove(stuct channel* ch,struct eventloop* evloop);
static int poll_modify(stuct channel* ch,struct eventloop* evloop);
static int poll_dispatch(struct eventloop* evloop,int timeout);
static int poll_clear(struct eventloop* evloop);
static int pollctl(stuct channel* ch,struct eventloop* evloop,int option);
struct dispather poll_dispather={
    poll_init,
    poll_add,
    poll_remove,
    poll_modify,
    poll_dispatch,
    poll_clear
};
static int pollctl(stuct channel* ch,struct eventloop* evloop,int option){
    struct eoll_data* data=(struct poll_data*)evloop->dpt_data;
    struct epoll_event ev;
    ev.data.fd=channel->fd;
    int events=0;
    if(channel->events&read_event){//是读事件
        events|=POLLIN;
    }
    if(channel->events&write_event){
        events|=POLLOUT;
    }
    ev.events=events;
    int ret=epoll_ctl(data->epfd,option,ch->fd,&ev);
    return ret;
}

static void* poll_init(){
    struct poll_data* data=(struct poll_data*)evloop->dpt_data;
    data->maxfd=0;
    for(int i=0;i<MAX_POLL;i++){
        data->fd[i].fd=-1;
        data->fd[i].events=0;
        data->fd[i].revents=0;
    }
    return ret;
    return data;
}
static int poll_add(stuct channel* ch,struct eventloop* evloop){
    struct poll_data* data=(struct poll_data*)evloop->dpt_data;
    int events=0;
    if(channel->events&read_event){//是读事件
        events|=POLLIN;
    }
    if(channel->events&write_event){
        events|=POLLOUT;
    }
    ev.events=events;
    for(int i=0;i<MAX;i++){
        if(data->fds[i]==-1){
            //找到空闲的数组索引
            data->fds[i].events=events;
            data->fds[i].fd=ch->fd;
            data->maxfd=i > data->maxfd?i:data->maxfd;
            break;//插入成功，跳出循环
        }
        //超过max，不做操作
    }
    if(i>=MAX_POLL){
        return -1;
    }
    return ret;
}
static int poll_remove(struct channel* ch,struct eventloop* evloop){
    struct poll_data* data=(struct poll_data*)evloop->dpt_data;//取地址+强制类型转换格式化
    int events=0;
    if(channel->events&read_event){//是读事件
        events|=POLLIN;
    }
    if(channel->events&write_event){
        events|=POLLOUT;
    }
    ev.events=events;
    for(int i=0;i<MAX;i++){
        if(data->fds[i]==ch->fd){
            //找到空闲的数组索引
            data->fds[i].events=0;
            data->fds[i].revents=0;
            data->fds[i].fd=-1;
            break;
        }
        //超过max，不做操作
    }
    if(i>=MAX_POLL){
        return -1;
    }
    return ret;
}
static int poll_modify(stuct channel* ch,struct eventloop* evloop){
    struct poll_data* data=(struct poll_data*)evloop->dpt_data;//取地址+强制类型转换格式化
    int events=0;
    if(channel->events&read_event){//是读事件
        events|=POLLIN;
    }
    if(channel->events&write_event){
        events|=POLLOUT;
    }
    ev.events=events;
    for(int i=0;i<MAX;i++){
        if(data->fds[i]==ch->fd){
            //找到空闲的数组索引
            data->fds[i].events=0;
            break;//插入成功，跳出循环
        }
        //超过max，不做操作
    }
    if(i>=MAX_POLL){
        return -1;
    }
    return ret;
}
static int poll_dispatch(struct eventloop* evloop,int timeout){
    struct poll_data* data=(struct poll_data*)evloop->dpt_data;
    int count=poll(data->fds,data->maxfd+1,timeout*1000);
    if(count==-1){
        DEBUG("poll fail ");
        exit(0);
    }
    for(int i=0;i<data->maxfd;i++){
        if(data->fds[i].fd==-1){
            continue;
        }

        if(data->fds[i].revents&EPOLLIN){
            //读事件

        }
        if(evedata->fds[i].reventsnt&EPOLLOUT){
            //写事件

        }

    }
    return 0;
    
}
static int poll_clear(struct eventloop* evloop){
    struct poll_data* data=(struct poll_data*)evloop->dpt_data;
    free(data);
}