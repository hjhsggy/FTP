/*************************************************************************
	> File Name: client.c
	> Author: 
	> Mail: 
	> Created Time: 2018年01月25日 星期四 10时04分01秒
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <fileStruct.h>
#include <libconfig.h>
#include <common.h>


//从路径中获取文件名
void GetFileNameFromPath(char *path, char *filename)
{
    char *begin = path + strlen(path) - 1;
    while((begin != path) && (*begin != '/'))
    {
        begin--;
    }
    if(begin == path)
    {
        memcpy(filename, path, strlen(path));
        return;
    }

    memset(filename, 0x00, strlen(filename));
    begin++;

    int i = 0;
    while(*begin != '\0')
    {
        filename[i++] = *begin;
        begin++;
    }
}


//处理下载文件的回应
void deal_rsp_download_file(int sock_fd, seq_head_t rsp_head, char *filename)
{
    file_head_t file_head;

    //从服务器接收文件头
    if(recv_data(sock_fd, (char *)&file_head, rsp_head.length) == -1)
    {
        printf("recv file head error\n");
        return;
    }

    //创建接收目录
    if(mkdir("./recvFile", 0777) == -1)
    {
        if(errno != EEXIST)
        {
            perror("mkdir");
            return;
        }
    }

    //从路径中获取文件名
    GetFileNameFromPath(file_head.fileName, filename);

    //从服务器接收文件数据信息
    char recvFile[MAX_PATH] ={0};
    sprintf(recvFile, "./recvFile/%s", filename);

    int fd;
    //打开一个新文件来存储接收的文件数据，如果文件存在则清空文件数据
    if((fd = open(recvFile, O_WRONLY | O_CREAT | O_TRUNC, 0664)) == -1)
    {
        perror("open");
        return;
    }
    int write_len = 0;
    char buff[BUFF_MAX];
    printf("请求文件大小为: %ld字节\n", file_head.fileLen);

    printf("\n已接收");
    printf("  0%%");
    while(write_len < file_head.fileLen)
    {
        double flen = write_len;
        printf("\b\b\b\b%3.0f%%", (flen/file_head.fileLen) * 100);

        //计算出还要接收的字节数len
        int len = file_head.fileLen - write_len;
        len = len > BUFF_MAX ? BUFF_MAX : len;

        //从服务器接收len字节数据
        if(recv_data(sock_fd, buff, len) == -1)
        {
            printf("recv file data error\n");
            return;
        }

        //将接收的文件数据写入文件
        write(fd, buff, len);
        write_len += len;
    }

    printf("\b\b\b\b%3d%%", 100);
    printf("\n接收完毕!\n");
    close(fd);
}

 //处理对上传文件的回应
void deal_rsp_upload_file(int sock_fd, file_head_t file_head, char *filename)
{
    
    int fd;
    //打开文件，向服务器上传数据
    if((fd = open(filename, O_RDONLY)) == -1)
    {
        perror("open");
        return;
    }

    int upload_len = 0;
    char buff[BUFF_MAX];
    printf("上传文件大小为: %ld字节\n", file_head.fileLen);

    printf("\n已上传");
    printf("  0%%");
    while(upload_len < file_head.fileLen)
    {
        double flen = upload_len;
        printf("\b\b\b\b%3.0f%%", (flen/file_head.fileLen) * 100);

        //计算出还要上传的字节数len
        int len = file_head.fileLen - upload_len;
        len = len > BUFF_MAX ? BUFF_MAX : len;

        //从文件中读取len字节数据
        if(recv_data(fd, buff, len) == -1)
        {
            printf("recv file data error\n");
            return;
        }

        //将从文件中读到的len字节数据上传给服务器
        if(send_data(sock_fd, buff, len) == -1)
        {
            printf("send file data error\n");
            return;
        }

        upload_len += len;
    }

    printf("\b\b\b\b%3d%%", 100);
    printf("\n上传完毕!\n");
    close(fd);

}   

//处理请求文件列表的回应
void deal_rsp_fileList(int sock_fd, seq_head_t rsp_head)
{
    printf("服务器文件列表开始:------------------------------------\n");
    fileList_t *pFileList = malloc(rsp_head.length);
    int len = rsp_head.length / sizeof(fileList_t);

    if(recv_data(sock_fd, (char *)pFileList, rsp_head.length) == -1)
    {
        printf("recv_data fileList error\n");
        return;
    }


    int i = 0;
    for(i = 0; i < len; i++)
    {
        printf("\t%s----------------%ld字节\n", pFileList[i].filePath, pFileList[i].fileSize);
    }

    printf("服务器文件列表结束:------------------------------------\n");
    free(pFileList);
    pFileList = NULL;
}

//请求登录或注册服务器
int login_register_server(int sock_fd, account_t *account, int seq_type)
{
    seq_head_t  seq_head;
    seq_head_t  rsp_head;
    seq_head.length = sizeof(account_t);
    seq_head.type = seq_type;

    //发送请求
    if(send_data(sock_fd, (char *)&seq_head, sizeof(seq_head_t)) == -1)
    {
        printf("send SEQ error\n");
        return FALSE;
    }
    
    //发送账户信息
    if(send_data(sock_fd, (char *)account, sizeof(account_t)) == -1)
    {
        printf("send account error\n");
        return FALSE;
    }

    //从服务器接收对请求的回应
    if(recv_data(sock_fd, (char *)&rsp_head, sizeof(seq_head_t)) == -1)
    {
        printf("recv file head error\n");
        return FALSE;
    }

    //判断回应是否正确
    if(rsp_head.type == RSP_LOGIN)
    {
        if(seq_type == SEQ_REGISTER)
        {
            printf("注册成功！");
            printf("你的用户名为: %s\t你的密码为: %s\t\n",account->username, account->password );
            printf("请按\"回车\"进行登录!\n");
            getchar();
        }
        printf("登录成功\n");
        return TRUE;
    }
    else
    {
        printf("登录失败\n");
        return FALSE;
    }
}
    

//发送请求给服务器
void send_seq_toserver(int sock_fd, char *filename, int seq_type, char *username)
{
    seq_head_t  seq_head;
    seq_head_t  rsp_head;
    file_head_t file_head;
    memset(&file_head, 0x00, sizeof(file_head_t));
    memset(&seq_head, 0x00, sizeof(seq_head_t));
    memset(&rsp_head, 0x00, sizeof(seq_head_t));

    seq_head.type = seq_type;


    //如果是下载文件请求的话，需要将文件名传递过去长度传递过去
    if(seq_type == SEQ_FILE)
    {
        seq_head.length = strlen(filename);
    }

    //如果是上传文件的话, 接下来要将文件名和文件长度传递过去
    if(seq_type == SEQ_UPLOAD)
    {
        seq_head.length = sizeof(file_head_t);
    }

    //发送请求
    if(send_data(sock_fd, (char *)&seq_head, sizeof(seq_head_t)) == -1)
    {
        printf("send SEQ error\n");
        return;
    }


    //如果是下载文件请求的话，需要将文件名传递过去
    if(seq_type == SEQ_FILE)
    {
        //发送请求的文件名
        if(send_data(sock_fd, filename, seq_head.length) < -1)
        {
            printf("send filename error\n");
            return;
        }
    }

    //如果是上传文件的话，得把文件名和文件长度都传过去
    if(seq_type == SEQ_UPLOAD)
    {
        //获取文件属性
        struct stat st;
        if(stat(filename, &st) < 0)
        {
            perror("stat");
            return;
        }

        file_head.fileLen = st.st_size;

        
        char tmpPath[MAX_PATH] = {0};
        //提取出文件名放入文件头
        GetFileNameFromPath(filename, tmpPath);
        sprintf(file_head.fileName, "./%s/%s", username, tmpPath);
        printf("构造出的的文件名: %s\n", file_head.fileName);
        

        //发送上传的文件名及其大小
        if(send_data(sock_fd, (char *)&file_head, sizeof(file_head_t)) < -1)
        {
            printf("send filename error\n");
            return;
        }
    }

    //从服务器接收对请求的回应
    if(recv_data(sock_fd, (char *)&rsp_head, sizeof(seq_head_t)) == -1)
    {
        printf("recv file head error\n");
        return;
    }

    //处理所有的服务端回应
    switch(rsp_head.type)
    {
        //如果是文件下载的回应
        case RSP_FILE:
        {
            //处理下载文件的回应
            deal_rsp_download_file(sock_fd, rsp_head, filename);
            break;
        }
        //如果是对上传文件请求的回应
        case RSP_UPLOAD:
        {
            //处理对上传文件的回应
            deal_rsp_upload_file(sock_fd, file_head, filename);
            break;
        }

        //如果是文件列表请求的回应
        case RSP_LIST:
        {
            //处理文件列表请求的回应
            deal_rsp_fileList(sock_fd, rsp_head);
            break;
        }

        //如果请求下载的文件或列表不存在
        case RSP_NOEXIST:
        {
            printf("sorry!您所请求的文件或目录不存在，请确认后重新请求！\n");
            break;
        }

        //错误的回应
        default:
        {
            break;
        }
    }
}


//判断上传文件是否合法
int judge_upload_istrue(char *argu)
{
    if(argu == NULL)
    {
        return FALSE;
    }

    if(access(argu, F_OK) != 0)
    {
        printf("sorry! 您所要上传的文件不存在，请确认后重新上传！\n");
        return FALSE;
    }

    //获取文件属性
    struct stat st;
    if(stat(argu, &st) < 0)
    {
        perror("stat");
        return FALSE;
    }

    //判断是否为目录文件
    if(S_ISDIR(st.st_mode))
    {
        printf("sorry! 本程序暂时不支持传输目录，请确认是文件后重新上传！\n");
        return FALSE;
    }

    return TRUE;
}


//登录注册界面
void Login_UI(int sock_fd, account_t *account)
{
    memset(account, 0x00, sizeof(account_t));
    int cmd;

    char *delims = " ";
    char *result = NULL;
    char password[7] = {0};
    char username[33] = {0};

    while(1)
    {
        printf("请输入选择功能编号:  1. 登录\t\t2.注册\t\t3.退出程序\n");
        printf("input>> ");
        scanf("%d", &cmd);
        getchar();

        switch(cmd)
        {
            case 1:
            {
                printf("请输入用户名: >> ");
                scanf("%s",account->username);
                getchar();

                printf("请输入密码: >> ");
                scanf("%s", account->password);
                getchar();
                
                printf("输入的用户名是%s, 密码是%s\n", account->username,account->password);

                if(login_register_server(sock_fd, account, SEQ_LOGIN) == TRUE)
                {
                    return;
                } 
                break;
            }
            case 2:
            {
                printf("请输入用户名: >> ");
                scanf("%s", account->username);
                getchar();

                printf("请输入密码: >> ");
                scanf("%s", account->password);
                getchar();
                
                printf("请再次输入密码: >> ");
                scanf("%s", password);
                getchar();

                if(strncasecmp(password, account->password, strlen(password)))
                {
                    printf("两次密码输入不同！请重新输入");
                }

                //去服务器上注册
                if(login_register_server(sock_fd, account, SEQ_REGISTER) == TRUE)
                {
                    return;
                }
                break;
            }
            case 3:
            {
                printf("已退出程序\n");
                exit(0);
            }
            default:
            {
                printf("请输入正确的编号!\n");
                break;
            }
        }
       
        printf("账号或密码错误！请重新登录\n");
    }
   
}

int main(void)
{
    int sock_fd;
    account_t account;

    char sev_IP[16] = {0};
    int  sev_port = -1;

    //解析配置文件获取服务端IP和端口号
    if(parse_conf(NULL, sev_IP, &sev_port) == -1)
    {
        exit(1);
    }

    //与服务器建立连接
    if((sock_fd = socket_connect(sev_IP, sev_port)) == -1)
    {
        exit(1);
    }


    Login_UI(sock_fd, &account);

    printf("命令说明:\n\tshow : 显示服务器端文件名和大小\n\tget + 文件名 : 从服务器下载改文件到当前目录\n\tupload : 向服务器上传文件\n\thelp : 显示命令说明\n\tquit : 退出程序\n");
    char input[1024] = {0};
    char cmdBuff[256] = {0};
    char argu[MAX_PATH] = {0};
    char *delims = " ";
    char *result = NULL;
    while(1)
    {
        printf("client input>> ");
        memset(input, 0x00, sizeof(input));
        memset(argu, 0x00, sizeof(argu));

        if(fgets(input, sizeof(input), stdin) != NULL)
        {
            //去掉最后的回车
            input[strlen(input) - 1] = '\0';

            //分割字符串
            if(( result = strtok(input, delims)) == NULL)
            {
                continue;
            }

            //获取命令
            memcpy(cmdBuff, result, strlen(result));
           
            if(!strncasecmp(cmdBuff, "show", strlen("show")))
            {
                //发送文件列表请求
                send_seq_toserver(sock_fd, NULL, SEQ_LIST, NULL);
            }
            else if(!strncasecmp(cmdBuff, "get", strlen("get")))
            {
                //获取参数
                if((result = strtok(NULL, delims)) == NULL)
                {
                    printf("参数错误！请重新输入:(get + 文件名)\n");
                    continue;
                }

                memcpy(argu, result, strlen(result));
                printf("请求的文件名是: %s\n", argu);

                send_seq_toserver(sock_fd, argu, SEQ_FILE, NULL);
            }
            else if(!strncasecmp(cmdBuff, "help", strlen("help")))
            {
                printf("命令说明:\n\tshow : 显示服务器端文件名和大小\n\tget + 文件名 : 从服务器下载改文件到当前目录\n\tupload : 向服务器上传文件\n\thelp : 显示命令说明\n\tquit : 退出程序\n");
            }
            else if(!strncasecmp(cmdBuff, "quit", strlen("quit")))
            {
                close(sock_fd);
                exit(0);
            }
            else if(!strncasecmp(cmdBuff, "upload", strlen("upload")))
            {
                //获取参数
                if((result = strtok(NULL, delims)) == NULL)
                {
                    printf("参数错误！请重新输入:(upload + 文件名)\n");
                    continue;
                }

                memcpy(argu, result, strlen(result));
                printf("正在上传文件: %s\n", argu);

                //判断上传文件是否合法
                if(judge_upload_istrue(argu))
                {
                    send_seq_toserver(sock_fd, argu, SEQ_UPLOAD, account.username);
                }
            }
            else
            {
                printf("命令无效！请重新输入\n");
            }
        }
    }


    return 0;
}
