#pragma once
#include "eventloop.h"
#include "workthread.h"
//定义线程池
struct TreadPool{
    struct eventloop* mainloop;
    bool is_start;
    int thread_num;
    struct work_thread* work_threads;
    int index;//下一个插入的索引
}
//初始化线程池
struct  TreadPool* thread_pool_init(struct eventloop* mainloop, int count);
void thread_pool_run(struct  TreadPool* pool);

struct eventloop* take_work_eventloop(struct  TreadPool* pool);