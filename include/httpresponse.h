#pragma once
#include "buffer.h"
#include <stdbool.h>
#define MSG_SEND_AUTO true
//状态码枚举
enum Statu_Code { unKown=0, OK = 200, MOVE_Permenatly = 301, MOVE_Temp = 302, BadRequest = 400, NotFound = 404 };
struct ResponseHeader{
    char key[32];
    char value[128];
};
//定义一个函数指针用来组织回复给客户端的数据块
typedef void (*responseBody)(const char *fileName, struct Buffer *sendBuf, int socket);
struct HttpResponse {
    //状态行:状态码，状态描述
    enum Statu_Code http_status_code;
    char statusMSG[128];
    char filename[128];
    //响应头-键值对
    struct ResponseHeader *header;//运行时指定大小
    int header_len;
    responseBody sendDataFunc;
    
};
struct HttpResponse *HttpResponse_init();
//销毁
void HttpResponse_destroy(struct HttpResponse * response);
//添加响应头
void HttpResponse_Add_Header(struct HttpResponse * response,const char* key,const char* value);
//组织HTTP响应数据
void HttpResponse_prepare_msg(struct HttpResponse *response,struct Buffer* sendbuffer,int socket);