#include "server.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstdio>
#include <sys/epoll.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <sys/sendfile.h>
#include <dirent.h>
#include <malloc.h>
#include <ctype.h>

int init_listen_fd(unsigned int port){
    DEBUG("begin init");
    //1.创建监听fd
    int listen_fd=socket(AF_INET,SOCK_STREAM,0);//ipv4,tcp,0
    if(listen_fd==-1){
        DEBUG("listend socket create error");
        return -1;
    }else{
        DEBUG("server listen socket create success");
    }
    //2.设置端口复用
    int opt=1;
    int ret = setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    if(ret==-1){
        DEBUG("reuse socket error");
        return -1;
    }else{
        DEBUG("reuse listen socket success");
    }

    //3.绑定
    struct sockaddr_in serveraddr;
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_port=htons(port);
    serveraddr.sin_addr.s_addr=INADDR_ANY;
    ret=bind(listen_fd,(struct sockaddr*)&serveraddr,sizeof(serveraddr));
    if(ret==-1){
        DEBUG("bind socket error");
        return -1;
    }else{
        DEBUG("%s: bind listen socket success",__func__);
    }
    //4.设置监听
    ret = listen(listen_fd,128);
    if(ret==-1){
        DEBUG("listen error\t\n");
        return -1;
    }else{
        DEBUG("listend success\t\n");
    }
    //5.返回监听fd
    return listen_fd;
}
int epollrun(int lfd){
    //1.创建epoll实例
    int epoll_fd=epoll_create(1);
    if(epoll_fd==-1){
        DEBUG("epoll fd create error");
        return -1;
    }else{
        DEBUG("epoll fd create success");
    }
    //2.lfd上树
    struct epoll_event ev;
    ev.data.fd=lfd;//listen文件描述符
    ev.events=EPOLLIN;//读事件
    int ret = epoll_ctl(epoll_fd,EPOLL_CTL_ADD,lfd,&ev);
    if(ret==-1){
        DEBUG("epoll init error\t\n");
        return -1;
    }else{
        DEBUG("epoll init success\t\n");
    }
    //3.检测  服务器不断检测listend接收事件
    struct epoll_event evs[1024];
    int size=sizeof(evs)/sizeof(struct epoll_event);
    while(1){
        
        int num = epoll_wait(epoll_fd,evs,size,-1);
        for(int i=0;i<num;i++){
            int fd = evs[i].data.fd;
            if(fd==lfd){//fd为listen所用的fd
                //建立新连接 accept,完成上树操作
                DEBUG("begin accpet client");
                acceptClient(lfd,epoll_fd);
            }else{//否则其他fd说明是连接好的客户端发过来的
                //接收数据
                recvHttpRequest(fd,epoll_fd);
            }
        }
    }
    return 0;
}
int acceptClient(int lfd,int epfd){
    //1.建立连接
    int cfd=accept(lfd,NULL,NULL);

    if(cfd==-1){
        DEBUG("ACCEPT error");
        return -1;
    }else{
        DEBUG("ACCEPT success");
    }
    //2.设置非阻塞
    int flag=fcntl(cfd,F_GETFL);
    flag|=O_NONBLOCK;//追加非阻塞
    fcntl(cfd,F_SETFL,flag);
    //3.cfd添加到epoll中
    struct epoll_event ev;
    ev.data.fd=cfd;//listen文件描述符
    ev.events=EPOLLIN|EPOLLET;//读事件,边沿
    int ret = epoll_ctl(epfd,EPOLL_CTL_ADD,cfd,&ev);
    if(ret==-1){
        DEBUG("cfd epoll init error");
        return -1;
    }else{
        DEBUG("cfd epoll init success");
    }
    return 0;
}

int recvHttpRequest(int cfd,int epfd){
    char tmp[1024]={0};
    char buf[4096]={0};
    int len=0,total=0;
    while((len=recv(cfd,buf,sizeof(buf),0))>0){
        if(total+len>sizeof(buf)){
            memcpy(buf+total,tmp,len);
        }
        
        total+=len;
    }
    if(len==-1 && errno==EAGAIN){//数据接收完毕
        //解析请求行
        //只处理get请求
        char* pt=strstr(buf,"\r\n");
        int reqlen=pt-buf;
        buf[reqlen]='\0';
        parseRequestLine(buf,cfd);


    }else if(len==0){
        //客户端断开连接

        epoll_ctl(epfd,EPOLL_CTL_DEL,cfd,NULL);
        close(cfd);
    }else{
        DEBUG("recv error");
    }
    return 0;
}

int parseRequestLine(const char* line,int cfd){
    //解析请求行 方法和路径  get /xxx/1.jpg http/1.1
    char method[12];
    char dir[1024];
    sscanf(line,"%[^ ] %[^ ]",method,dir);
    DEBUG("%s,%s",method,dir);
    //判断是get还是post
    if(strcasecmp(method,"get")!=0){
        return -1;
    }
    decodeMsg(dir, dir);
     //处理客户端请求的静态资源
    char* file=NULL;
    if(strcmp(dir,"/")==0){
        file=dir;
    }else{
        file=dir+1;
    }
    printf("%s",dir);
    //获取文件属性
    struct stat st;
    int ret=stat(file,&st);
    if(ret==-1){
        //文件不存在
        sendHeadMsg(cfd,404,"Not Found",getFileType(".html"),-1);
        sendFile("404.html",cfd);
        return 0;
    }
    //判断文件类型
    if(S_ISDIR(st.st_mode)){
        //是目录，返回目录内容
        sendHeadMsg(cfd,200,"OK",getFileType(".html"),-1);
        sendDir(file,cfd);
        // sendFile("404.html",cfd);
    }else{
        //不是目录，是文件返回文件
        sendHeadMsg(cfd,200,"OK",getFileType(file),st.st_size);
        sendFile(file,cfd);
    }
   
    return 0;
}

int sendFile(const char* fileName,int cfd){
    //打开
    DEBUG("%s",fileName);
    int fd=open(fileName,O_RDONLY);
    assert(fd>0);
    //读取并发送文件
#if 0
    while(1){
        char buf[1024];
        int len=read(fd,buf, sizeof(buf));
        if(len>0){
            send(cfd,buf,len,0); 
            usleep(10);
        }else if(len==0){
            break;
            //文件读完
        }else{
            DEBUG("DURING READ ERROR");
        }
    }
#else
    int size=lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);
    off_t offset_now=0;
    while(offset_now<size){
        int ret= sendfile(cfd,fd,&offset_now,size);
        if(ret==-1){
            DEBUG("SEND FILE ERROR");
        }
    }
   
    // if(ret==-1){
    //     DEBUG("file send error");
    //     return -1;
    // }else{
    //     DEBUG("file send success");
    // }
#endif  
    close(fd);
    return 0;
}

int sendHeadMsg(int cfd,int status,const char* descr,const char* type,int length){
    //状态行
    char buf[4096]={0};
    sprintf(buf,"http/1.1 %d %s\r\n",status,descr);
    //响应头
    sprintf(buf+strlen(buf),"content-type: %s\r\n",type);
    sprintf(buf+strlen(buf),"content-length: %d\r\n\r\n",length);
    send(cfd,buf,strlen(buf),0);


    return 0;
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
/*
<html>
    <head>
        <title>test</title>
    </title>
    <body>
        <table>
            <tr>
                <td></td>   #内容
                <td></td>   #内容
            </tr>
            <tr>
                <td></td>
                <td></td>
            </tr>
        </table>
    </body>
</html>
*/
int sendDir(const char* dirname,int cfd){
    struct dirent** namelist;
    char buff[4096]={0};
    sprintf(buff,"<html><head><title>%s</title></head><body><table>",dirname);
    int num =scandir(dirname,&namelist,NULL,alphasort);
    for(int i=0;i<num;i++){
        char* name=namelist[i]->d_name;
        struct stat st;
        
        char subpath[1024]={0};

        sprintf(subpath,"%s/%s",dirname,name);
        stat(subpath,&st);
        if(S_ISDIR(st.st_mode)){
            //添加跳转html标签<a href="">name</a>
            sprintf(buff+strlen(buff),"<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",name,name,st.st_size);
        }else{
            sprintf(buff+strlen(buff),"<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",name,name,st.st_size);
        }
        send(cfd,buff,sizeof(buff),0);
        memset(buff,0,sizeof(buff));
        free(namelist[i]);
    }
    sprintf(buff,"</table></body></html>");
    send(cfd,buff,sizeof(buff),0);
    free(namelist);
    return 0;
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