/*************************************************************************
	> File Name: fileStruct.h
	> Author: 
	> Mail: 
	> Created Time: 2018年01月25日 星期四 09时38分35秒
 ************************************************************************/

#ifndef _FILESTRUCT_H
#define _FILESTRUCT_H
#include <stdint.h>
#include <pthread.h>
#include "condition.h"

//linux中文件名最大长度为256字节
#define MAX_PATH 1024
#define BUFF_MAX 4096

//TLV格式中，type字段的类型
enum cmd_t
{
    SEQ_LIST = 0,       //请求文件列表
    RSP_LIST,           //对请求文件列表的回复
    SEQ_FILE,           //请求文件数据
    RSP_FILE,           //对请求文件数据的回复
    RSP_NOEXIST,        //对客户端回复文件或目录不存在
    SEQ_UPLOAD,         //请求上传文件
    RSP_UPLOAD,         //对请求上传文件的回复
    SEQ_LOGIN,          //请求登录
    RSP_LOGIN,          //对登录请求的回复
    SEQ_REGISTER,       //请求注册
    RSP_REGISTER,       //对注册请求的回复
    
};

//公共请求头结构
typedef struct __seq_head_t
{
    uint8_t     type;           //此次请求的类型
    uint64_t    length;         //此次请求携带的数据的长度
}seq_head_t;

//文件头结构
typedef struct __file_head_t
{
    char        fileName[MAX_PATH];  //文件名
    uint64_t    fileLen;             //文件长度
}file_head_t;

//客户端账户结构
typedef struct __account_t
{
    char    username[32];
    char    password[6];
}account_t;

//客户端信息结构
typedef struct __clientInfo_t
{
    char    cli_username[32];
    char    cli_ip[16];
    int     cli_port;
    int     cli_fd;
}clientInfo_t;

//服务器IP和端口
#define MAX_LISTEN  1024


#define TRUE 1
#define FALSE 0
//传输任务结点
typedef struct task
{
    clientInfo_t cli_info;      //与客户端通信的套接字描述符
    uint64_t offset;      //客户端请求的文件的已发送的字节数
    int      isDirent;    //请求的是目录, TRUE表示是目录，FALSE表示是文件
    char     filePath[MAX_PATH];   //请求的文件/目录名
    uint64_t file_size;   //文件大小
    struct task *next;
}task_t;

//传输文件任务控制结构
typedef struct 
{
    condition_t ready;         //条件变量
    pthread_t   transThread;   //传输文件线程标识符
    task_t  *head;      //任务队列头指针
    task_t  *last;      //任务队列尾指针
    int     epfd;       //操作监听集合句柄

}TransTask_t;

//文件列表结点结构
typedef struct _file
{
    char filePath[MAX_PATH];
    uint64_t  fileSize;
    struct _file *next;
}fileList_t;

#endif
