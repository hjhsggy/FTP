/*************************************************************************
	> File Name: mysql_c.c
	> Author: WishSun
	> Mail: WishSun_Cn@163.com
	> Created Time: 2018年01月30日 星期二 08时26分58秒
 ************************************************************************/

//数据库调用实现模块******************************************************
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <mysql_c.h>
#include <fileStruct.h>

//执行sql语句
//返回值: TRUE :成功
//        FALSE:失败
int query_sql(MYSQL **p_mysql, char *sql)
{
    *p_mysql = mysql_init(NULL);
    if(p_mysql == NULL)
    {
        printf("mysql_init error:%s\n", mysql_error(*p_mysql));
        return FALSE;
    }

    //连接数据库服务器
    if(mysql_real_connect(*p_mysql, "127.0.0.1", "root", "123456", NULL, 0, NULL, 0) == NULL)
    {
        printf("connect mysql server error : %s\n", mysql_error(*p_mysql));
        return FALSE;
    }

    //选择数据库
    if(mysql_select_db(*p_mysql, "ftpUser") != 0)
    {
        printf("select database error: %s\n", mysql_error(*p_mysql));
        return FALSE;
    }

    //执行sql语句
    if(mysql_query(*p_mysql, sql) != 0)
    {
        printf("query sql error:%s\n", mysql_error(*p_mysql));

        return FALSE;
    }
    return TRUE;
}

//获取结果的行数
int get_result_num(MYSQL *p_mysql)
{
    int num;

    //获取上次SQL语句的执行结果
    MYSQL_RES *p_res = NULL;
    p_res = mysql_store_result(p_mysql);
    if(p_res == NULL)
    {
        printf("store result error:%s!\n", mysql_error(p_mysql));
        return -1;
    }

    num = mysql_num_rows(p_res);

    mysql_free_result(p_res);
    return num;
}

//关闭数据库连接
void close_database(MYSQL *p_mysql)
{
    mysql_close(p_mysql);
}

