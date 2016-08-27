#include "mysql_connect.h"

sql_api::sql_api(const string& host, const string& user, const string& passwd, const string& db)
    :_host(host)
    ,_user(user)
    ,_passwd(passwd)
    ,_db(db)
{
    res = NULL;
    mysql_base = mysql_init(NULL);
}
//连接一个MYSQL服务器
bool sql_api::begin_connect()
{
    if(mysql_real_connect(mysql_base, _host.c_str(), _user.c_str(), _passwd.c_str(), _db.c_str(), 3306, NULL, 0) == NULL)
    {
        cerr<<"connect error"<<endl;
        return false;
    }
    else
    {
        cout<<"connect done"<<endl;
    }
    return true;
}
//关闭一个服务器链接
bool sql_api::close_connect()
{
    mysql_close(mysql_base);
    cout<<"connect close"<<endl;
}
bool sql_api::select_sql(string fd_name[], string out_data[][5], int& out_row)
{
   string sql = "SELECT * FROM student_five_class";
   if(mysql_query(mysql_base, sql.c_str()) == 0)
   {
        //执行指定为一个空结尾的字符串的SQL查询
        cout<< "query success"<<endl;
   }
   else
   {
       cerr<<"query failed"<<endl;
       return false;
   }
   res = mysql_store_result(mysql_base);
   //通过检查mysql_store_result()是否返回0，可检查查询是否没有结果集
   int row_num = mysql_num_fields(res);
   //返回一个结果集合中的行的数量
   int fd_num = mysql_num_fields(res);
   //返回结果集中字段的数
   out_row = row_num;
   MYSQL_FIELD *fd = NULL;
   //这个结构包括字段信息，例如字段名、类型和大小。其成员在下面更详细的描述
   //你可以通过重复 调用mysql_fetch_field()对每一个获得MYSQL_FIELD结构。
   //字段值不是这个结构的部分，他们被包含在一个MYSQL_ROW结构中
   int i =0 ;
   for(i = 0; fd = mysql_fetch_field(res);)
   {
       //从结果集中取得列信息并作为对象返回
       //cout<< fd->name<<" \t";
       fd_name[i++] = fd->name;
   }
   //cout<<endl;
   //
   //这是一个行数据的类型安全（type->safe）的表示。当前它实现为一个计数字节
   //的字符串数组。（如果字段值包含二进制数据，你不能将这些视为空终止串。
   //因为这样的值可以在内部包含空字节）行通过调用mysql_fetch_row()获得。
   for(int index = 0; index < row_num; index++)
   {
        MYSQL_ROW _row = mysql_fetch_row(res);
        if(_row)
        {
            int start = 0;
            for(; start <fd_num; ++start)
            {
                out_data[index][start] = _row[start];
            }
        }
   }
   return true;
}
bool sql_api::insert_sql(const string &data)
{
    string sql = "INSERT INTO student_five_class(name, age, school, hobby)value";
    sql +="(";
    sql += data;
    sql += ");";
    if(mysql_query(mysql_base, sql.c_str()) == 0)
    {
        //执行指定为一个空结尾的字符窜的SQL查询
        cout<<"query success"<<endl;
        return true;
    }
    else
    {
        cerr<<"query failed"<<endl;
        return false;
    }
}   
sql_api::~sql_api()
{
   close_connect();
    if(res)
    {
        free(res);
    }
}
void sql_api::show_info()
{
    //取得MYSQL客户端信息
    cout<<mysql_get_client_info()<<endl;
}
#ifdef _DEBUG_
int main()
{
    string _sql_data[1024][5];
    string header[5];
    int curr_row = -1;
    const string _host = "127.0.0.1";
    const string _user = "root";
    const string _passwd = "";
    const string _db = "remote_db";
    const string _data = "\"mnt\", 19, \"shankeda\", \"sleeping\"";
    sql_api conn(_host, _user, _passwd, _db);
    conn.begin_connect();
    conn.select_sql(header, _sql_data, curr_row);
    sleep(1);
    int i = 0;
    for(; i < 5; i++)
    {
        cout<<header[i]<<endl;
    }
    cout<<endl;
    int j = 0;
    for(i = 0; i < curr_row; i++)
    {
        for(j = 0;j < 5; j++)
        {
            cout<<_sql_data[i][j]<<"\t";
        }
        cout<<endl;
    }
    conn,show_info();
    return 0;
}
#endif
