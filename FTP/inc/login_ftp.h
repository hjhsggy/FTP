/*************************************************************************
	> File Name: loginFIP.h
	> Author: WishSun
	> Mail: WishSun_Cn@163.com
	> Created Time: 2018年01月30日 星期二 08时25分09秒
 ************************************************************************/

//登录声明模块******************************************************
#ifndef _LOGINFIP_H
#include "mysql_c.h"

//登录ftp服务器
//@username : 用户名
//@password : 密码
//返回值: 登录成功返回TRUE
//        登录失败返回FALSE
int login_ftp_server(char *username, char *password);

//注册ftp客户端账户
//@username : 用户名
//@password : 密码
//返回值: 注册成功返回TRUE
//        注册失败返回FALSE
int register_ftp_account(char *username, char *password);



#define _LOGINFIP_H
#endif
