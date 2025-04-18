#include "channel.h"
#include <stdlib.h>
#include "server.h"
struct Channel *channel_init(int fd, int event, handleFunc read, handleFunc write,handleFunc destroy_call_back, void *args) {
    struct Channel *channel = (struct Channel *)malloc(sizeof(struct Channel));
    DEBUG("channel_init1 ");
    channel->arg            = args;
    channel->fd             = fd;
    channel->event          = event;
    channel->read_call_back = read;
    channel->destroy_call_back = destroy_call_back;
    channel->write_call_back   = write;
    DEBUG("channel_init end");
    return channel;
}
void write_event_enable(struct Channel *channel, bool flag) {
    DEBUG("事件写入检测使能");
    if (flag) {
        channel->event |= write_event;
    } else {
        channel->event = channel->event & ~write_event;
    }
}
bool is_write_event_enable(struct Channel *channel) { return channel->event & write_event; }