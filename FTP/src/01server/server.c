/*************************************************************************
	> File Name: server.c
	> Author: 
	> Mail: 
	> Created Time: 2018年01月25日 星期四 10时04分01秒
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <taskLink.h>
#include <common.h>
#include <login_ftp.h>
#include <sys/epoll.h>
#include <libconfig.h>
#include <log.h>

//处理请求文件列表
void deal_seq_list(TransTask_t *trans_task, clientInfo_t *cli_info, char *dir_path)
{
    //如果是请求文件列表的话，path为空(例 show )，如果是请求目录文件的话，path为目录路径(例 get ..)
    if(dir_path == NULL)
    {
        dir_path = ".";
    }

    //将客户端事件添加到任务队列中 
    TransTask_Add_DownLoadTask(trans_task, cli_info, dir_path);
}

//处理文件请求
int deal_seq_flie(TransTask_t *trans_task, clientInfo_t *cli_info, seq_head_t seq_head)
{
    char fileName[MAX_PATH];
    memset(fileName, 0x00, MAX_PATH);
    //获取文件名
    if(recv_data(cli_info->cli_fd, fileName, seq_head.length) == -1)
    {
        ERROR_LOG("server recv from client [%s(%s : %d)] request file name error!", cli_info->cli_username, cli_info->cli_ip, cli_info->cli_port);
        return -1;
    }

    printf("请求下载的文件是: %s\n", fileName);
    //定义请求回复头
    seq_head_t rsp_head;
    //判断文件是否存在
    if(access(fileName, F_OK) != 0)
    {
        rsp_head.type = RSP_NOEXIST;
        //发送回复头信息(告诉客户端请求的文件或目录不存在)
        if(send_data(cli_info->cli_fd, (char *)&rsp_head, sizeof(seq_head_t)) == -1)
        {
            ERROR_LOG("server send to client [%s(%s : %d)] request file noexist response error!", cli_info->cli_username, cli_info->cli_ip, cli_info->cli_port);
            return -1;
        }
        return 0;
    }
    //获取文件属性
    struct stat st;
    if(stat(fileName, &st) < 0)
    {
        ERROR_LOG("client [%s(%s : %d)] server get stat of download file error: %s!", cli_info->cli_username, cli_info->cli_ip, cli_info->cli_port, strerror(errno));
        return -1;
    }
    file_head_t file_head;
    //判断是否为目录文件
    if(S_ISDIR(st.st_mode))
    {
        deal_seq_list(trans_task, cli_info, fileName);   
        return 0;
    }

    //是普通文件--------------------------------
    //填充文件头信息(包含文件名，文件长度)
    memset(&file_head, 0x00, sizeof(file_head_t));
    memcpy(file_head.fileName, fileName, strlen(fileName));
    file_head.fileLen = st.st_size;

    //定义请求回复的公共头
    rsp_head.type = RSP_FILE;
    rsp_head.length = sizeof(file_head_t);  //数据大小就是文件头结构的大小
    
    //发送回复头信息
    if(send_data(cli_info->cli_fd, (char *)&rsp_head, sizeof(seq_head_t)) == -1)
    {
        ERROR_LOG("server send to client [%s(%s : %d)] download file response head error!", cli_info->cli_username, cli_info->cli_ip, cli_info->cli_port);
        return -1;
    }
    //发送回复数据(文件头信息)
    if(send_data(cli_info->cli_fd, (char *)&file_head, rsp_head.length) == -1)
    {
        ERROR_LOG("server send to client [%s(%s : %d)] download file data error!", cli_info->cli_username, cli_info->cli_ip, cli_info->cli_port);
        return -1;
    }

    INFO_LOG("client [%s(%s : %d)] request download file :[%s(%d bytes)] !", cli_info->cli_username, cli_info->cli_ip, cli_info->cli_port, file_head.fileName, file_head.fileLen);

    //将客户端事件添加到任务队列中 
    TransTask_Add_DownLoadTask(trans_task, cli_info, fileName);

    return 0;
}

//处理上传文件请求
int deal_seq_upload(TransTask_t *trans_task, clientInfo_t *cli_info, seq_head_t seq_head)
{
    file_head_t file_head;
    seq_head_t rsp_head;

    //接收上传的文件名和其大小
    if(recv_data(cli_info->cli_fd, (char *)&file_head, sizeof(file_head_t)) == -1)
    {
        ERROR_LOG("server recv from client [%s(%s : %d)] upload file name and size error!", cli_info->cli_username, cli_info->cli_ip, cli_info->cli_port);
        return -1;
    }

    printf("上传的文件是%s, 大小为%ld\n", file_head.fileName, file_head.fileLen);
    INFO_LOG("client [%s(%s : %d)] request upload file :[%s(%d bytes)] !", cli_info->cli_username, cli_info->cli_ip, cli_info->cli_port, file_head.fileName, file_head.fileLen);

    //定义请求回复的公共头
    rsp_head.type = RSP_UPLOAD;

    //发送回复头信息
    if(send_data(cli_info->cli_fd, (char *)&rsp_head, sizeof(seq_head_t)) == -1)
    {
        ERROR_LOG("server send to client [%s(%s : %d)] upload response head error!", cli_info->cli_username, cli_info->cli_ip, cli_info->cli_port);
        return -1;
    }
  
    //将客户端上传任务添加到任务队列中 
    TransTask_Add_UploadTask(trans_task, cli_info, &file_head);
    
    return 0;
}

//处理登录或注册请求
void deal_seq_login_or_register(clientInfo_t *cli_info, int seq_type)
{
    seq_head_t rsp_head;
    account_t account;
    memset(&rsp_head, 0x00, sizeof(seq_head_t));
    memset(&account, 0x00, sizeof(account_t));

    //先接登录或注册账户信息
    if(recv_data(cli_info->cli_fd, (char *)&account, sizeof(account_t)) == -1)
    {
        ERROR_LOG("server recv client [(%s : %d)] account login/register infomation error!",  cli_info->cli_ip, cli_info->cli_port);
        return;
    }

    int success = 0;
    if(seq_type == SEQ_LOGIN)
    {
        if(login_ftp_server(account.username, account.password) == TRUE)
        {
            success = 1;
        }
    }
    else
    {
        if(register_ftp_account(account.username, account.password) == TRUE)
        {
            success = 1;
        }
        rsp_head.type = RSP_LOGIN;
    }
    if(success)
    {
        //成功返回RSP_LOGIN;
        rsp_head.type = RSP_LOGIN;

        //创建接收客户端上传文件的目录
        if(mkdir(account.username, 0777) == -1)
        {
            if(errno != EEXIST)
            {
                ERROR_LOG("server mkdir client [%s(%s : %d)] upload dir error: %s", cli_info->cli_username, cli_info->cli_ip, cli_info->cli_port, strerror(errno));
                return;
            }
        }

        memcpy(cli_info->cli_username, account.username, strlen(account.username));
        INFO_LOG("client [%s(%s : %d)] login success!", cli_info->cli_username, cli_info->cli_ip, cli_info->cli_port);
        printf("有新客户端登录\n");
    }
    else
    {
        //失败返回RSP_REGISTER
        rsp_head.type = RSP_REGISTER;
    }

    //向客户端回复请求
    if(send_data(cli_info->cli_fd, (char *)&rsp_head, sizeof(seq_head_t)) == -1)
    {
        ERROR_LOG("server send to client [%s(%s : %d)] login/register response error!", cli_info->cli_username, cli_info->cli_ip, cli_info->cli_port);
        return;
    }
}

//接收并处理客户端发送过来的请求
int recv_handle(TransTask_t *trans_task, clientInfo_t *cli_info)
{
    seq_head_t  seq_head;

    //先接收请求头结构
    int ret = recv_data(cli_info->cli_fd, (char *)&seq_head, sizeof(seq_head_t));
    if(ret == -1)
    {
        ERROR_LOG("server recv from client [%s(%s : %d)] request head error!", cli_info->cli_username, cli_info->cli_ip, cli_info->cli_port);
        return -1;
    }
    else if(ret == 1)
    { 
        //将其从监听列表中删除，并释放资源
        epoll_ctl(trans_task->epfd, EPOLL_CTL_DEL, cli_info->cli_fd, NULL);
        free(cli_info);

        INFO_LOG("client [%s(%s : %d)] close connect!", cli_info->cli_username, cli_info->cli_ip, cli_info->cli_port);
        return -1;
    }

    //判断请求类型
    switch(seq_head.type)
    {
        case SEQ_LIST://请求文件列表
        { 
            //处理文件列表请求
            INFO_LOG("client [%s(%s : %d)] request server file list!", cli_info->cli_username, cli_info->cli_ip, cli_info->cli_port);
            deal_seq_list(trans_task,cli_info, NULL);
            break;
        }
        case SEQ_FILE://请求文件数据 
        {
            //处理文件请求
            return deal_seq_flie(trans_task, cli_info, seq_head);
        }
        case SEQ_UPLOAD://请求上传文件
        {
            //将客户端描述符从监听列表中移除
            epoll_ctl(trans_task->epfd, EPOLL_CTL_DEL, cli_info->cli_fd, NULL);

            return deal_seq_upload(trans_task, cli_info, seq_head);
        }
        case SEQ_LOGIN: //请求登录
        {
            INFO_LOG("client [%s(%s : %d)] request login!", cli_info->cli_username, cli_info->cli_ip, cli_info->cli_port);
            deal_seq_login_or_register(cli_info, SEQ_LOGIN);
            break;
        }
        case SEQ_REGISTER: //请求注册
        {
            INFO_LOG("client [%s(%s : %d)] request register!", cli_info->cli_username, cli_info->cli_ip, cli_info->cli_port);
            deal_seq_login_or_register(cli_info, SEQ_REGISTER);
            break;
        }
        default:      //错误请求 
        {
            break;
        }
    }
    return 0;
}


int main(void)
{
    //创建文件传输任务控制结构
    TransTask_t  trans_task;
    //初始化控制结构
    TransTask_Init(&trans_task);

    
    char log_cfg_path[MAX_PATH] = {0};
    char sev_IP[16] = {0};
    int  sev_port = -1;

    //读取配置文件，获取日志文件路径、IP、PORT信息
    if(parse_conf(log_cfg_path, sev_IP, &sev_port) == -1)
    {
        exit(1);
    }
    
    //初始化日志
    get_log_handle(log_cfg_path, "f_cat");

    int lst_fd;    //监听套接字描述符
    int cli_fd;    //新连接的客户端套接字描述符
    struct sockaddr_in  cli_addr;    //新连接的客户端地址信息
    int len = sizeof(struct sockaddr_in);

    //初始化服务器端地址和端口
    if((lst_fd = socket_create_listen_fd(sev_IP, sev_port, MAX_LISTEN)) == -1)
    {
        exit(1);
    }

    //存放已就绪的描述符的事件数组
    struct epoll_event evs[MAX_LISTEN];
    struct epoll_event ev;
    //将监听套接字描述符添加到监听列表中
    ev.events = EPOLLIN;  //监听可读事件
    ev.data.fd = lst_fd;
    epoll_ctl(trans_task.epfd, EPOLL_CTL_ADD, lst_fd, &ev);

    while(1)
    {
        //等待监听(超时时间为3秒)的描述符就绪事件的发生，如有被监听的描述符就绪，
        //则将该描述符放入evs数组中，返回值为就绪描述符个数
        int nfds = epoll_wait(trans_task.epfd, evs, MAX_LISTEN, 3000);
        if(nfds < 0)
        {
            ERROR_LOG("server epoll_wait error: %s!", strerror(errno));
            continue;
        }
        else if(nfds == 0)
        {
            printf("time out!\n");
            continue;
        }
        int i = 0;
        //nfds是就绪描述符个数，evs中前nfds个描述符都是已就绪的描述符
        for(i = 0; i < nfds; i++)
        {
            if(evs[i].data.fd == lst_fd)
            {//监听套接字描述符就绪，说明有新连接到来
                if((cli_fd = accept(lst_fd, (struct sockaddr *)&cli_addr, &len)) == -1)
                {
                    continue;
                }

                //将新客户端的通信套接字描述符添加到监听列表
                clientInfo_t *newCli = (clientInfo_t *)malloc(sizeof(clientInfo_t));
                memset(newCli, 0x00, sizeof(clientInfo_t));
                memcpy(newCli->cli_ip, inet_ntoa(cli_addr.sin_addr), strlen(inet_ntoa(cli_addr.sin_addr)));
                newCli->cli_port = ntohs(cli_addr.sin_port);
                newCli->cli_fd = cli_fd;
                ev.data.ptr = newCli;
                ev.events = EPOLLIN;
                epoll_ctl(trans_task.epfd, EPOLL_CTL_ADD, cli_fd, &ev);

            }
            //说明客户端有请求过来
            else if(evs[i].events & EPOLLIN)
            {
                //接收并处理客户端发送过来的请求
                if( recv_handle(&trans_task, (clientInfo_t *)evs[i].data.ptr) == -1)
                {
                    //关闭描述符
                    close(cli_fd);
                } 
            }
        }
    }
    return 0;
}
