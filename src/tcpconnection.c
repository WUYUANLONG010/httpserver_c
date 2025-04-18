
#include "tcpconnection.h"

#include <stdio.h>
#include <stdlib.h>

#include "channel.h"
#include "eventloop.h"
#include "httprequest.h"
#include "httpresponse.h"
#include "server.h"

int tcp_process_read(void *argv) {
    struct TcpConnection *conn = (struct TcpConnection *)argv;
    // 接收http请求数据

    int count = buffer_socket_read(conn->read_buffer, conn->channel->fd);
    DEBUG("接收到数据:%s", conn->read_buffer->data + conn->read_buffer->readpos);
    // DEBUG("接收到数据:%d",count);
    if (count > 0) {
        int socket=conn->channel->fd;
        // 接收到了数据（http请求）
        // 发送http响应
#ifdef MSG_SEND_AUTO
        write_event_enable(conn->channel, true);
        event_add_task(conn->evloop, conn->channel, MODIFY);

#endif
        DEBUG("开始解析");
        bool ret = Parse_Http_requset(conn->httprequset, conn->read_buffer, conn->httpresponse, conn->write_buffer,socket);//buffer组织好了响应数据
        DEBUG("解析是否成功 %d",ret);
        if(!ret){
            // 解析失败 回复一个简单的html
            DEBUG("HTTP数据解析失败，回复404");
            char *errMSG = "Http/1.1 400 Bad Request\r\n\r\n"; 
            buffer_append_string(conn->write_buffer, errMSG); 
        }
    } else {
        // count==0
#ifdef MSG_SEND_AUTO
    event_add_task(conn->evloop, conn->channel, DELETE);
#endif
    }
    //断开连接
#ifndef MSG_SEND_AUTO
    event_add_task(conn->evloop, conn->channel, DELETE);
#endif
    return 0;
}
int tcp_process_write(void *argv) {
    DEBUG("开始发送数据了(基于写事件发送)....");
    struct TcpConnection *conn = (struct TcpConnection *)argv;
    int count                  = buffer_socket_send(conn->write_buffer, conn->channel->fd);
    DEBUG("发送数据count:%d,%s",count,conn->write_buffer->data);
    if (count > 0) {

        if (buffer_readable(conn->write_buffer) == 0) { // 没有读的数据了
            DEBUG("全部数据已发送，删除发送任务");
            //1.不再检测写事件--修改channel中保存的事件
            write_event_enable(conn->channel, false);
            // 2.修改dispather检测集合--添加任务节点
            event_add_task(conn->evloop, conn->channel, MODIFY);

            // 3.删除这个节点
            event_add_task(conn->evloop, conn->channel, DELETE);
        }
    }
    return 0;
}
struct TcpConnection *tcp_connection_init(int fd, struct EventLoop *evloop) {
    struct TcpConnection *conn = (struct TcpConnection *)malloc(sizeof(struct TcpConnection));
    conn->evloop               = evloop;
    conn->read_buffer          = buffer_init(10240);
    conn->write_buffer         = buffer_init(10240);
    sprintf(conn->name, "connection-%d", fd);
    conn->channel = channel_init(fd, read_event, tcp_process_read, tcp_process_write,tcp_connection_destroy, conn);
    event_add_task(evloop, conn->channel, ADD);
    conn->httprequset  = HttpRequest_init();
    conn->httpresponse = HttpResponse_init();
    DEBUG("和客户端建立连接, threadName: %s, threadID:%lu, connName: %s",
        evloop->threadName, evloop->thread_ID, conn->name);
    return conn;
}
int tcp_connection_destroy(void *arg) {
    // DEBUG("开始断开连接");
    struct TcpConnection *conn = (struct TcpConnection *)arg;
    if (conn != NULL) {
        if(conn->read_buffer&&buffer_readable(conn->read_buffer)&&
        conn->write_buffer&&buffer_writable(conn->write_buffer)){
            destroychannel(conn->evloop, conn->channel);
            buffer_destroy(conn->read_buffer);
            buffer_destroy(conn->write_buffer);
            HttpRequest_destroy(conn->httprequset);
            HttpResponse_destroy(conn->httpresponse);
            free(conn);
        }
        DEBUG("连接断开, 释放资源, gameover, connName: %s", conn->name);
        return 0;
    }
    return -1;
}
