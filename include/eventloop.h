#pragma once
#include "channelmap.h"
#include "dispatch.h"
#include "pthread.h"
#include "stdbool.h"
extern struct Dispather epoll_dispather;

extern struct Dispather poll_dispather;
extern struct Dispather select_dispather;
// 定义任务队列的节点
enum elem_type { ADD, DELETE, MODIFY };
struct ChannelElement {
    int type; // epoll poll select
    struct Channel *ch;
    struct ChannelElement *next;
};
struct Dispather;
struct EventLoop {
    bool is_quit;
    struct Dispather *dpt;
    void *dpt_data;
    struct ChannelElement *head; // 头节点
    struct ChannelElement *tail; // 尾节点
    // map
    struct ChannelMap *ch_map;
    // 线程ID name mutex
    pthread_t thread_ID;
    char threadName[32];
    pthread_mutex_t mutex;
    int socket_pair[2]; // 存储自己和子线程的
};
// 初始化反应堆内存
struct EventLoop *eventloop_init();
struct EventLoop *eventloop_ex_init(const char *theadName);
// 启动反应堆模型
int eventloop_run(struct EventLoop *evloop);
// 处理激活的文件描述符
int event_activate(struct EventLoop *evloop, int fd, int event);
// 添加eventloop任务
int event_add_task(struct EventLoop *evloop, struct Channel *ch, int type);
// 任务处理函数
int eventloop_process_task(struct EventLoop *evloop);
// 任务节点添加到dispath检测集合
int eventloop_add(struct EventLoop *evloop, struct Channel *ch);
int eventloop_del(struct EventLoop *evloop, struct Channel *ch);
int eventloop_mod(struct EventLoop *evloop, struct Channel *ch);
int destroychannel(struct EventLoop *evloop, struct Channel *ch);