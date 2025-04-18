#include "httpresponse.h"
#include "stdio.h"
#include "string.h"
#include <stdlib.h>

#define ResHeaderSize 16 // 默认header数量

struct HttpResponse *HttpResponse_init() {
    struct HttpResponse *response = (struct HttpResponse *)malloc(sizeof(struct HttpResponse));
    int size                      = sizeof(struct ResponseHeader) * ResHeaderSize;
    response->header_len          = 0;
    response->header              = (struct ResponseHeader *)malloc(size);
    response->http_status_code    = unKown;
    response->sendDataFunc        = NULL;
    bzero(response->header, size);
    bzero(response->statusMSG, sizeof(response->statusMSG));
    bzero(response->filename, sizeof(response->filename));
    return response;
}
void HttpResponse_destroy(struct HttpResponse * response){
    if(response!=NULL){
        free(response->header);
        free(response);
        
    }
}
void HttpResponse_Add_Header(struct HttpResponse * response,const char* key,const char* value){
    if(response==NULL||key==NULL||value==NULL){
        return;
        }
        strcpy(response->header[response->header_len].key , key);
        strcpy(response->header[response->header_len].value, value);
        response->header_len++;

}
void HttpResponse_prepare_msg(struct HttpResponse *response, struct Buffer *sendbuffer, int socket) {
    //状态行
    char tmp[1024] = {0};
    sprintf(tmp, "HTTP/1.1 %d %s\r\n", response->http_status_code, response->statusMSG);
    buffer_append_string(sendbuffer, tmp);
    //响应头
    int i;
    for (i = 0; i < response->header_len;i++){
        sprintf(tmp, "%s: %s\r\n", response->header[i].key, response->header[i].value);
        buffer_append_string(sendbuffer, tmp);
    }
    //空行
    sprintf(tmp, "\r\n");
    buffer_append_string(sendbuffer, tmp);
#ifdef MSG_SEND_AUTO
    buffer_socket_send(sendbuffer, socket);
#endif
    //回复的数据
    response->sendDataFunc(response->filename, sendbuffer, socket);
}