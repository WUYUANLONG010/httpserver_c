#include "dispatch.h"
#include <poll.h>
#include <sys/epoll.h>
#include "eventloop.h"
#include "server.h"
#include "channel.h"

#define MAX_POLL 1024
struct PollData{
    int maxfd;//poll查询数组的最高有效位的索引，即要遍历的文件描述符的数量
    struct pollfd fds[MAX_POLL];
};
//init 初始化epoll （epoll event） poll:pollfd select:fdset其中一个,返回值是epoll poll select中的一个
static void* poll_init();
static int poll_add(struct Channel* ch,struct EventLoop* evloop);
static int poll_remove(struct Channel* ch,struct EventLoop* evloop);
static int poll_modify(struct Channel* ch,struct EventLoop* evloop);
static int poll_dispatch(struct EventLoop* evloop,int timeout);
static int poll_clear(struct EventLoop* evloop);
// static int pollctl(struct Channel* ch,struct EventLoop* evloop,int option);
 
struct Dispather poll_dispather={
    poll_init,
    poll_add,
    poll_remove,
    poll_modify,
    poll_dispatch,
    poll_clear
};
// static int pollctl(struct Channel* ch,struct EventLoop* evloop,int option){
//     struct poll_data* data=(struct PollData*)evloop->dpt_data;
//     struct poll_event ev;
//     int events=0;
//     if(ch->event&read_event){//是读事件
//         events|=POLLIN;
//     }
//     if(ch->event&write_event){
//         events|=POLLOUT;
//     }
//     ev.events=events;
//     int ret=epoll_ctl(data->epfd,option,ch->fd,&ev);
//     return ret;
// }

static void* poll_init(){
    // struct PollData* data=(struct PollData*)evloop->dpt_data;
    struct PollData* data=(struct PollData*)malloc(sizeof(struct PollData));
    data->maxfd=0;
    for(int i=0;i<MAX_POLL;i++){
        data->fds[i].fd=-1;
        data->fds[i].events=0;
        data->fds[i].revents=0;
    }
    return data;
}
static int poll_add(struct Channel* ch,struct EventLoop* evloop){
    struct PollData* data=(struct PollData*)evloop->dpt_data;
    int events=0;
    if(ch->event&read_event){//是读事件
        events|=POLLIN;
    }
    if(ch->event&write_event){
        events|=POLLOUT;
    }
    int i=0;
    for(i=0;i<MAX_POLL;i++){
        if(data->fds[i].fd==-1){
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
}
static int poll_remove(struct Channel* ch,struct EventLoop* evloop){
    struct PollData* data=(struct PollData*)evloop->dpt_data;//取地址+强制类型转换格式化
    int events=0;
    if(ch->event&read_event){//是读事件
        events|=POLLIN;
    }
    if(ch->event&write_event){
        events|=POLLOUT;
    }
    int i=0;
    for(i=0;i<MAX_POLL;i++){
        if(data->fds[i].fd==ch->fd){
            //找到空闲的数组索引
            data->fds[i].events=0;
            data->fds[i].revents=0;
            data->fds[i].fd=-1;
            break;
        }
        //超过max，不做操作
    }
    ch->destroy_call_back(ch->arg);
    if (i >= MAX_POLL) {
        return -1;
    }
}
static int poll_modify(struct Channel* ch,struct EventLoop* evloop){
    struct PollData* data=(struct PollData*)evloop->dpt_data;//取地址+强制类型转换格式化
    int events=0;
    if(ch->event&read_event){//是读事件
        events|=POLLIN;
    }
    if(ch->event&write_event){
        events|=POLLOUT;
    }
    int i;
    for(i=0;i<MAX_POLL;i++){
        if(data->fds[i].fd==ch->fd){
            //找到空闲的数组索引
            data->fds[i].events=0;
            break;//插入成功，跳出循环
        }
        //超过max，不做操作
    }
    if(i>=MAX_POLL){
        return -1;
    }
}
static int poll_dispatch(struct EventLoop* evloop,int timeout){
    struct PollData* data=(struct PollData*)evloop->dpt_data;
    int count=poll(data->fds,data->maxfd+1,timeout*1000);
    if(count==-1){
        DEBUG("poll fail ");
        exit(0);
    }
    for(int i=0;i<data->maxfd;i++){
        if(data->fds[i].fd==-1){
            continue;
        }

        if(data->fds[i].revents&POLLIN){
            //读事件
            event_activate(evloop,data->fds[i].fd,read_event);
        }
        if(data->fds[i].revents&POLLOUT){
            //写事件
            event_activate(evloop,data->fds[i].fd,write_event);
        }

    }
    return 0;
    
}
static int poll_clear(struct EventLoop* evloop){
    struct PollData* data=(struct PollData*)evloop->dpt_data;
    free(data);
}