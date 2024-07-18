#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include<string.h>
#include<dirent.h>
#include<poll.h>
#include "tcp.h"
#include "ftp.h"
void handler(int sig)
{
    switch (sig)
    {
    case SIGCHLD://说明有子进程结束了
        waitpid(-1,NULL,1);//回收退出子进程的资源，防止成为僵尸进程
        break;
    
    default:
        break;
    }
}

void get_filename(int fd)
{
    DIR *dir;
    struct dirent *dirent;
    dir=opendir(".");
    if(dir==NULL)
    {
        perror("open failed");
        return ;
    }
    char filelist[100]={0};
    char file[100]={0};
    int i=0;
    while(1)
    {
        dirent=readdir(dir);
        if(NULL==dirent)
        break;
        if(strcmp(dirent->d_name,".")==0 || strcmp(dirent->d_name,"..")==0)
             continue;
        sprintf(filelist,"%s",dirent->d_name);
        strcat(file,filelist);
        strcat(file,"\n");
     //   printf("%s\n",filelist);
       
    }
    printf("file : %s\n",file);
    send_message(fd,file,sizeof(file));
    closedir(dir);
}
//子进程和客户端通信的代码
void communication(int fd)
{
    unsigned char buf[1000];
    char filename[100];
    int r,rf;
    while(1)
    {
        struct pollfd fds[2];//要监听两个文件描述符，所以需要两个结构体，-》数组
        fds[0].fd = STDIN_FILENO;
        fds[0].events = POLLIN;
        fds[0].revents = 0;

        fds[1].fd = fd;
        fds[1].events = POLLIN | POLLRDHUP;
        fds[1].revents = 0;

        r = poll(fds,2,50000);
         if(r==-1)
        {
            perror("监听失败\n");
        }
        else if(r==0)
        {
            printf("5秒钟内没有文件描述符就绪\n");
        }   
        else if(r > 0)
        {
            //有文件描述符就绪
            if(fds[0].revents & POLLIN)//说明用户从键盘输入了
            {
                 
            }

            if(fds[1].revents & POLLRDHUP)//和下面的if语句顺序不能交换
            {
                printf("对方断开连接\n");
                break;
            }

            if(fds[1].revents & POLLIN)
            {
                // //说明fd就绪了，-》对方发送消息给我了
                // r = read(fd,buf1,100);
                // printf("r=%d\n",r);
                // buf1[r] = '\0';
                // printf("收到消息:%s\n",buf1);
                r = recv_message(fd,buf,1000);
                switch (buf[0])
                {
                case DOWNLOAD:
                    /* code */
                    strncpy(filename,&buf[2],buf[1]);
                    filename[buf[1]] = '\0';
                    //把这个文件通过网络传送给客户端
                    printf("download filename:%s\n",filename);
                    send_file(fd,filename);
                    break;
                
                case UPLOAD:
                    strncpy(filename,&buf[2],buf[1]);
                    filename[buf[1]] = '\0';
                    //把这个文件通过网络传送给客户端
                    printf("upload filename:%s\n",filename);
                    recv_file(fd,filename);
                    break;

                case LIST:
                get_filename(fd);
                    break;
                default:
                    break;
                }
            }
        }
      // 
        printf("%s,%d,r=%d\n",__func__,__LINE__,r);
        //收到客户端的消息后，需要理解客户端的意图
        
    }
}


// 需要两个参数，服务端ip和端口
int main(int argc,char *argv[])
{
    if(argc != 3)
    {
        printf("参数错误\n");
        return -1;
    }

    signal(SIGCHLD,handler);

    int sockfd = init_tcp_server_socket(argv[1],atoi(argv[2]));
    if(-1==sockfd)
    {
        printf("初始化tcp服务端失败\n");
        return -1;
    }

    //并发服务器，同时处理多个客户端的请求
    int fd;//用于通信的描述符

    while(1)
    {
        struct sockaddr_in client_addr;
        int len = sizeof(client_addr);
        fd = accept(sockfd,//是否有客户端向当前套接字描述符发起连接请求，如果没有会阻塞
                (struct sockaddr*)&client_addr,//传入client_addr地址，如果accept成功返回了，那么client_addr
                                                //保存了客户端的套接字地址
                &len
            );
        //printf("%s函数的第%d行\n",__func__,__LINE__);
        printf("客户端连接成功了，它的ip为:%s,端口为:%hu\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

        //创建子进程和已经连接成功的客户端通信，父进程继续处理其他客户端的连接请求
        pid_t pid = fork();
        if(-1==pid)
        {
            perror("创建进行失败");
            close(fd);
            close(sockfd);
            return -1;
        }
        else if(pid == 0)//子进程
        {
            //和客户端通信
            communication(fd);
            exit(0);
        }

        //父进程继续循环执行 accept
        //wait(NULL); //绝对不能在此wait，因为wait会阻塞，父进程就无法继续响应其他客户端的连接
        //应该捕捉SIGCHLD信号，在信号处理函数中 wait回收子进程资源，防止子进程成为僵尸进程

    }

    close(fd);
    close(sockfd);

    return 0;
}