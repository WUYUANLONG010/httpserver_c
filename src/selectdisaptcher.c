#include "dispatch.h"
#include <sys/select.h>
#include "eventloop.h"
#include "server.h"
#include "channel.h"

#define MAX_SELECT 1024
struct select_data{
    fd_set readset;
    fd_set writeset;
};
//init 初始化epoll （epoll event） poll:pollfd select:fdset其中一个,返回值是epoll poll select中的一个
static void* select_init();
static int select_add(struct Channel* ch,struct EventLoop* evloop);
static int select_remove(struct Channel* ch,struct EventLoop* evloop);
static int select_modify(struct Channel* ch,struct EventLoop* evloop);
static int select_dispatch(struct EventLoop* evloop,int timeout);
static int select_clear(struct EventLoop* evloop);
struct Dispather select_dispather={
    select_init,
    select_add,
    select_remove,
    select_modify,
    select_dispatch,
    select_clear 
};
static void setfdset(struct Channel* ch,struct select_data* data);
static void clearfdset(struct Channel* ch,struct select_data* data);
static void setfdset(struct Channel* ch,struct select_data* data){
    if(ch->event&read_event){//是读事件
        FD_SET(ch->fd,&data->readset);

    }
    if(ch->event&write_event){
        FD_SET(ch->fd,&data->writeset);
    }
}
static void clearfdset(struct Channel* ch,struct select_data* data){
    if(ch->event&read_event){//是读事件
        FD_CLR(ch->fd,&data->readset);
    }
    if(ch->event&write_event){
        FD_CLR(ch->fd,&data->writeset);
    }
}
static void* select_init(){
    // struct select_data* data=(struct select_data*)evloop->dpt_data;
    struct select_data* data=(struct select_data*)malloc(sizeof(struct select_data));//初始化一片内存空间
    FD_ZERO(&data->readset);
    FD_ZERO(&data->writeset);
    return data;
}
static int select_add(struct Channel* ch,struct EventLoop* evloop){
    struct select_data* data=(struct select_data*)evloop->dpt_data;//取数据
    if(ch->fd>=MAX_SELECT){
        return -1;//检测的fd大于1024个,直接退出增加函数
    }
    setfdset(ch,data);
    return 0;
}
static int select_remove(struct Channel* ch,struct EventLoop* evloop){
    struct select_data* data=(struct select_data*)evloop->dpt_data;//取数据//取地址+强制类型转换格式化
    clearfdset(ch,data);
    return 0;
}
static int select_modify(struct Channel* ch,struct EventLoop* evloop){
    struct select_data* data=(struct select_data*)evloop->dpt_data;//取地址+强制类型转换格式化
    //有可能是往写集合中添加或删除，也有可能是往读集合中添加或删除
    setfdset(ch,data);
    clearfdset(ch,data); 
}
static int select_dispatch(struct EventLoop* evloop,int timeout){
    struct select_data* data=(struct select_data*)evloop->dpt_data;
    struct timeval val;
    val.tv_sec=timeout;
    val.tv_usec=0;
    fd_set rdtmp=data->readset;
    fd_set wrtmp=data->writeset;
    int count=select(MAX_SELECT,&rdtmp,&wrtmp,NULL,&val);
    if(count==-1){
        DEBUG("select fail ");
        exit(0);
    }
    //分别遍历读集合和写集合
    for(int i=0;i<MAX_SELECT;i++){ 
        if(FD_ISSET(i,&rdtmp)){//读集合中有人激活
            event_activate(evloop,i,read_event);
        }
        if(FD_ISSET(i,&wrtmp)){
            //写集合中有人激活
            event_activate(evloop,i,write_event);
        }
    }
    return 0;
    
}
static int select_clear(struct EventLoop* evloop){
    struct select_data* data=(struct select_data*)evloop->dpt_data;
    free(data);
}