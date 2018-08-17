#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <log.h>

//定义zlog日志操作句柄
zlog_category_t *log_handle = NULL;

/*获取操作日志文件句柄
 * @conf_path: 日志配置文件路径
 * @mode : 使用配置文件中的哪种日志格式
 * 返回值: 出错返回-1
 *         成功返回0
 */
int get_log_handle(char *log_cfg_path, char *mode)
{
    if (!log_cfg_path) {
        fprintf(stdout, "log conf file is null\n");
        return -1;
    }
    //zlog初始化
    if (zlog_init(log_cfg_path)) {
        fprintf(stdout, "zlog_init error\n");
        return -1;
    }
    //获取日志操作句柄
    if ((log_handle = zlog_get_category(mode)) == NULL) {
        fprintf(stdout, "zlog_get_category error\n");
        return -1;
    }
    return 0;
}

/*关闭日志
 */
void close_log()
{
    zlog_fini();
}
