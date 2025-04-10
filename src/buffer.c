#include "buffer.h"
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdlib.H>
#include "stdio.h"
#include "sys/uio.h"

struct Buffer* buffer_init(int size){
    struct Buffer* buff = (struct Buffer*)malloc(sizeof(struct Buffer)); 
    if(buff!=NULL){
        //为data初始化size大小内存
        buffer->data=(char*)malloc(size);//
        buffer->writepos=0;
        buffer->readpos=0;
        buffer->capacity=size;
        memset(buffer->data,0,size);
    }
    return buff;
}
void  buffer_destroy(struct Buffer* buff){
    if(buff!=NULL){
         if(buff->data!=NULL){
             free(buff->data);
         }
         free(buff);
    }
}
//得到剩余可写内存容量
int buffer_writable(struct Buffer* buff){
    return buff->capacity-buff->writepos;
}
//得到剩余可读内存容量
int buffer_readable(struct Buffer* buff){
    return buff->writepos-buff->readpos;
}
int buffer_extend(struct Buffer* buff,int size){
    //1.内存够用，不扩容
    if(buffer_readable(buff)>=size){
        return 0;
    }
    if(buff->readpos+buffer_writable(buff)>=size)(//移动 不扩容
        int readable=buffer_readable(buff);
        memcopy(buff->data,buff->data+buffer->readpos,readable);
        buffer->readpos=0;
        buffer->writepos=readable;
    )else{
        void* tmp=realloc(buffer->data,buffer->capacity+size);
        if(tmp==NULL){
            return -1;
        }
        memset(tmp+buff->capacity,0,size);
        buff->data=tmp;
        buff->capacity+=size;

    }
}
int buffer_append_data(struct Buffer* buff,const char* data,int size){
    if(buff==NULL||data==NULL||data<=0){
        return -1;
    }
    // int size = strlen(data);
    //判断空间是否可以写入这么多数据
    buffer_extend(buffer,size);
    //数据拷贝
    memcpy(buff->data+buff->writepos,data,size);
    buff->writepos+=size;
    return 0;
}

int buffer_append_string(struct Buffer* buff,const char* data){
    if(buff==NULL||data==NULL||data<=0){
        return -1;
    }
    int size = strlen(data);
    //判断空间是否可以写入这么多数据
    buffer_extend(buffer,size);
    //数据拷贝
    memcpy(buff->data+buff->writepos,data,size);
    buff->writepos+=size;
    return 0;
}

int buffer_socket_read(struct Buffer* buff,int fd){
    //read recv readv
    struct iovec vec[2];
    //初始化数组元素
    int writeable = buffer_writable(buff);
    vec[0].iov_base=buffer->data+buffer->writepos;
    vec[0].iov_len=writeable;
    char* tempbuff=(char*)malloc(40960);//4MB缓存
    vec[1].iov_base=tempbuff;
    vec[1].iov_len=40960;
    int result =readv(fd,vec,2);
    if(result==-1){
        return -1;
    }else if(result<=writeable){//数据全部写入到buff中，不需要处理
        buffer-writepos+=result;
    }else{
        buffer_append_data(buff,tempbuff,result-writeable);
    }
    return 0;
}1