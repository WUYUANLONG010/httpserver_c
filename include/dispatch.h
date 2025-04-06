#progma once
#include "channel.h"
#include <stdbool.h>
#include "eventloop.h"
struct disaptcher
{
    //init 初始化epoll （epoll event） poll:pollfd select:fdset其中一个,返回值是epoll poll select中的一个
    void* (*init)();
    //add 访问抽象好的fd   抽象好的fdset，epoll event等数据
    int (*add)(stuct channel* ch,struct eventloop* evloop);
    //delete
    int (*remove)(stuct channel* ch,struct eventloop* evloop);
    //修改
    int (*modify)(stuct channel* ch,struct eventloop* evloop);
    //事件检测
    int (*dispatch)(struct eventloop* evloop,int timeout);
    //清除数据  关闭fd 释放内存
    int (*clear)(struct eventloop* evloop);
}