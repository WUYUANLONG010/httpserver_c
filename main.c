#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "server.h"
#include "tcpserver.h"
#include "server.h"
int main(int argc, char* argv[])
{
#if 0
    if (argc < 3)
    { 
        printf("./a.out port path\n");
        return -1;
    }
    unsigned short port = atoi(argv[1]);
    // 切换服务器的工作路径
    chdir(argv[2]);
#else
    unsigned short port = 10000;  
    chdir("/home/wyl/data");  
#endif
    // 启动服务器
    // DEBUG("beging init tcpserver");
    struct TcpServer *server = TcpServer_init(port, 4);
    // DEBUG("beging run tcpserver");
    TCP_Server_run(server);
     

    return 0;

}