/*************************************************************************
	> File Name: common.c
	> Author: 
	> Mail: 
	> Created Time: 2018年01月27日 星期六 17时55分15秒
 ************************************************************************/

#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <common.h>
#include <libconfig.h>

/*对发送数据或向文件中写入数据进行封装
 * @fd : 套接字描述符或文件描述符 
 * @buff: 将要写入的数据的地址
 * @len: 将要写入的数据的长度
 *
 * 返回值: 成功返回0
 *         失败返回-1
 */
int send_data(int fd, char *buff, int len)
{
    int s_len = 0;
    int ret;

    while(s_len < len)
    {
        //将接受的数据放到已有数据后边: buff + r_len
        //接收长度为总长度len - 已经接收的长度r_len
        ret = write(fd, buff + s_len, len - s_len);
        if(ret <= 0)
        {
            if(errno == EAGAIN || errno == EINTR)
            {
                continue;
            }
            return -1;
        }
        s_len += ret;
    }
    return 0;
}

/*对接收数据或从文件中读取数据进行封装
 * @fd : 套接字描述符或文件描述符 
 * @buff: 将要写入的数据的地址
 * @len: 将要写入的数据的长度
 *
 * 返回值: 成功返回0
 *         失败返回-1
 */
int recv_data(int fd, char *buff, int len)
{
    int r_len = 0;
    int ret;

    while(r_len < len)
    {
        //将接受的数据放到已有数据后边: buff + r_len
        //接收长度为总长度len - 已经接收的长度r_len
        ret = read(fd, buff + r_len, len - r_len);
        if(ret <= 0)
        {
            if(ret == 0)
            {
                return 1;
            }
            else if(errno == EAGAIN || errno == EINTR)
            {
                continue;
            }
            return -1;
        }
        r_len += ret;
    }
    return 0;
}

/*对创建监听套接字进行封装
 * @ip_address: 服务器IP
 * @port: 服务器端口
 * @listen_num: 服务器同时最大监听数
 *
 * 返回值: 成功返回创建好的监听套接字描述符
 *         失败返回-1
 */
int socket_create_listen_fd(char *ip_address, int port, int listen_num)
{
    int lst_fd;

    if((lst_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        return -1;
    }

    //设置lst_fd可地址重用
    int option = 1;
    setsockopt(lst_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&option, sizeof(int));

    //配置监听地址和端口
    struct sockaddr_in sev_addr;
    sev_addr.sin_family = AF_INET;
    sev_addr.sin_port = htons(port);
    sev_addr.sin_addr.s_addr = inet_addr(ip_address);


    //将监听套接字绑定到指定地址和端口
    if(bind(lst_fd, (struct sockaddr *)&sev_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("bind");
        return -1;
    }

    //开始监听客户端
    if(listen(lst_fd, listen_num) == -1)
    {
        perror("listen");
        return -1;
    }

    return lst_fd;
}


/*对接收客户端连接进行封装
 * @lst_fd: 服务器监听套接字描述符
 *
 * 返回值: 成功返回与新客户端通信的套接字描述符
 *         失败返回-1
 */
int socket_accept(int lst_fd)
{
    int cli_fd;
    struct sockaddr_in cli_addr;
    int len = sizeof(struct sockaddr);

    if((cli_fd = accept(lst_fd, (struct sockaddr *)&cli_addr, &len)) == -1)
    {
        perror("accept");
        return -1;   
    }

    return cli_fd;
}


/*对连接服务器操作进行封装
 * @sock_fd : 客户端套接字描述符
 * @ip_address: 服务器IP
 * @port: 服务器端口
 *
 * 返回值: 成功与服务端通信的套接字描述符
 *         失败返回-1
 */
int socket_connect(char *ip_address, int port)
{
    int sock_fd;
    struct sockaddr_in sev_addr;
    int len = sizeof(struct sockaddr_in);


    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        return -1;
    }

    //配置监听地址和端口
    sev_addr.sin_family = AF_INET;
    sev_addr.sin_port = htons(port);
    sev_addr.sin_addr.s_addr = inet_addr(ip_address);

    //连接服务器
    if(connect(sock_fd, (struct sockaddr *)&sev_addr, len) == -1)
    {
        perror("connect");
        return -1;
    }

    return sock_fd;
}

/*解析配置文件, 获取日志配置文件路径、服务端IP、端口
 * @log_cfg_path: 接收日志配置文件路径
 * @sev_IP: 接收服务端IP
 * @sev_port: 接收服务端port
 * 返回值: 成功返回0
 *         失败返回-1
 */
int parse_conf(char *log_cfg_path, char *sev_IP, int *sev_port)
{
    config_t cfg;

    //初始化配置文件句柄
    config_init(&cfg);

    //打开配置文件，获取操作配置文件句柄
    if (!config_read_file(&cfg, "../../etc/trans_file.cfg")) {
        printf("read config file error!\n");
        return -1;
    }
    const char *ptr = NULL;

    if(log_cfg_path != NULL)
    {
        //获取日志文件路径
        if (!config_lookup_string(&cfg, "conf.log_cfg_path", &ptr)) {
            printf("get log file path error!\n");
            return -1;
        }
        memcpy(log_cfg_path, ptr, strlen(ptr));
    } 

    //获取服务器IP地址
    if (!config_lookup_string(&cfg, "conf.sev_IP", &ptr)) {
        printf("get sev_IP error!\n");
        return -1;
    }
    memcpy(sev_IP, ptr, strlen(ptr));

    //获取服务器端口号
    if (!config_lookup_int(&cfg, "conf.sev_port", sev_port)) {
        printf("get sev_port error!\n");
        return -1;
    }
    printf("log:[%s],addr:[%s], port[%d]\n", log_cfg_path, sev_IP, *sev_port);

    config_destroy(&cfg);
    return 0;
}


/*在命令行读取输入一行输入
 *@buff: 接收输入的缓冲区地址
 *@size: 缓冲区大小
 *
 *返回值: 无
 */
void read_input(char *buff, int size)
{
    char *n = NULL;
    memset(buff, 0x00, size);

    if(fgets(buff, size, stdin) != NULL)
    {
        n = strchr(buff, '\n');
        if(n)
        {
            *n = '\0';
        }
    }
}

