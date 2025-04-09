#pragma once
#include "pthread.h"


struct work_thread{
    pthread_t threadID;
    char name[24];
    pthread_mutex_t mutex;//互斥锁
    pthread_cond_t cond;//条件变量
    struct eventloop* evloop;//反应堆模型

}
int work_thread_init(struct work_thread* work_thread,int index);
int work_thread_run(struct work_thread* work_thread);