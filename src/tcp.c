#include "tcp.h"


/*
    初始化一个tcp服务端套接字(主要包括创建套接字，绑定套接字地址，进入监听状态)
    参数：
        ip  服务端的IP地址
        port  服务端的端口号
    返回值：
        失败返回-1
        成功返回套接字描述符
*/

int init_tcp_server_socket(const char *ip,short port)
{
     int sockfd ;//套接字描述符，对于服务端而言，该描述符用来监听和建立连接
    //1，创建套接字
    sockfd = socket(AF_INET,//tcp属于 IPV4协议族
            SOCK_STREAM,//流式套接字专门用于tcp
            0   //不知名的私有协议(指的应用层协议)
        );
    if(-1==sockfd)
    {
        perror("创建套接字失败");
        return -1;
    }

    //2，初始化服务端自己的套接字地址
    struct sockaddr_in server_addr;//服务端自己的套接字地址  ip + 端口
                                    //服务端程序在哪里运行就是哪个ip地址
                                    //端口号 随意(但是不能是知名应用的端口)，也不能跟本机其他程序端口冲突
    memset(&server_addr,0,sizeof(server_addr));//把整个结构体内存全部置0
    server_addr.sin_family = AF_INET;//IPV4协议族
    server_addr.sin_port = htons(port);//指定端口为 10000
    inet_aton(ip,&(server_addr.sin_addr));//指定ip地址

    //3，绑定套接字地址
    int r = bind(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr));
    if(-1 == r)
    {
        perror("绑定套接字地址失败");
        close(sockfd);
        return -1;
    }

    //4，监听套接字
    r = listen(sockfd,3);
    if(-1 == r)
    {
        perror("监听套接字失败");
        close(sockfd);
        return -1;
    }
    return sockfd;
}


/*
    连接tcp服务端
    参数：
        ip  服务端的IP地址
        port  服务端的端口号
    返回值：
        失败返回-1
        成功返回套接字描述符
*/

int connect_tcp_server(const char *ip,short port)
{
    int sockfd ;
    //1，创建套接字
    sockfd = socket(AF_INET,//tcp属于 IPV4协议族
            SOCK_STREAM,//流式套接字专门用于tcp
            0   //不知名的私有协议(指的应用层协议)
        );
    if(-1==sockfd)
    {
        perror("创建套接字失败");
        return -1;
    }

    //2，可以绑定，也可以不绑定，如果绑定也是绑定自己的IP和端口
    #if 0
    struct sockaddr_in client_addr;//客户端自己的套接字地址  ip + 端口
                                    //客户端程序在哪里运行就是哪个ip地址
                                    //端口号 随意(但是不能是知名应用的端口)，也不能跟本机其他程序端口冲突
    memset(&client_addr,0,sizeof(client_addr));//把整个结构体内存全部置0
    client_addr.sin_family = AF_INET;//IPV4协议族
    client_addr.sin_port = htons(10000);//指定端口为 10000
    inet_aton("172.80.1.197",&(client_addr.sin_addr));//指定ip地址
    bind(sockfd,(struct sockaddr*)&client_addr,sizeof(client_addr));
    #endif

    //3，向服务端发送连接请求
    struct sockaddr_in server_addr;//服务端的套接字地址  ip + 端口
                                    //服务端程序在哪里运行就是哪个ip地址
                                    //端口号 随意(但是不能是知名应用的端口)，也不能跟本机其他程序端口冲突
    memset(&server_addr,0,sizeof(server_addr));//把整个结构体内存全部置0
    server_addr.sin_family = AF_INET;//IPV4协议族
    server_addr.sin_port = htons(port);//指定端口为 10000
    inet_aton(ip,&(server_addr.sin_addr));//指定ip地址
    int r = connect(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr));//第二个参数需要指定服务端的套接字地址
    if(-1==r)
    {
        perror("连接失败");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

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

int send_message(int fd,unsigned char *data,int len)
{
    unsigned char buf[len*2+3];//最极端的情况，原始数据中全是0xff和0xfd，那么转换之后的总长度翻倍
                                //前面还要加上包头1字节，数据包长度2字节
    buf[0] = 0xff;//包头

    int j=3;//前面还要加上包头1字节，数据包长度2字节
    int i;
    for(i=0;i<len;i++)
    {
        if(data[i] == 0xff)
        {
            buf[j++] = 0xfd;
            buf[j++] = 0xfe;
        }
        else if(data[i] == 0xfd)
        {
            buf[j++] = 0xfd;
            buf[j++] = 0xfd;
        }
        else//普通数据
        {
            buf[j++] = data[i];
        }
    }

    short *p = (short *)&buf[1];
    *p = htons(j-3);

    int r = write(fd,buf,j);
    return r;
}

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

int recv_message(int fd,unsigned char *data,int len)
{
    int r;
    //找到该消息的包头
    unsigned char ch;
    do
    {
        r = read(fd,&ch,1);
        if(-1==r)
            return -1;
    } while (ch!=0xff);

    short messageLen;
    r = read(fd,&messageLen,2);//读取数据包的长度
    if(-1==r)
        return -1;
    //printf("%s,%d,r=%d\n",__func__,__LINE__,r);
    messageLen = ntohs(messageLen);
    unsigned char buf[messageLen];
    r = read(fd,buf,messageLen);//读取的是转换之后的数据，还要经过反向转换才能得到原始数据
    if(-1==r)
        return -1;
    //printf("%s,%d,r=%d\n",__func__,__LINE__,r);
    int i;
    int j=0;
    for(i=0;i<messageLen && j<len;i++)
    {
        if(buf[i] == 0xfd)
        {
            i++;
            if(buf[i] == 0xfe)
            {
                data[j++] = 0xff;
            }
            else if(buf[i]==0xfd)
            {
                data[j++] = 0xfd;
            }
        }
        else
        {
            data[j++] = buf[i];
        }
    }
    
    return j;
}