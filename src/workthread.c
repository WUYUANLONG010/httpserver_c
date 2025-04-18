#include "workthread.h"
#include <pthread.h>
#include <stdio.h>
#include "eventloop.h"
#include "server.h"
int work_thread_init(struct WorkThread* work_thread,int index){
    //index用来起名字 workthread是一个内存
    work_thread->evloop=NULL;
    work_thread->threadID=0;
    sprintf(work_thread->name,"Subthread id:%d",index);
    pthread_mutex_init(&work_thread->mutex,NULL);
    pthread_cond_init(&work_thread->cond,NULL);
    DEBUG("work_thread_init success");
    return 0;
}
void* subthread_run(void* arg){
    struct WorkThread* work_thread=(struct WorkThread*)arg;
    pthread_mutex_lock(&work_thread->mutex);
    work_thread->evloop=eventloop_ex_init(work_thread->name);   
    pthread_mutex_unlock(&work_thread->mutex);
    pthread_cond_signal(&work_thread->cond);//向主线程发出初始化成功信号

    eventloop_run(work_thread->evloop); 
    return NULL;
}
int work_thread_run(struct WorkThread* work_thread){//主线程等待子线程创建
    pthread_create(&work_thread->threadID,NULL,subthread_run,work_thread);
    //阻塞主线程，让当前函数不会直接结束
    pthread_mutex_lock(&work_thread->mutex);
    while(work_thread->evloop==NULL){
        pthread_cond_wait(&work_thread->cond,&work_thread->mutex);//等待子线程发出完成初始化信号

    }
    pthread_mutex_unlock(&work_thread->mutex);
}