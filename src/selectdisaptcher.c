#include "dispatch.h"
#include <sys\select.h>
#include "server.h"
#include "channel.h"

#define MAX_SELECT 1024
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
struct disaptcher select_dispather={
    select_init,
    select_add,
    select_remove,
    select_modify,
    select_dispatch,
    select_clear 
};
static int setfdset(struct channel* ch,struct select_data* data);
static int clearfdset(struct channel* ch,struct select_data* data);
static void setfdset(struct channel* ch,struct select_data* data){
    if(ch->events&read_event){//是读事件
        FD_SET(ch->fd,&data->readset);

    }
    if(ch->events&write_event){
        FD_SET(ch->fd,&data->writeset);
    }
}
static void clearfdset(struct channel* ch,struct select_data* data){
    if(ch->events&read_event){//是读事件
        FD_CLR(ch->fd,&data->readset);
    }
    if(ch->events&write_event){
        FD_CLR(ch->fd,&data->writeset);
    }
}
static void* select_init(){
    // struct select_data* data=(struct select_data*)evloop->dpt_data;
    struct select_data* data=(struct select_data*)malloc(sizeof(select_data));//初始化一片内存空间
    FD_ZERO(&data->readset);
    FD_ZERO(&data->writeset);
    return data;
}
static int select_add(stuct channel* ch,struct eventloop* evloop){
    struct select_data* data=(struct select_data*)evloop->dpt_data;//取数据
    if(channel->fd>=MAX_SELECT){
        return -1;//检测的fd大于1024个,直接退出增加函数
    }
    setfdset(ch,data);
    return 0;
}
static int select_remove(struct channel* ch,struct eventloop* evloop){
    struct select_data* data=(struct select_data*)evloop->dpt_data;//取数据//取地址+强制类型转换格式化
    clearfdset(ch,data);
    return 0;
}
static int select_modify(stuct channel* ch,struct eventloop* evloop){
    struct select_data* data=(struct select_data*)evloop->dpt_data;//取地址+强制类型转换格式化
    //有可能是往写集合中添加或删除，也有可能是往读集合中添加或删除
    setfdset(ch,data);
    clearfdset(ch,data); 
}
static int select_dispatch(struct eventloop* evloop,int timeout){
    struct select_data* data=(struct select_data*)evloop->dpt_data;
    struct timeval val;
    val.tv_sec=timeout;
    val.tv_usec=0;
    val.tv_nsec=0;
    fd_set rdtmp=data->readset;
    fd_set wrtmp=data->writeset;
    int count=select(MAX_SELECT,rdtmp,wrtmp,NULL,&val);
    if(count==-1){
        DEBUG("select fail ");
        exit(0);
    }
    //分别遍历读集合和写集合
    for(int i=0;i<MAX_SELECT;i++){ 
        if(FD_ISSET(i,&rdtmp)){//读集合中有人激活
            event_activate(evloop,fd,read_event);
        }
        if(FD_ISSET(i,&wrtmp)){
            //写集合中有人激活
            event_activate(evloop,fd,write_event);
        }
    }
    return 0;
    
}
static int select_clear(struct eventloop* evloop){
    struct select_data* data=(struct select_data*)evloop->dpt_data;
    free(data);
}