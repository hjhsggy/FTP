/*************************************************************************
	> File Name: mysql_c.h
	> Author: WishSun
	> Mail: WishSun_Cn@163.com
	> Created Time: 2018年01月30日 星期二 08时27分20秒
 ************************************************************************/

//数据库调用声明模块******************************************************
#ifndef _MYSQL_C_H
#include <mysql/mysql.h>

//执行sql语句
//返回值: TRUE :成功
//        FALSE:失败
int query_sql(MYSQL **p_mysql, char *sql);

//获取结果的行数
int get_result_num(MYSQL *p_mysql);

//关闭数据库连接
void close_database(MYSQL *p_mysql);

#define _MYSQL_C_H
#endif
