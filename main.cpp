#include <iostream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "include/server.h"
#define PORT 1234
//绑定的端口+资源目录
int main(int argc,char* argv[]) {
    printf("%d",argc);
    if(argc<3){
        printf("./a.out port path");
        exit(0);
    }
    unsigned short port=atoi(argv[1]);
    //切换服务器的工作路径到指定目录
    chdir(argv[2]);
    unsigned int server_port=port;
    int lfd=init_listen_fd(server_port);
    int ret=epollrun(lfd);
    return 0;
}