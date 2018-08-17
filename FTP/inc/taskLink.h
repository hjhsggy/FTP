/*************************************************************************
	> File Name: taskLink.h
	> Author: 
	> Mail: 
	> Created Time: 2018年01月27日 星期六 20时18分52秒
 ************************************************************************/

#ifndef _TASKLINK_H

#include "fileStruct.h"

//处理所有客户端传输文件的线程函数
//@arg : 将任务控制结构结构作为参数传递进来
void *trans_route(void *arg);

//初始化传输任务控制结构
//@trans_task: 任务控制结构指针
void TransTask_Init(TransTask_t *trans_task);

//向任务队列中添加下载任务
//@trans_task: 任务控制结构指针
//@cli_fd : 请求文件的客户端的通信套接字描述符
//filePath: 请求的文件路径名
void TransTask_Add_DownLoadTask(TransTask_t *trans_task, clientInfo_t *pCli_info, char *filePath);

//向任务队列中添加上传任务
//@trans_task: 任务控制结构指针
//@cli_fd : 请求文件的客户端的通信套接字描述符
//filePath: 请求的文件路径名
void TransTask_Add_UploadTask(TransTask_t *trans_task, clientInfo_t *pCli_info, file_head_t *file_head);

//删除任务队列中的指定任务
//@trans_task: 任务控制结构指针
//del_task   : 指定任务指针
void TransTask_Del_Task(TransTask_t *trans_task, task_t *del_task);


#define _TASKLINK_H
#endif
