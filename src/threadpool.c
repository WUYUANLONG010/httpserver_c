#include "eventloop.h"
#include "theadpool.h"
#include "workthread.h"
#include <assert.h>
#include <stdlib.h>

struct  TreadPool* thread_pool_init(struct EventLoop* mainloop, int count){
    struct  TreadPool* pool=(struct  TreadPool*)malloc(sizeof(struct  TreadPool));
    pool->index=0;
    pool->is_start=false;
    pool->mainloop=mainloop;
    pool->thread_num=count;
    pool->work_threads=(struct work_thread*)malloc(sizeof(struct WorkThread) * count);
    return pool;
}

void thread_pool_run(struct  TreadPool* pool){
    assert(pool&&!pool->is_start);
    if(pool->mainloop->thread_ID!=pthread_self())//只能是主线程启动线程池
    {
        exit(0);
    }

    pool->is_start=true;
    if(pool->thread_num){
        for(int i=0;i<pool->thread_num;i++){
            work_thread_init(&pool->work_threads[i],i);
            work_thread_run(&pool->work_threads[i]);
        }
    }
}

struct EventLoop* take_work_eventloop(struct  TreadPool* pool){
    //主线程取子线程的eventloop
    assert(pool->is_start);
    assert(pool->mainloop->thread_ID!=pthread_self());
    struct EventLoop* evloop=pool->mainloop;
    if(pool->thread_num>0){
        evloop=pool->work_threads[pool->index].evloop;
        pool->index=++pool->index%pool->thread_num;
 
    }
    return evloop;
    
}