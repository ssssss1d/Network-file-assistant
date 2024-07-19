#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ftp.h"
#include "tcp.h"

void send_file(int fd,const char *pathname)
{
    //打开文件
    int filefd = open(pathname,O_RDONLY);
    //先暂时不讨论失败的情况
    if(-1 == filefd)
    {
        close(filefd);
        perror("open failed");
        return ;
    }
    unsigned char buf[1000];
    int r;
    while(1)
    {
        //读取文件内容
        buf[0] = 0;//第一个字节为0表示，当前文件还没有发送完毕
        r = read(filefd,buf+1,999);
        if(0==r)
        {
            break;
        }
        //发送给对方
        send_message(fd,buf,r+1);
    }
    buf[0] = 1;//第一个字节为1表示，当前文件已经发送完毕
    send_message(fd,buf,1);
    //关闭文件
    close(filefd);
}

void recv_file(int fd,const char *pathname)
{
    //创建或打开本地文件
    printf("%s,%d\n",__func__,__LINE__);
    int filefd = open(pathname,O_WRONLY | O_TRUNC | O_CREAT,0664);
    if(-1==filefd)
    {
        printf("%s,%d\n",__func__,__LINE__);
        perror("");
    }

    //接收网络数据(文件内容)并写入到本地文件
    unsigned char buf[1000];
    int r ;
    while(1)//循环接收，怎么知道什么时候接收完毕？协议
    {
        r = recv_message(fd,buf,1000);
        if(buf[0]==1)//说明文件已经发送完毕
            break;
        write(filefd,buf+1,r-1);
    }
    close(filefd);
}