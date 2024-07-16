#ifndef __FTP_H__
#define __FTP_H__

enum MESSAGE_TYPE
{
    DOWNLOAD,//下载文件
    UPLOAD,//上传文件
    LIST   //获取文件列表
    //......
};

void recv_file(int fd,const char *pathname);

void send_file(int fd,const char *pathname);

#endif // !__FTP_H__

