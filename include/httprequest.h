#pragma once
#include "buffer.h"
#include <stdbool.h>
#include "httpresponse.h"
#define MSG_SEND_AUTO true
struct RequsetHeader {
  char *key;
  char *value;
};
enum HttpRequse_status {
  ParseHttpLine,
  ParseHttpHeader,
  ParseHttpBody,
  ParseHttpDown
};
struct HttpRequest {
  char *method;
  char *url;
  char *version;
  struct RequsetHeader *reqHeaders; // 一个数组头
  int RequsetHeader_len;//数组长度
  enum HttpRequse_status curstatus;
};
struct HttpRequest *HttpRequest_init();
int HttpRequest_reset(struct HttpRequest * httprequest);
int HttpRequest_resetex(struct HttpRequest *httprequest);
int HttpRequest_destroy(struct HttpRequest *httprequest);
// RequsetHeader insert 请求头添加键值对
void Requset_Add_Header(struct HttpRequest * httprequest,const char* key,const char* value);
// RequsetHeader get 请求头通过key 获取 value
char *Requset_Get_Header(struct HttpRequest *httprequest, const char *key);
// 解析請求行 数据存在struct buffer中
bool Parse_Http_requsetLine(struct HttpRequest *httprequest,struct Buffer* readbuffer);
// 解析請求頭
bool Parse_Http_requsetHeader(struct HttpRequest *httprequest,struct Buffer* readbuffer);
// 解析http请求协议
bool Parse_Http_requset(struct HttpRequest *httprequest,struct Buffer* readbuffer,struct HttpResponse* response,struct Buffer* sendbuff, int socket);
// 处理http协议
bool Process_Http_requset(struct HttpRequest *httprequest,struct HttpResponse* response);
void decodeMsg(char* to, char* from);
const char *getFileType(const char *name);
//发送文件
void sendFile(const char *fileName, struct Buffer *sendBuf, int socket);
void sendDir(const char *fileName, struct Buffer *sendBuf, int socket);
int hexToDec(char c);
