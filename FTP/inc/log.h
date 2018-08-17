#ifndef __SL_LOG_H__
#define __SL_LOG_H__

#include <zlog.h>

extern zlog_category_t *log_handle;

/*致命错误日志*/
#define FATAL_LOG(...) zlog_fatal(log_handle, __VA_ARGS__)
/*普通操作日志*/
#define INFO_LOG(...) zlog_info(log_handle, __VA_ARGS__)
/*操作警告日志*/
#define WARNING_LOG(...) zlog_warn(log_handle, __VA_ARGS__)
/*调试操作日志*/
#define DEBUG_LOG(...) zlog_debug(log_handle, __VA_ARGS__)
/*错误操作日志*/
#define ERROR_LOG(...) zlog_error(log_handle, __VA_ARGS__)

/*获取操作日志文件句柄
 * @conf_path: 日志配置文件路径
 * @mode : 使用配置文件中的哪种日志格式
 * 返回值: 出错返回-1
 *         成功返回0
 */
int get_log_handle(char *log_cfg_path, char *mode);

/*关闭日志
 */
void close_log();

#endif
