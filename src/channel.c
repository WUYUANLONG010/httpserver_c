#include "channel.h"


struct channel* channel_init(int fd,int event,handleFunc read,handleFunc write,void* args){
    struct channel* channel=(struct channel*)malloc(sizeof(struct channel));
    channel->arg=args;
    channel->fd=fd;
    channel->event=event;
    channel->read_call_back=read;

    channel->write_call_back=write;
    return channel; 
    
}
void write_event_enable(struct channel* channel,bool flag){
    if (flag){
        channel->event|=write_event;
    }else{
        channel->event=channel->event&~write_event;
    }
}
bool is_write_event_enable(struct channel* channel){
    return channel->event&write_event;
}