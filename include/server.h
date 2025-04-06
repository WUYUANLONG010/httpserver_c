#ifndef SERVER_H 
#define SERVER_H 
#define DEBUG(fmt,...) printf("[%s:func:%s() line:%d] " fmt "\t\n", __FILE__,__func__,__LINE__, ##__VA_ARGS__)
//初始化监听套接字
int init_listen_fd(unsigned int port);
//启动epoll
int epollrun(int fd);
//和服务器建立连接
int acceptClient(int lisfd,int epfd);
//接收http请求
int recvHttpRequest(int cfd,int epfd);
//解析请求行
int parseRequestLine(const char* line,int cfd);
//发送文件
int sendFile(const char* fileName,int cfd);
int sendHeadMsg(int cfd,int status,const char* descr,const char* type,int length);
void decodeMsg(char* to, char* from);
const char* getFileType(const char* name);
int sendDir(const char* dirname,int cfd);
int hexToDec(char c);

#endif
