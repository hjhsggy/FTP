/*************************************************************************
	> File Name: common.h
	> Author: 
	> Mail: 
	> Created Time: 2018年01月27日 星期六 17时56分16秒
 ************************************************************************/

#ifndef _SEND_RECV_DATA_H

/*对发送数据或向文件中写入数据进行封装
 * @fd : 套接字描述符或文件描述符 
 * @buff: 将要写入的数据的地址
 * @len: 将要写入的数据的长度
 *
 * 返回值: 成功返回0
 *         失败返回-1
 */
int send_data(int fd, char *buff, int len);


/*对接收数据或从文件中读取数据进行封装
 * @fd : 套接字描述符或文件描述符 
 * @buff: 将要写入的数据的地址
 * @len: 将要写入的数据的长度
 *
 * 返回值: 成功返回0
 *         失败返回-1
 */
int recv_data(int fd, char *buff, int len);


/*对创建监听套接字进行封装
 * @ip_address: 服务器IP
 * @port: 服务器端口
 * @listen_num: 服务器同时最大监听数
 *
 * 返回值: 成功返回创建好的监听套接字描述符
 *         失败返回-1
 */
int socket_create_listen_fd(char *ip_address, int port, int listen_num);


/*对接收客户端连接进行封装
 * @lst_fd: 服务器监听套接字描述符
 *
 * 返回值: 成功返回与新客户端通信的套接字描述符
 *         失败返回-1
 */
int socket_accept(int lst_fd);


/*对连接服务器操作进行封装
 * @sock_fd : 客户端套接字描述符
 * @ip_address: 服务器IP
 * @port: 服务器端口
 *
 * 返回值: 成功返回0
 *         失败返回-1
 */
int socket_connect(char *ip_address, int port);

/*解析配置文件, 获取日志配置文件路径、服务端IP、端口
 * @log_cfg_path: 接收日志配置文件路径
 * @sev_IP: 接收服务端IP
 * @sev_port: 接收服务端port
 * 返回值: 成功返回0
 *         失败返回-1
 */
int parse_conf(char *log_cfg_path, char *sev_IP, int *sev_port);

/*在命令行读取输入一行输入
 *@buff: 接收输入的缓冲区地址
 *@size: 缓冲区大小
 *
 *返回值: 无
 */
void read_input(char *buff, int size);

#define _SEND_RECV_DATA_H
#endif
