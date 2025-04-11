#ifndef CHANNEL_H 
#define CHANNEL_H
#include <stdbool.h>
typedef int(*handleFunc)(void * arg); 
struct channel{
    //文件描述符
    int fd;
    //事件
    int event;
    //回调函数
    handleFunc read_call_back;
    handleFunc write_call_back;
    void* arg;//回调函数参数
};
//文件描述符的读写事件
enum fdevent
{
    time_out=0x01,
    read_event=0x02,
    write_event=0x04
};
struct channel* channel_init(int fd,int event,handleFunc read,handleFunc write,void* args);
//修改fd的写事件
void write_event_enable(struct channel* channel,bool flag);
bool is_write_event_enable(struct channel* channel);
#endif
