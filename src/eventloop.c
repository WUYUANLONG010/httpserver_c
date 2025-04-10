#include "eventloop.h"
#include "assert.h"
#include <sys/socket.h>
#include "dispatch.h"
struct eventloop* eventloop_init(){
    return eventloop_ex_init(NULL);
}
int read_local_message(void* arg){
    struct eventloop* evloop = (struct eventloop*)arg;
    char buf[256];
    read(evloop->socket_pair[1],buf,sizeof(buf));
    return 0;
}
struct eventloop* eventloop_ex_init(const char* theadName){
    //创建内存
    struct eventloop* evloop = (struct eventloop*)malloc(sizeof(struct eventloop));
    evloop->is_quit=false;
    evloop->thead_ID=pthread_self();
    pthread_mutex_init(&evloop->mutex,NULL);
    strcopy(evloop->theadName,theadName==NULL?"main":threadName)
    evloop->dispatcher=&epoll_dispather;
    evloop->dpt_data=evloop->disaptcher->init();//epoll dispather初始化函数
    evloop->head=NULL;
    evloop->tail=NULL;
    int ret = socketpair(AF_UNIX,SOCK_STREAM,0,evloop->socket_pair);

    if(ret ==-1){
        DEBUG("socket pair create fail");
    }
    struct channel* ch=channel_init(evloop->socket_pair[1],read_event,read_local_message,NULL,evloop);
    event_add_task(evloop,ch,ADD);
    return evloop;
}

int  eventloop_run(struct eventloop* evloop){
    assert(evloop!=NULL);
    //启动epoll模型
    //比较线程ID是否正常
    if(evloop->threadID!=pthread_self()){
        return -1;
    }
    struct disaptcher* dispather=evloop->dpt;
    while(!ev->is_quit){
        dispather->dispatch(evloop,2);//超时时长2s
        eventloop_process_task(evloop);
    }
    return 0;
}

int event_activate(struct eventloop* evloop,int fd,int event){
    if(fd<0|evloop==NULL){
        return -1;
    }
    //取出channel  寻找对应的处理方法
    struct channel* ch=evloop->channelmap->list[fd];
    assert(fd==channel->fd);
    if(event& read_event&& channel->read_call_back){
        channel->read_call_back(channel->arg);
    }
    if(event&write_event && channel->write_call_back){
        channel->write_call_back(channel->arg);
    }
    return 0;
}
void task_wake_up(struct eventloop* evloop){
    const char* msg="1";
    write(evloop->socket_pair[0],msg,strlen(msg));
}
int event_add_task(struct eventloop* evloop,struct channel* ch,int type){
    //任务队列加锁，防止子线程访问
    pthread_mutex_lock(&evloop->mutex);
    //创建新的节点
    struct channel_element* node=(struct channel_element*)malloc(sizeof(struct channel_element));
    node->channel=ch;
    node->type=type;
    node->next=NULL;
    if(evloop->head==NULL){
        evloop->head=evloop->tail=node;
    }else{
        evloop->tail->next=node;//添加节点
        evloop->tail=node;//尾节点后移
    }
    pthread_mutex_unlock(&evloop->mutex);
    //处理节点
    /*
    细节:
        1.对于链表节点的添加:可能是当前线程也可能是其他线程(主线程)
            1)，修改fa的事件，当前子线程发起，当前子线程处理
            2)，添加新的fd，添加任务节点的操作是由主线程
        2.不能让主线程处理任务队列，需要由当前的子线程取处理
    */
   if(evLoop->threadID == pthread_self()){
    //当前子线程给自己添加文件描述符
        eventloop_process_task(evloop);
   }else{
       //主线程给子线程添加文件描述符  1.子线程在工作 2.子线程由于epoll等处于阻塞状态  超时时间2s，不可能等阻塞完再给他添加文件描述符
       //因此做一个操作，如何主动让子线程结束阻塞状态  1.给子线程添加一个channel，不属于连接，专属于用于结束阻塞状态，
       //在插入任务的同时主动给channel一个读事件
       //实现方式 1.pipe通信  2.socket pair通信
       task_wake_up(evloop);


   }


//主线程
    return 0;
}
//任务队列处理任务
int eventloop_process_task(struct eventloop* evloop){
    pthread_mutex_lock(&evloop->mutex);
    //取出头节点
    struct channel_element* head =evloop->head;
    while(head!=NULL){
        struct channel* ch=head->channel;
        if(head->type==ADD){
            //添加节点
            eventloop_add(evloop,ch);
        }else if(head->type==DELETE){
            //删除
            eventloop_del(evloop,ch);
        }else if(head->type==MODIFY){
            //修改
            eventloop_mod(evloop,ch);
        }
        struct channel_element* tmp=head;
        head=head->next;
        free(tmp);
    }
    evloop->head=evloop->tail=NULL;
    
    pthread_mutex_unlock(&evloop->mutex);
    return 0;
}

int eventloop_add(struct eventloop* evloop,struct channel* ch){
    int fd = channel->fd;
    struct channelmap* ch_map=evloop->channelmap;
    if(fd>=ch_map->size){
        if(!makeMapRoom(channelmap,fd,sizeof(struct channel*))){
            return -1;
        }
    }
    //找到fd对应的数组元素位置并存储
    if(ch_map->list[fd]==NULL){
        ch_map->list[fd]=ch;
        evloop->dispatch->add(ch,evloop);
    }
    return 0;
}
int eventloop_del(struct eventloop* evloop,struct channel* ch){
    int fd = channel->fd;
    struct channelmap* ch_map=evloop->channelmap;
    if(fd>=ch_map->size){//fd不在dispatch检测集合里面
            return -1;
    }
    //找到fd对应的数组元素位置并存储
    int ret = evloop->dispatcher->remove(ch,evloop);
    return ret;
}
int eventloop_mod(struct eventloop* evloop,struct channel* ch){
    int fd = channel->fd;
    struct channelmap* ch_map=evloop->channelmap;
    if(fd>=ch_map->size){//fd不在dispatch检测集合里面
            return -1;
    }
    //找到fd对应的数组元素位置并存储
    int ret = evloop->dispatcher->modify(ch,evloop);
    return ret;
}

int destroychannel(struct eventloop* evloop,struct channel* ch){
    evloop->ch_map->list[channel-fd]=NULL;//channel map 删除ch
    //关闭fd
    close(channel->fd);
    //释放channel的地址
    free(channel);
    return 0;
}