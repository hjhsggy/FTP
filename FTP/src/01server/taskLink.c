/*************************************************************************
	> File Name: taskLink.c
	> Author: 
	> Mail: 
	> Created Time: 2018年01月27日 星期六 20时12分29秒
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <taskLink.h>
#include <common.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <log.h>

//读取目录
//@direntName: 目录名
//返回值: 文件列表的头指针
fileList_t* read_dir(char *direntName, int level)
{
    //文件列表的头指针
    static fileList_t *fileHead = NULL;

    //定义一个目录流指针
    DIR *p_dir = NULL;

    //定义一个目录结构体指针
    struct dirent *p_dirent = NULL;

    //打开目录，返回一个目录流指针指向第一个目录项
    p_dir = opendir(direntName);
    if(p_dir == NULL)
    {
        ERROR_LOG("server opendir of scandir error: %s", strerror(errno));
        return NULL;
    }

    //循环读取每个目录项, 当返回NULL时读取完毕
    while((p_dirent = readdir(p_dir)) != NULL)
    {
        char fileName[MAX_PATH] = {0};

        //排除掉当前目录和上级目录
        if(!strcmp(p_dirent->d_name, ".") || !strcmp(p_dirent->d_name, ".."))
        {
            continue;
        }

        sprintf(fileName, "%s/%s", direntName, p_dirent->d_name);

        //构造文件列表新结点
        fileList_t *newFile = malloc(sizeof(fileList_t));
        memset(newFile, 0x00, sizeof(fileList_t));
        memcpy(newFile->filePath, fileName, strlen(fileName));
        struct stat st;
        if(stat(fileName, &st) == -1)
        {
            ERROR_LOG("server get stat of after scandir error: %s", strerror(errno));
            return NULL;
        }
        newFile->fileSize = st.st_size;
        newFile->next = NULL;

        //将改文件头插如文件列表中
        newFile->next = fileHead;
        fileHead = newFile;

        //如果目录项仍是一个目录的话，进入目录
        if(p_dirent->d_type == DT_DIR)
        {
            char *pathBuff = NULL;

            //为子目录路径申请内存
            pathBuff = malloc(strlen(direntName) + strlen(p_dirent->d_name) + 2);

            //构建子目录路径
            sprintf(pathBuff, "%s/%s", direntName, p_dirent->d_name);

            //递归进入子目录
            read_dir(pathBuff, level + 1);

            //释放内存
            free(pathBuff);
            pathBuff = NULL;
        }
    }
    closedir(p_dir);
    if(level == 0)
    {
        fileList_t *ret = fileHead;
        fileHead = NULL;
        return ret;
    }
    else
    {
        return fileHead;  
    } 
}

//传输文件
int trans_file(task_t *pt)
{
    int send_len, temp;
    char buff[BUFF_MAX] = {0};


    //获取发送长度，最多每次发送4096字节
    temp = (pt->file_size) - (pt->offset);
    send_len = temp > BUFF_MAX ? BUFF_MAX : temp;

    //在文件偏移offset位置处读取send_len长度的数据到buff中
    int fd;

    if((fd = open(pt->filePath, O_RDONLY)) == -1)
    {
        ERROR_LOG("server open client [%s(%s : %d)] download file:[%s] error %s!", pt->cli_info.cli_username, pt->cli_info.cli_ip, pt->cli_info.cli_port,pt->filePath, strerror(errno));
        return -1;
    }

    //调整文件指针偏移量为pt->offset
    if(lseek(fd, pt->offset, SEEK_SET) == -1)
    {
        ERROR_LOG("server lseek client [%s(%s : %d)] download file:[%s] error %s!", pt->cli_info.cli_username, pt->cli_info.cli_ip, pt->cli_info.cli_port, pt->filePath, strerror(errno));
        return -1;
    }

    //在文件中读取send_len长度的数据到buff中
    if(recv_data(fd, buff, send_len) == -1)
    {
        ERROR_LOG("server read file:[%s] error!", pt->filePath);
        return -1;
    }

    //将读到的内容发送给客户端
    if(send_data(pt->cli_info.cli_fd, buff, send_len) == -1)
    {
        ERROR_LOG("server send to client [%s(%s : %d)] download file:[%s] data error!", pt->cli_info.cli_username, pt->cli_info.cli_ip, pt->cli_info.cli_port, pt->filePath);
        return -1;
    }

    //重置文件偏移量
    pt->offset += send_len;

    //关闭文件描述符
    close(fd);
}

//接收上传文件
int trans_upload_file(task_t *pt)
{
    int send_len, temp;
    char buff[BUFF_MAX] = {0};


    //获取接收长度，最多每次接收4096字节
    temp = (pt->file_size) - (pt->offset);
    send_len = temp > BUFF_MAX ? BUFF_MAX : temp;

    //在文件偏移offset位置处将buff中send_len长度的数据写进去
    int fd;

    if((fd = open(pt->filePath, O_WRONLY | O_CREAT, 0644)) == -1)
    {
        ERROR_LOG("server open client [%s(%s : %d)] upload file:[%s] error:%s!", pt->cli_info.cli_username, pt->cli_info.cli_ip,pt->cli_info.cli_port, pt->filePath, strerror(errno));
        return -1;
    }

    //调整文件指针偏移量为pt->offset
    if(lseek(fd, pt->offset, SEEK_SET) == -1)
    {
        ERROR_LOG("server lseek client [%s(%s : %d)] upload file:[%s] error:%s!", pt->cli_info.cli_username, pt->cli_info.cli_ip,pt->cli_info.cli_port, pt->filePath, strerror(errno));
        return -1;
    }

    //从客户端接受文件数据
    if(recv_data(pt->cli_info.cli_fd, buff, send_len) == -1)
    {
        ERROR_LOG("server recv from client [%s(%s : %d)] upload file:[%s] data error!", pt->cli_info.cli_username, pt->cli_info.cli_ip,pt->cli_info.cli_port, pt->filePath);
        return -1;
    }

    //将数据写入文件
    if(send_data(fd, buff, send_len) == -1)
    {
        ERROR_LOG("server write client [%s(%s : %d)] upload file:[%s] data error!", pt->cli_info.cli_username, pt->cli_info.cli_ip,pt->cli_info.cli_port, pt->filePath);
        return -1;
    }


    //重置文件偏移量
    pt->offset += send_len;

    //关闭文件描述符
    close(fd);

    return 0;
}

//传输目录
int trans_dirent(task_t *pt)
{
    fileList_t *pDirents = NULL;
    int count = 0;

    //获取目录文件列表的头指针
    fileList_t *head = read_dir(pt->filePath, 0);               
    fileList_t *p = head;

    //获取目录中文件的个数
    while(p)
    {
        p = p->next;
        count++;
    }

    //申请内存存放文件列表
    pDirents = (fileList_t *)malloc(sizeof(fileList_t) * count);
    memset(pDirents, 0x00, sizeof(fileList_t)*count);
    fileList_t *pD = pDirents;
    p = head;

    //将文件列表中的内容拷贝到申请的内存中
    while(p)
    {
        memcpy(pD, p, sizeof(fileList_t));
        p = p->next;
        pD++;
    }

    //发送请求文件列表的回应
    seq_head_t rsp_head;
    rsp_head.type = RSP_LIST;
    rsp_head.length = sizeof(fileList_t) * count;  

    //发送回复头信息
    if(send_data(pt->cli_info.cli_fd, (char *)&rsp_head, sizeof(rsp_head)) == -1)
    {
        return -1;
    }

    //发送文件列表
    if(send_data(pt->cli_info.cli_fd, (char *)pDirents, rsp_head.length) == -1)
    {
        return -1;
    }

    //释放文件列表内存
    p = head;
    fileList_t *del = p;
    while(p)
    {
        p = p->next;
        free(del);
        del = p;
    }
    free(pDirents);
    pDirents = NULL;

    return 0;
}

//删除任务结点并修改指针
void delete_task_and_change_point(TransTask_t *trans_task, task_t **pt, task_t **pre)
{
    //删除任务结点
    task_t *del = *pt; 
    *pt = (*pt)->next;
    (*pre)->next = *pt;
    TransTask_Del_Task(trans_task, del);
}

//处理所有客户端传输文件的线程函数
//@arg : 将任务控制结构结构作为参数传递进来
void *trans_route(void *arg)
{
    //从参数获取传输控制结构
    TransTask_t *trans_task = (TransTask_t *)arg;

    while(1)
    {
        //如果任务队列为空，即当前无任务，则处理线程处于等待状态
        if(trans_task->head == NULL)
        {
            printf("任务处理线程正在等待传输任务。。。\n");
            condition_wait(&trans_task->ready);
        }

        task_t *pt = trans_task->head;
        task_t *pre = pt;    //pt的前一个结点指针，为了删除方便

        //轮询每个任务结点，使其每一个结点的任务只传输一部分
        while(1)
        {
            if(pt != NULL)          
            {
                //如果是目录传输
                if(pt->isDirent == 1)
                {
                    //传输目录
                    trans_dirent(pt);
                    printf("目录传送完毕\n");
                    //删除任务结点并修改指针
                    delete_task_and_change_point(trans_task, &pt, &pre);
                    continue;
                }

                //如果是文件传输
                else if(pt->isDirent == 0)
                {
                    if(trans_file(pt) == -1)
                    {
                        //删除任务结点并修改指针
                        delete_task_and_change_point(trans_task, &pt, &pre);
                        continue;
                    }

                    //当该客户端请求的文件传输完毕时
                    if(pt->offset == pt->file_size)
                    {
                        printf("已经发送完毕\n");

                        //删除任务结点并修改指针
                        delete_task_and_change_point(trans_task, &pt, &pre);
                    }
                    else
                    {
                        pt = pt->next;
                        pre = pt;
                    }
                }
                //如果是上传文件
                else if(pt->isDirent == 2)
                {
                    if(trans_upload_file(pt) == -1)
                    {
                        //删除任务结点并修改指针
                        delete_task_and_change_point(trans_task, &pt, &pre);
                        continue;
                    }

                    //当该客户端请求的文件传输完毕时
                    if(pt->offset == pt->file_size)
                    {
                        printf("已经接受上传完毕\n");

                        //将客户端描述符再次添加到监听列表中
                        clientInfo_t *newCli = (clientInfo_t *)malloc(sizeof(clientInfo_t));
                        memset(newCli, 0x00, sizeof(clientInfo_t));
                        memcpy(newCli, &pt->cli_info, sizeof(clientInfo_t));
    
                        struct epoll_event ev;
                        ev.events = EPOLLIN;
                        ev.data.ptr = newCli;
                        epoll_ctl(trans_task->epfd, EPOLL_CTL_ADD, newCli->cli_fd, &ev);

                        //删除任务结点并修改指针
                        delete_task_and_change_point(trans_task, &pt, &pre);

                    }
                    else
                    {
                        pt = pt->next;
                        pre = pt;
                    }
                }
                if(pt == NULL)
                {
                    //当所有任务执行完毕之后，又进入等待状态
                    if(trans_task->head == NULL)
                    {
                        break;
                    }

                    //再次从头指针开始
                    pt = trans_task->head;
                    pre = pt;
                }

            }
            else
            {
                //当所有任务执行完毕之后，又进入等待状态
                if(trans_task->head == NULL)
                {
                    break;
                }
            }
        }
    }
}

//初始化传输任务控制结构
//@trans_task: 任务控制结构指针
void TransTask_Init(TransTask_t *trans_task)
{
    //初始化条件变量
    condition_init(&(trans_task->ready));

    //任务队列为空
    trans_task->head = NULL;
    trans_task->last = NULL;

    //创建一个epoll的监听集合
    if((trans_task->epfd = epoll_create(MAX_LISTEN)) == -1)
    {
        ERROR_LOG("server epoll_create error: %s", strerror(errno));
        exit(1);
    }

    //创建处理任务的线程
    if(-1 == pthread_create(&trans_task->transThread, NULL, trans_route, trans_task))
    {
        ERROR_LOG("server pthread_create error: %s", strerror(errno));
        exit(1);
    }
}

//向任务队列中添加下载任务
//@trans_task: 任务控制结构指针
//@cli_fd : 请求文件的客户端的通信套接字描述符
//@filePath: 请求的文件路径名
void TransTask_Add_DownLoadTask(TransTask_t *trans_task, clientInfo_t *pCli_info, char *filePath)
{
    //配置任务结点
    task_t *new_task = (task_t *)malloc(sizeof(task_t));
    memset(new_task, 0x00, sizeof(task_t));
    new_task->cli_info = *pCli_info;
    //printf("添加上传任务 %d\n", new_task->cli_info.cli_fd);
    new_task->offset = 0;

    //拷贝文件路径名
    memcpy(new_task->filePath, filePath, strlen(filePath));

    //判断是否为目录
    struct stat st;
    if(-1 == stat(filePath, &st))
    {
        ERROR_LOG("server get stat of client [%s(%s : %d)] download file:[%s] error:%s!", pCli_info->cli_username, pCli_info->cli_ip, pCli_info->cli_port, filePath, strerror(errno));
        return;
    }
    new_task->isDirent = S_ISDIR(st.st_mode);

    //next置为NULL
    new_task->next = NULL;

    //获取文件大小
    new_task->file_size = st.st_size;
    
    //如果队列为空
    if(trans_task->head == NULL)
    {
        trans_task->head = new_task;   
    }
    else
    {
        trans_task->last->next = new_task;
    }

    //修正尾指针
    trans_task->last = new_task;

    //处理任务线程可能因为没有任务执行处于等待状态，发送一个唤醒信号
    condition_signal(&trans_task->ready);
}

//向任务队列中添加上传任务
//@trans_task: 任务控制结构指针
//@cli_fd : 请求文件的客户端的通信套接字描述符
//@file_head: 上传的文件头(包括文件名和文件大小)
void TransTask_Add_UploadTask(TransTask_t *trans_task, clientInfo_t *pCli_info, file_head_t *file_head)
{
    //配置任务结点
    task_t *new_task = (task_t *)malloc(sizeof(task_t));
    memset(new_task, 0x00, sizeof(task_t));
    new_task->cli_info = *pCli_info;

    //释放原有资源
    free(pCli_info);
    pCli_info = NULL;

    new_task->offset = 0;

    //拷贝文件路径名
    memcpy(new_task->filePath, file_head->fileName, strlen(file_head->fileName));

    //next置为NULL
    new_task->next = NULL;

    //设置上传标记
    new_task->isDirent = 2;

    //获取文件大小
    new_task->file_size = file_head->fileLen;
    
    //如果队列为空
    if(trans_task->head == NULL)
    {
        trans_task->head = new_task;   
    }
    else
    {
        trans_task->last->next = new_task;
    }

    //修正尾指针
    trans_task->last = new_task;

    //处理任务线程可能因为没有任务执行处于等待状态，发送一个唤醒信号
    condition_signal(&trans_task->ready);
}


//删除任务队列中的指定任务
//@trans_task: 任务控制结构指针
//del_task   : 指定任务指针
void TransTask_Del_Task(TransTask_t *trans_task, task_t *del_task)
{
    //任务队列只有一个结点标记
    int oneNodeFlag = 0;

    task_t *pre = trans_task->head;
    task_t *cur = pre;

    //任务队列只有一个结点
    if((trans_task->last == del_task) && (trans_task->last == trans_task->head))
    {
        oneNodeFlag = 1;
        cur = del_task;
        trans_task->last = trans_task->head = NULL;
    }


    while(cur != NULL && !oneNodeFlag)
    {
        if(cur == del_task)
        {
            //如果删除的是最后一个结点
            if(trans_task->last == cur)
            {
                //重新修正last指针
                trans_task->last = pre;
                trans_task->last->next = NULL;
                break;
            }

            //如果删除的是第一个结点
            else if(trans_task->head == cur)
            {
                //重新修正head指针
                trans_task->head = cur->next; 
                break;
            }

            //删除的中间结点
            else
            {
                pre->next = cur->next;
                break;
            }
        }
        pre = cur;
        cur = cur->next;
    }

    //找到了删除结点
    if(cur != NULL)
    {
        free(cur);
        cur = NULL;
    }
}

