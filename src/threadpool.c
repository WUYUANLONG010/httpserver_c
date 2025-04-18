#include "eventloop.h"
#include "server.h"
#include "theadpool.h"
#include "workthread.h"
#include <assert.h>
#include <stdlib.h>

struct  TreadPool* thread_pool_init(struct EventLoop* mainloop, int count){
    struct TreadPool *pool = (struct TreadPool *)malloc(sizeof(struct TreadPool));
    DEBUG("thread pool init beging");
    pool->index=0;
    pool->is_start=false;
    pool->mainloop=mainloop;
    pool->thread_num=count;
    pool->work_threads = (struct WorkThread *)malloc(sizeof(struct WorkThread) * count);
    DEBUG("thread pool init end:%p,%p",pool,pool->work_threads);
    return pool;
}

void thread_pool_run(struct TreadPool *pool) {
    DEBUG("lu");
    DEBUG("%lu,%lu",pool->mainloop->thread_ID,pthread_self());
    assert(pool && !pool->is_start);
    DEBUG("%lu,%lu",pool->mainloop->thread_ID,pthread_self());
    if(pool->mainloop->thread_ID!=pthread_self())//只能是主线程启动线程池
    {
        exit(0);
    }
    DEBUG("thread_pool_run 11");
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
    assert(pool->mainloop->thread_ID==pthread_self());
    struct EventLoop* evloop=pool->mainloop;
    if(pool->thread_num>0){
        evloop=pool->work_threads[pool->index].evloop;
        pool->index=++pool->index%pool->thread_num;
 
    }
    return evloop;
    
}