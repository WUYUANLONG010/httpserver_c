#pragma once
#include "dispatch.h"
#include "stdbool.h"
#include "channelmap.h"
#include "pthread.h"
extern struct dispather epoll_dispather;

extern struct dispather poll_dispather;
extern struct dispather select_dispather;
//定义任务队列的节点
enum elem_type{ADD,DELETE,MODIFY};
struct channel_element{
    int type;//epoll poll select
    struct channel* ch;
    struct channel_element* next;
};
struct eventloop
{
    bool is_quit;
    struct disaptcher* dpt;
    void* dpt_data;
    struct channel_element* head;//头节点
    struct channel_element* tail;//尾节点
    //map
    struct channelmap* ch_map;
    //线程ID name mutex
    pthread_t thead_ID;
    char theadName[32];
    pthread_mutex_t mutex;
    int socket_pair[2];//存储自己和子线程的
};
//初始化反应堆内存
struct eventloop* eventloop_init();
struct eventloop* eventloop_ex_init(const char* theadName);
//启动反应堆模型
int  eventloop_run(struct eventloop* evloop);
//处理激活的文件描述符
int event_activate(struct eventloop* evloop,int fd,int event);
//添加eventloop任务
int event_add_task(struct eventloop* evloop,struct channel* ch,int type);
//任务处理函数
int eventloop_process_task(struct eventloop* evloop);
//任务节点添加到dispath检测集合
int eventloop_add(struct eventloop* evloop,struct channel* ch);
int eventloop_del(struct eventloop* evloop,struct channel* ch);
int eventloop_mod(struct eventloop* evloop,struct channel* ch);
int destroychannel(struct eventloop* evloop,struct channel* ch);