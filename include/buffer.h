
#pragma once
#include "eventloop.h"
#include <stddef.h>

struct Buffer {
    // 指向内存的指针
    char *data;
    int capacity;
    int readpos;
    int writepos;
};
struct Buffer *buffer_init(int size);
void buffer_destroy(struct Buffer *buff);
void buffer_write(struct Buffer *buff);
int buffer_extend(struct Buffer *buff, int size);
int buffer_readable(struct Buffer *buff);
int buffer_writable(struct Buffer *buff);
// 向内存写数据
int buffer_append_data(struct Buffer *buff, const char *data, int size);
int buffer_append_string(struct Buffer *buff, const char *data);

// 接收套接字数据
int buffer_socket_read(struct Buffer *buff, int fd);
// 根据/r/n来取出一行，找到其在buffer中的位置
char *buffer_find_CRLF(struct Buffer *buff);
