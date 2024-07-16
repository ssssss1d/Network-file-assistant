#ifndef __TCP_H_
#define __TCP_H_
#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

/*
    初始化一个tcp服务端套接字(主要包括创建套接字，绑定套接字地址，进入监听状态)
    参数：
        ip  服务端的IP地址
        port  服务端的端口号
    返回值：
        失败返回-1
        成功返回套接字描述符
*/

int init_tcp_server_socket(const char *ip,short port);


/*
    连接tcp服务端
    参数：
        ip  服务端的IP地址
        port  服务端的端口号
    返回值：
        失败返回-1
        成功返回套接字描述符
*/

int connect_tcp_server(const char *ip,short port);

/*
    发送一个完整的消息/数据包
    参数：
        fd 描述符
        data 要发送的原始数据的首地址
        len 原始数据的长度
    返回值：
        失败返回-1
        成功返回实际写入的字节数

    **如果内容中出现了0xff ，就用两个字节 0xfd 0xfe来代替**
    **如果内容中出现了0xfd ，就用两个字节 0xfd 0xfd来代替**
*/

int send_message(int fd,unsigned char *data,int len);

/*
    读取一个完整的消息/数据包
    参数：
        fd 文件描述符
        data 用来保存收到的原始数据的首地址
        len data指向内存的长度，防止越界
    返回值：
        失败返回-1
        成功返回原始数据包的长度

    收到数据之后， 遇到连续两个字节是 0xfd 0xfe 就转换为0xff，遇到连续两个字节是0xfd 0xfd就转换为0xfd
*/

int recv_message(int fd,unsigned char *data,int len);
#endif // ! __MY_SOCKET_H_