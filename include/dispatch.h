#pragma once
#include "channel.h"
#include <stdbool.h>
#include "eventloop.h"
struct EventLoop;
struct Dispather
{
    //init 初始化epoll （epoll event） poll:pollfd select:fdset其中一个,返回值是epoll poll select中的一个
    void* (*init)();
    //add 访问抽象好的fd   抽象好的fdset，epoll event等数据
    int (*add)(struct Channel* ch,struct EventLoop* evloop);
    //delete
    int (*remove)(struct Channel* ch,struct EventLoop* evloop);
    //修改
    int (*modify)(struct Channel* ch,struct EventLoop* evloop);
    //事件检测
    int (*dispatch)(struct EventLoop* evloop,int timeout);
    //清除数据  关闭fd 释放内存
    int (*clear)(struct EventLoop* evloop);
};
