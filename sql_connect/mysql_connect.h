#pragma once
#include <iostream>
#include <string>
#include "mysql.h"
using namespace std;


#pragma comment(lib,"mysqlclient.lib")

class sql_api
{
    public:
        sql_api(const string &host ,const string &user, const string &passwd, const string &db);
        bool begin_connect();
        bool close_connect();
        bool insert_sql(const string &data);
        bool select_sql(string fd_name[], string out_data[][5], int &out_row);
        ~sql_api();
        void show_info();
    private:
        MYSQL_RES* res;
        //这个结构代表返回行的一个查询的（SELECT，SHOW，DESCRIBE，EXPLAIN）
        //的结果，从查询的信息在本章下文称为结果集合
        MYSQL *mysql_base;
        string _host;
        string _user;
        string _passwd;
        string _db;
}; 
