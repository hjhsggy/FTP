/*************************************************************************
	> File Name: login_ftp.c
	> Author: WishSun
	> Mail: WishSun_Cn@163.com
	> Created Time: 2018年01月30日 星期二 08时26分16秒
 ************************************************************************/

//登录实现模块******************************************************
#include <stdio.h>
#include <login_ftp.h>
#include <fileStruct.h>

//登录ftp服务器
//@username : 用户名
//@password : 密码
//返回值: 登录成功返回TRUE
//        登录失败返回FALSE
int login_ftp_server(char *username, char *password)
{
    MYSQL *p_mysql = NULL;
    char sql_buff[BUFF_MAX] = {0};

    sprintf(sql_buff, "select * from account where username='%s' and password='%s';", username, password);

    if(query_sql(&p_mysql, sql_buff) == FALSE)
    {
        close_database(p_mysql);
        return FALSE;
    }

    if(get_result_num(p_mysql) > 0)
    {
        close_database(p_mysql);
        return TRUE;
    }
    else
    {
        close_database(p_mysql);
        return FALSE;
    }
}

//注册ftp客户端账户
//@username : 用户名
//@password : 密码
//返回值: 注册成功返回TRUE
//        注册失败返回FALSE
int register_ftp_account(char *username, char *password)
{
    MYSQL *p_mysql = NULL;
    char sql_buff[BUFF_MAX] = {0};

    sprintf(sql_buff, "insert into account value('%s', '%s');", username, password);

    if(query_sql(&p_mysql, sql_buff) == FALSE)
    {
        close_database(p_mysql);
        return FALSE;
    }

    return TRUE;
}



