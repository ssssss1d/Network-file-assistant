#include <stdio.h>
#include "tcp.h"
#include "ftp.h"

//需要两个参数，服务端的ip和端口号
int main(int argc,char *argv[])
{
    if(argc != 3)
    {
        printf("参数错误\n");
        return -1;
    }

    int sockfd = connect_tcp_server(argv[1],atoi(argv[2]));
    if(-1 == sockfd)
    {
        printf("连接服务端失败\n");
        return -1;
    }
    char buf[100];
    unsigned char data[100];
    unsigned char filename[100];
    while(1)
    {
        printf("三个功能，请输入:(输入格式如下)\n");
        printf("下载文件输入:download filename\n");
        printf("上传文件输入:upload filename\n");
        printf("获取文件列表请输入:list\n");
        printf("退出请输入:exit\n");
        fgets(buf,100,stdin);
        //printf("%s\n",buf);
        if(strncmp(buf,"download ",9)==0)
        {
            printf("%s,%lu\n",buf,strlen(buf));
            data[0] = DOWNLOAD;
            data[1] = strlen(buf) - 10;
            strncpy(&data[2],&buf[9],data[1]);
            send_message(sockfd,data,2+strlen(buf) - 10);
            printf("%s,%d\n",__func__,__LINE__);
            strncpy(filename,&buf[9],data[1]);
            filename[data[1]] = '\0';
            printf("%s\n",filename);
            recv_file(sockfd,filename);
        }
        else if(strncmp(buf,"upload ",7)==0)
        {
            data[0]=UPLOAD;
            data[1]=strlen(buf)-8;
            strncpy(&data[2],&buf[7],data[1]);
            send_message(sockfd,data,2+strlen(buf) - 8);
            strncpy(filename,&buf[7],data[1]);
            filename[data[1]] = '\0';
            send_file(sockfd,filename);
        }
        else if(strncmp(buf,"list",4)==0)
        {
             data[0]=LIST;
             data[1]=strlen(buf)-5;
             strncpy(&data[2],&buf[4],data[1]);
             //printf("%c\n",data);
            send_message(sockfd,data,2+strlen(buf) - 5);
           // strncpy(filename,&buf[4],data[1]);
           // filename[data[1]] = '\0';
           // send_file(sockfd,filename);
            char rbuf[100];
           recv_message(sockfd,rbuf,sizeof(rbuf));
           printf("%s\n",rbuf);
        }
        else if(strncmp(buf,"exit",4)==0)
        {
            break;
        }
        else 
        {
            printf("输入有误\n");
        }
    }

    close(sockfd);
    return 0;
}