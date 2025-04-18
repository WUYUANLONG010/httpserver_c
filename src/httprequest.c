#include "httprequest.h"
#include "buffer.h"
#include "server.h"

#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include "httpresponse.h"

#define headsize 12
// enum HttpRequse_status {
//   ParseHttpLine,
//   ParseHttpHeader,
//   ParseHttpBody,
//   ParseHttpDown
// };
struct HttpRequest *HttpRequest_init() {
    struct HttpRequest *request = (struct HttpRequest *)malloc(sizeof(struct HttpRequest));
    HttpRequest_reset(request);
    request->reqHeaders        = (struct RequsetHeader *)malloc(sizeof(struct RequsetHeader) * headsize);
    return request;
}
// 初始化函数 可以在init中调用
int HttpRequest_reset(struct HttpRequest *httprequest) {
    httprequest->curstatus = ParseHttpLine;
    httprequest->method    = NULL;
    httprequest->url       = NULL;
    httprequest->version   = NULL;
    // httprequest->reqHeaders =0;
    httprequest->RequsetHeader_len = 0;
    return 0;
}
// 内存释放函数，该函数不能在init里面调用
int HttpRequest_resetex(struct HttpRequest *httprequest) {
    free(httprequest->url);
    free(httprequest->method);
    free(httprequest->version);

    if (httprequest->reqHeaders != NULL) {
        int i = 0;
        for (i = 0; i < httprequest->RequsetHeader_len; i++) {
            free(httprequest->reqHeaders[i].key);
            free(httprequest->reqHeaders[i].value);
        }
        free(httprequest->reqHeaders);
    }
    return HttpRequest_reset(httprequest);
}
int HttpRequest_destroy(struct HttpRequest *httprequest) {
    if (httprequest != NULL) {
        HttpRequest_resetex(httprequest);
        free(httprequest);
    }
    return 0;
}
enum HttpRequse_status get_httprequest_status(struct HttpRequest *httprequest) { return httprequest->curstatus; }
// 没有进行内存申请
void Requset_Add_Header(struct HttpRequest *httprequest, const char *key, const char *value) {
    httprequest->reqHeaders[httprequest->RequsetHeader_len].key   = (char*)key; // 指向一块堆内存
    httprequest->reqHeaders[httprequest->RequsetHeader_len].value = (char*)value;
    httprequest->RequsetHeader_len++;
}
char *Requset_Get_Header(struct HttpRequest *httprequest, const char *key) {
    if (httprequest != NULL) {
        int i = 0;
        for (i = 0; i < httprequest->RequsetHeader_len; i++) { // 每一轮取出一个key value
            if (strncasecmp( httprequest->reqHeaders[i].key,key,
                            strlen(key))==0) { // 比较不区分大小写，比较前几个字符
                                            // 找到了key值
                return httprequest->reqHeaders[i].value;
            }
        }
    }
    return NULL;
}
char* split_request_line(const char *startpos, const char *endpos, const char *sub,char** ptr) {
    char* space=endpos;
    if (sub!=NULL) {
        space = memmem(startpos, endpos - startpos, sub, strlen(sub)); // 寻找空格
        assert(space != NULL);
    }
 
    int length = space - startpos;
    char* tmp = (char *)malloc(length + 1);
    strncpy(tmp, startpos, length);
    tmp[length] = '\0';
    *ptr        = tmp;
    return space+1;
}
bool Parse_Http_requsetLine(struct HttpRequest *httprequest, struct Buffer *readbuffer) {
    // 读出请求行 找到第一行 \r\n
    //
    // 保存字符串起始地址
    // 保存字符串结束地址
    DEBUG("Parse_Http_requsetLine beging");
    char *endpos   = buffer_find_CRLF(readbuffer);
    char *startpos = readbuffer->data + readbuffer->readpos;
    // 请求行总长度
    int linesize = endpos - startpos;
    if (linesize) {
        // get /xxx/xx/txt http/1.1
        // 请求方式
        // char *space = memmem(startpos, linesize, " ", 1); // 寻找空格
        // assert(space != NULL);
        // int mothod_size = space - startpos;
        // httprequest->method = (char *)malloc(mothod_size + 1);
        // strncpy(httprequest->method, startpos, mothod_size);
        // httprequest->method[mothod_size] = '\0';
        startpos = split_request_line(startpos, endpos, " ", &httprequest->method);//请求第一行方法
        startpos = split_request_line(startpos, endpos, " ", &httprequest->url);//请求第二行 静态资源
        startpos = split_request_line(startpos, endpos, NULL, &httprequest->version); // 请求 http版本号
                                                                                      //          // 为解析请求头做准备
        readbuffer->readpos += linesize;
        readbuffer->readpos += 2;
        // 修改状态
        httprequest->curstatus = ParseHttpHeader;
        DEBUG("curstatus=  ParseHttpHeader");
        return true;
        // // 请求行(第一行)解析完毕
        // //请求第二行静态资源
        // startpos = space + 1;
        // char *space = memmem(startpos, endpos-startpos, " ", 1); // 寻找空格
        // assert(space != NULL);
        // int url_size = space - startpos;
        // httprequest->url = (char *)malloc(url_size + 1);
        // strncpy(httprequest->url, startpos, url_size);
        // httprequest->url[url_size] = '\0';
        // // 请求第三行 http版本
        // startpos = space + 1;
        // // char *space = memmem(startpos, endpos-startpos, " ", 1); // 寻找空格
        // // assert(space != NULL);
        // // int url_size = space - startpos;
        // int http_size=endpos-startpos;
        // httprequest->version = (char *)malloc(http_size + 1);
        // strncpy(httprequest->url, startpos, http_size);
        // httprequest->url[http_size] = '\0';
        // // 为解析请求头做准备 读指针后移 \r\n
        // readbuffer->readpos += (linesize + 2);
        // httprequest->curstatus=ParseHttpHeader;
        // return true;
    }
    return false;
}

bool Parse_Http_requsetHeader(struct HttpRequest *httprequest, struct Buffer *readbuffer) {
    char *end = buffer_find_CRLF(readbuffer);
    if (end != NULL) {
        char *start = readbuffer->data + readbuffer->readpos;
        int linesize = end - start;
        char *mid    = memmem(start, linesize, ": ", 2);
        if (mid != NULL) {
            char *key = malloc(mid - start + 1);//后面补充一个\0
            strncpy(key, start, mid - start);
            key[mid - start] = '\0';

            char *value = malloc(end-mid-2+1);//后面补充一个\0
            strncpy(value, mid+2, end-mid-2);
            value[end-mid-2] = '\0';

            Requset_Add_Header(httprequest, key, value);
            // 移动指针到下一行的
            readbuffer->readpos += linesize;
            readbuffer->readpos += 2;
        } else {
            // 有可能请求头解析完进入到第三部分了，无法搜索到 :
            // 将指针跳过当前的空行
            readbuffer->readpos += 2;
            // 修改解析状态 如果是post，则解析第四部分
            httprequest->curstatus=ParseHttpDown;
            DEBUG("curstatus=  ParseHttpDown");
        }
        return true;
    } else {
        DEBUG("error end = NULL");
    }
    
    return false;
}
bool Parse_Http_requset(struct HttpRequest *httprequest, struct Buffer *readbuffer, struct HttpResponse *response, struct Buffer *sendbuff, int socket) {
    DEBUG("Parse_Http_requset");
    bool flag = true;
    while (httprequest->curstatus!=ParseHttpDown) {
        switch (httprequest->curstatus)
        {
        case ParseHttpLine:
            flag=Parse_Http_requsetLine(httprequest,readbuffer);
            break;
        case ParseHttpHeader:
            flag=Parse_Http_requsetHeader(httprequest,readbuffer);
            break;
        case ParseHttpBody:
            // HttpResponse_prepare_msg
            break;        
        default:
            break;
        }
        if (!flag) {
            return flag;
        }
        // 判断是否解析完成  解析完成回复数据
        if (httprequest->curstatus == ParseHttpDown) {
            // 1.解析出原始数据
            Process_Http_requset(httprequest,response);
            // 2.组织响应数据发送给客户端
            DEBUG("组织响应数据");
            HttpResponse_prepare_msg(response, sendbuff, socket);
        }
    }
    DEBUG("还原状态");
    httprequest->curstatus =ParseHttpLine;//还原状态，以备处理第二条请求
    return flag;
}

bool Process_Http_requset(struct HttpRequest *httprequest,struct HttpResponse* response) {
    // 处理基于GET的http请求
      //解析请求行 方法和路径  get /xxx/1.jpg http/1.1


    // 判断是get还是post
    DEBUG("beging process http request");
    if(strcasecmp(httprequest->method,"get")!=0){
        return -1;
    }
    decodeMsg(httprequest->url, httprequest->url);
     //处理客户端请求的静态资源
    char* file=NULL;
    if(strcmp(httprequest->url,"/")==0){
        file="./";
    }else{
        file=httprequest->url+1;
    }
    printf("%s",httprequest->url);
    //获取文件属性
    struct stat st;
    int ret=stat(file,&st);

    if(ret==-1){
        //文件不存在
        // sendHeadMsg(cfd,404,"Not Found",getFileType(".html"),-1);
        // sendFile("404.html",cfd);
        strcpy(response->filename, "404.html");
        response->http_status_code = NotFound;
        strcpy(response->statusMSG, "NotFound");
        //响应头
        HttpResponse_Add_Header(response, "Content-type",getFileType(".html"));
        response->sendDataFunc = sendFile;
        return 0;
    }
    strcpy(response->filename, file);
    response->http_status_code = OK;
    strcpy(response->statusMSG, "OK");
    //判断文件类型
    if(S_ISDIR(st.st_mode)){
        //是目录，返回目录内容

        //响应头
        HttpResponse_Add_Header(response, "Content-type",getFileType(".html"));
        response->sendDataFunc = sendDir;
        // return 0;
    }else{//发送文件
        //响应头
        char tmp[12]={0};
        sprintf(tmp,"%ld", st.st_size);
        HttpResponse_Add_Header(response, "Content-type", getFileType(file));
        HttpResponse_Add_Header(response, "Content-length",tmp);
        response->sendDataFunc = sendFile;
        // return true;
    }
   
    // return 0;
    return false;
}
const char* getFileType(const char* name)
{
    // a.jpg a.mp4 a.html
    // 自右向左查找‘.’字符, 如不存在返回NULL
    const char* dot = strrchr(name, '.');
    if (dot == NULL)
        return "text/plain; charset=utf-8";	// 纯文本
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
        return "text/html; charset=utf-8";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(dot, ".gif") == 0)
        return "image/gif";
    if (strcmp(dot, ".png") == 0)
        return "image/png";
    if (strcmp(dot, ".css") == 0)
        return "text/css";
    if (strcmp(dot, ".au") == 0)
        return "audio/basic";
    if (strcmp(dot, ".wav") == 0)
        return "audio/wav";
    if (strcmp(dot, ".avi") == 0)
        return "video/x-msvideo";
    if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
        return "video/quicktime";
    if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
        return "video/mpeg";
    if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
        return "model/vrml";
    if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
        return "audio/midi";
    if (strcmp(dot, ".mp3") == 0)
        return "audio/mpeg";
    if (strcmp(dot, ".ogg") == 0)
        return "application/ogg";
    if (strcmp(dot, ".pac") == 0)
        return "application/x-ns-proxy-autoconfig";

    return "text/plain; charset=utf-8";

}
void decodeMsg(char* to, char* from)
{
    for (; *from != '\0'; ++to, ++from)
    {
        // isxdigit -> 判断字符是不是16进制格式, 取值在 0-f
        // Linux%E5%86%85%E6%A0%B8.jpg
        if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
        {
            // 将16进制的数 -> 十进制 将这个数值赋值给了字符 int -> char
            // B2 == 178
            // 将3个字符, 变成了一个字符, 这个字符就是原始数据
            *to = hexToDec(from[1]) * 16 + hexToDec(from[2]);

            // 跳过 from[1] 和 from[2] 因此在当前循环中已经处理过了
            from += 2;
        }
        else
        {
            // 字符拷贝, 赋值
            *to = *from;
        }
    
    }
    *to = '\0';

}
void sendFile(const char *fileName, struct Buffer *sendBuf, int socket) {
    DEBUG("开始发送文件");
    int fd = open(fileName, O_RDONLY);
    assert(fd > 0);
    while (1) {
        char buf[1024];
        int len=read(fd,buf, sizeof(buf));
        if(len>0){
            // send(cfd,buf,len,0);
            buffer_append_data(sendBuf, buf,len);
#ifndef MSG_SEND_AUTO
            buffer_socket_send(sendBuf, socket);
#endif
            // usleep(10);
        } else if (len == 0) {
            DEBUG("文件发送完毕");
            break;
            //文件读完
        }else{
            close(fd);
            DEBUG("DURING READ ERROR");
        }
    }
    DEBUG("文件发送完毕");
    close(fd);
}
void sendDir(const char *fileName, struct Buffer *sendBuf, int socket) {
    DEBUG("sendDir beging");
    struct dirent** namelist;
    char buff[4096]={0};
    sprintf(buff,"<html><head><title>%s</title></head><body><table>",fileName);
    int num =scandir(fileName,&namelist,NULL,alphasort);
    for(int i=0;i<num;i++){
        char* name=namelist[i]->d_name;
        struct stat st;
        
        char subpath[1024]={0};

        sprintf(subpath,"%s/%s",fileName,name);
        stat(subpath,&st);
        if(S_ISDIR(st.st_mode)){
            //添加跳转html标签<a href="">name</a>
            sprintf(buff+strlen(buff),"<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",name,name,st.st_size);
        }else{
            sprintf(buff+strlen(buff),"<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",name,name,st.st_size);
        }
        // send(socket,buff,sizeof(buff),0);
        buffer_append_string(sendBuf, buff);
#ifndef MSG_SEND_AUTO
        buffer_socket_send(sendBuf, socket);
#endif
        memset(buff, 0, sizeof(buff));
        free(namelist[i]);
    }
    sprintf(buff,"</table></body></html>");
    // send(socket,buff,sizeof(buff),0);
    buffer_append_string(sendBuf, buff);
#ifndef MSG_SEND_AUTO
    buffer_socket_send(sendBuf, socket);
#endif
    free(namelist);
    DEBUG("sendDir end");
    // return 0;
}
int hexToDec(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    return 0;

}