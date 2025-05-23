#pragma once
#include "pthread.h"
#include "eventloop.h"

struct WorkThread{
    pthread_t threadID;
    char name[24];
    pthread_mutex_t mutex;//互斥锁
    pthread_cond_t cond;//条件变量
    struct EventLoop* evloop;//反应堆模型

};
int work_thread_init(struct WorkThread* work_thread,int index);
int work_thread_run(struct WorkThread* work_thread);