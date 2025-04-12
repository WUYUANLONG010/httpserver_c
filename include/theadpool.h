#pragma once
#include "eventloop.h"
#include "workthread.h"
//定义线程池
struct TreadPool{
    struct EventLoop* mainloop;
    bool is_start;
    int thread_num;
    struct WorkThread* work_threads;
    int index;//下一个插入的索引
};
//初始化线程池
struct  TreadPool* thread_pool_init(struct EventLoop* mainloop, int count);
void thread_pool_run(struct  TreadPool* pool);

struct EventLoop* take_work_eventloop(struct  TreadPool* pool);