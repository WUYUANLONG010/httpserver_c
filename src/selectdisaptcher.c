#include "dispatch.h"
#include <sys\select.h>
#include "server.h"
#include "channel.h"

#define MAX_POLL 1024
struct select_data{
    fd_set readset;
    fd_set writeset;
}
//init 初始化epoll （epoll event） poll:pollfd select:fdset其中一个,返回值是epoll poll select中的一个
static void* select_init();
static int select_add(stuct channel* ch,struct eventloop* evloop);
static int select_remove(stuct channel* ch,struct eventloop* evloop);
static int select_modify(stuct channel* ch,struct eventloop* evloop);
static int select_dispatch(struct eventloop* evloop,int timeout);
static int select_clear(struct eventloop* evloop);
struct dispather poll_dispather={
    poll_init,
    poll_add,
    poll_remove,
    poll_modify,
    poll_dispatch,
    poll_clear
};


static void* poll_init(){
    struct poll_data* data=(struct poll_data*)evloop->dpt_data;

    FD_ZERO(&data->readset);
    FD_ZERO(&data->writeset);
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