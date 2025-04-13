#include "buffer.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "sys/uio.h"

struct Buffer *buffer_init(int size) {
    struct Buffer *buff = (struct Buffer *)malloc(sizeof(struct Buffer));
    if (buff != NULL) {
        // 为data初始化size大小内存
        buff->data     = (char *)malloc(size); //
        buff->writepos = 0;
        buff->readpos  = 0;
        buff->capacity = size;
        memset(buff->data, 0, size);
    }
    return buff;
}
void buffer_destroy(struct Buffer *buff) {
    if (buff != NULL) {
        if (buff->data != NULL) {
            free(buff->data);
        }
        free(buff);
    }
}
// 得到剩余可写内存容量
int buffer_writable(struct Buffer *buff) { return buff->capacity - buff->writepos; }
// 得到剩余可读内存容量
int buffer_readable(struct Buffer *buff) { return buff->writepos - buff->readpos; }
int buffer_extend(struct Buffer *buff, int size) {
    // 1.内存够用，不扩容
    if (buffer_readable(buff) >= size) {
        return 0;
    }
    if (buff->readpos + buffer_writable(buff) >= size) { // 移动 不扩容
        int readable = buffer_readable(buff);
        memcpy(buff->data, buff->data + buff->readpos, readable);
        buff->readpos  = 0;
        buff->writepos = readable;
    } else {
        void *tmp = realloc(buff->data, buff->capacity + size);
        if (tmp == NULL) {
            return -1;
        }
        memset(tmp + buff->capacity, 0, size);
        buff->data = tmp;
        buff->capacity += size;
    }
}
int buffer_append_data(struct Buffer *buff, const char *data, int size) {
    if (buff == NULL || data == NULL || &data <= 0) {
        return -1;
    }
    // int size = strlen(data);
    // 判断空间是否可以写入这么多数据
    buffer_extend(buff, size);
    // 数据拷贝
    memcpy(buff->data + buff->writepos, data, size);
    buff->writepos += size;
    return 0;
}

int buffer_append_string(struct Buffer *buff, const char *data) {
    if (buff == NULL || data == NULL || &data <= 0) {
        return -1;
    }
    int size = strlen(data);
    // 判断空间是否可以写入这么多数据
    buffer_extend(buff, size);
    // 数据拷贝
    memcpy(buff->data + buff->writepos, data, size);
    buff->writepos += size;
    return 0;
}

int buffer_socket_read(struct Buffer *buff, int fd) {
    // read recv readv
    struct iovec vec[2];
    // 初始化数组元素
    int writeable   = buffer_writable(buff);
    vec[0].iov_base = buff->data + buff->writepos;
    vec[0].iov_len  = writeable;
    char *tempbuff  = (char *)malloc(40960); // 4MB缓存  申请了内存
    vec[1].iov_base = tempbuff;
    vec[1].iov_len  = 40960;
    int result      = readv(fd, vec, 2);
    if (result == -1) {
        return -1;
    } else if (result <= writeable) { // 数据全部写入到buff中，不需要处理
        buff->writepos += result;
    } else { // 有部分数据在vec[1]中
        buff->writepos = buff->capacity;
        buffer_append_data(buff, tempbuff, result - writeable);
    }
    free(tempbuff);
    return 0;
}
char *buffer_find_CRLF(struct Buffer *buff) {
    // strstr() 从大字符串中匹配小字符串\\遇到\0直接结束
    // memmem-->大数据块中匹配小数据块
    // void *memmem(const void haystack[.haystacklen], size_t haystacklen,
    //                 const void needle[.needlelen], size_t needlelen);
    return memmem(buff->data + buff->readpos, buffer_readable(buff), "\r\n", 2);
}