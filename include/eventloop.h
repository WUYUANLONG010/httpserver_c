#progma once
#include "dispatch.h"

extern struct dispather epoll_dispather;

struct eventloop
{
    disaptcher* dpt;
    void* dpt_data;
}