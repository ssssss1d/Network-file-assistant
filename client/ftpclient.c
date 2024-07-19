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
int sockfd;
  int flag;
void handler(int sig)
{
    switch (sig)
    {
    case SIGINT://说明有子进程结束了
       // waitpid(-1,NULL,1);//回收退出子进程的资源，防止成为僵尸进程
        close(sockfd);
        exit(0);
        break;
    
    default:
        break;
    }
}
int cmp_filename(int fd,const char * fp,const char * addr)
{
    printf("start cmp_filename\n");
    printf("addr:%s  sizeof(addr):%ld\n",addr,sizeof(addr));
    DIR *dir;
    struct dirent *dirent;
    dir=opendir(addr);
    if(dir==NULL)
    {
        perror("open failed\n");
        return -1;
    }
    char filelist[100]={0};
    char file[100]={0};
    int i=0;
  
    while(1)
    {
        dirent=readdir(dir);
        if(NULL==dirent)
        break;
       // printf("dirent->d_name:%s\n",dirent->d_name);
        if(strcmp(dirent->d_name,fp)==0)
           {
             flag=1;//标志位，如果存在该文件则开始发送
            break;
           }
      //  sprintf(filelist,"%s",dirent->d_name);
       // strcat(file,filelist);
        //strcat(file,"\n");
     //   printf("%s\n",filelist);
       
    }
   // printf("file : %s\n",file);
   //   printf("flag==%d\n",flag);
    if(flag==0)
    {
       // perror("file does not exist");
        return -1;
    }
  
  //  send_message(fd,file,sizeof(file));
    closedir(dir);
    return 0;
}

//需要两个参数，服务端的ip和端口号
int main(int argc,char *argv[])
{
    if(argc != 3)
    {
        printf("参数错误\n");
        return -1;
    }
    signal(SIGINT,handler);
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
            if(cmp_filename(sockfd,filename,"../server")==0)
            recv_file(sockfd,filename);
            else
            printf("file does not exist\n");
        }
        else if(strncmp(buf,"upload ",7)==0)
        {
            data[0]=UPLOAD;
            int ans=7;
            char path[100]={0};
            while(buf[ans++]!=' ')
             {}
            data[1]=ans-8;//计算文件名长度
            strncpy(&data[2],&buf[7],data[1]);
            strncpy(filename,&buf[7],data[1]);//文件名拼接
            printf("data[1]:%d\n",data[1]);
            filename[data[1]] = '\0';
            int sum=ans;//截止为文件名后一位下标
             while(buf[ans]!='\0')
                {ans++;}
                ans-=sum;//目录路径长度
                
                printf("sum:%d  buf[sum]:%c\n",sum,buf[sum]);
            strncpy(path,&buf[sum],ans-1);//获取输入的目标文件所在的路径
            printf("current file path:%s\n",path);
            printf("current filename :%s\n",filename);
          //  char  clientpath[]="../client";
            int x=cmp_filename(sockfd,filename,path);
             strcat(path,"/");
            strcat(path,filename);
            printf("path:%s\n",path);
          
            //    printf("cmp_filename==%d\n",x);
          //  if(cmp_filename(sockfd,filename)==-1)
            //    continue;
            if(flag==1)
                {
                    //printf("start send_file\n");
                    send_message(sockfd,data,2+data[1]);
                    //printf("看看在哪阻塞住了！");
                    send_file(sockfd,path);
                }
                else
                printf("file does not exist!\n");
            flag=0;
        }
        else if(strncmp(buf,"list ",5)==0)
        {
             data[0]=LIST;
             data[1]=strlen(buf)-6;
             strncpy(&data[2],&buf[5],data[1]);
             //printf("%c\n",data);
            send_message(sockfd,data,2+strlen(buf) - 6);
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