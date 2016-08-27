#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define _SIZE_ 1024

static void usage(const char* proc)
{
    printf("usage:%s[ip][port]\n", proc);
}
void print_log(const char*fun, int line, int err_no, const char *err_str)
{
    printf("[%s:&d][%d][%s]\n", fun, line, err_no, err_str);
}
void print_debug(const char* msg)
{
#ifdef _DEBUG_
    printf("%s", msg);
#endif
}
void bad_request(int sock)
{
    char buf[_SIZE_];
    sprintf(buf, "HTTP/1.1 400 BAD REQUEST\r\n");
    send(sock, buf, sizeof(buf), 0);
    sprintf(buf, "Content-type:text/html!\r\n");
    send(sock, buf, sizeof(buf), 0);
    sprintf(buf, "\r\n");
    send(sock, buf, sizeof(buf), 0);
    sprintf(buf, "<P>Your browser sent a bad request,");;
    send(sock, buf, sizeof(buf), 0);
    sprintf(buf, "such as a POST without a Content-Length.\r\n");
    send(sock, buf, sizeof(buf), 0);
}
void cannot_execute(int sock)
{
    char buf[_SIZE_];
    sprintf(buf, "HTTP/1.1 500 Internal Server Error\r\n");
    send(sock, buf, sizeof(buf), 0);
    sprintf(buf, "Content-type:text/html!\r\n");
    send(sock, buf, sizeof(buf), 0);
    sprintf(buf, "\r\n");
    send(sock, buf, sizeof(buf), 0);
    sprintf(buf, "<P>Erroe prohibited CGI execution.\r\n");
    send(sock, buf, sizeof(buf), 0);
}
void not_found(int sock)
{
    char buf[_SIZE_];
    /* 404 页面 */ 
    sprintf(buf, "HTTP/1.1 404 NOT FOUND\r\n"); 
    send(sock, buf, strlen(buf), 0); 
    /*服务器信息*/   
    sprintf(buf, "Content-Type: text/html\r\n");
    send(sock, buf, strlen(buf), 0); 
    sprintf(buf, "\r\n"); 
    send(sock, buf, strlen(buf), 0); 
    sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n"); 
    send(sock, buf, strlen(buf), 0); 
    sprintf(buf, "<BODY><P>The server could not fulfill\r\n"); 
    send(sock, buf, strlen(buf), 0); 
    sprintf(buf, "your request because the resource specified\r\n"); 
    send(sock, buf, strlen(buf), 0); 
    sprintf(buf, "is unavailable or nonexistent.\r\n");
    send(sock, buf, strlen(buf), 0);
    sprintf(buf, "</BODY></HTML>\r\n");
    send(sock, buf, strlen(buf), 0);
}
void unimplemented(int sock)
{
    char buf[1024]; 

    /* HTTP method 不被支持*/ 
    sprintf(buf, "HTTP/1.1 501 Method Not Implemented\r\n"); 
    send(sock, buf, strlen(buf), 0); 
    /*服务器信息*/ 
    sprintf(buf, "Content-Type: text/html\r\n"); 
    send(sock, buf, strlen(buf), 0); 
    sprintf(buf, "\r\n"); 
    send(sock, buf, strlen(buf), 0); 
    sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n"); 
    send(sock, buf, strlen(buf), 0); 
    sprintf(buf, "</TITLE></HEAD>\r\n"); 
    send(sock, buf, strlen(buf), 0); 
    sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n"); 
    send(sock, buf, strlen(buf), 0); 
    sprintf(buf, "</BODY></HTML>\r\n"); 
    send(sock, buf, strlen(buf), 0);
}
void echo_errno(int sock, int error_code)
    {
        switch(error_code)
        {
            case 400:
                bad_request(sock);
                break;
            case 404:
                not_found(sock);
                break;
            case 500:
                cannot_execute(sock);
                break;
            case 501:
                unimplemented(sock);
                break;
            default:
                perror("error_code");
                break;
        }
    }
//读取一行，不管原来是以\n还是以\r\n结束，全部转化为以\n加上\0字符结束
int get_line(int sock, char buf[], int len)
{
    if(!buf || len < 0)
    {
        return -1;
    }
    char c = '\0';
    int n = 0;
    int i = 0;
    while(i < len - 1 && c != '\n')
    {
        n = recv(sock, &c, 1, 0);//从sock中一次读一个字符，循环读
        if(n > 0)
        {
            if(c == '\r')//当读到\r
            {
                n = recv(sock, &c, 1, MSG_PEEK);//用MSG_PEEK不读区，窥探下一个字符
                //读一下个字符为\n
                if(n > 0 && c == '\n')
                {
                    recv(sock, &c, 1, 0);//继续读
                }
                else
                {
                    c = '\n';
                }
            }
            buf[i++] = c;
        }
        else
        {
            c = '\n';
        }
    }
    buf[i] = '\0';
    return i;//返回读取的字符数
}
//清除所有头部信息
static void clear_header(int sock)
{
    int ret = -1;
    char buf[_SIZE_];
    do
    {
        ret = get_line(sock, buf, sizeof(buf));
    }while(ret > 0 && (strcmp(buf, "\n") != 0));
}
void exec_cgi(int sock, const char* path, const char* method, const char* query_string)
{
    printf("AAAAAAAAAAAAAAAAAAAA\n");
    char buf[_SIZE_];
    int ret = -1;
    int content_length = -1;
    int cgi_input[2];
    int cgi_output[2];
    char method_env[_SIZE_];
    char query_string_env[_SIZE_];
    char content_length_env[_SIZE_];
    if(strcasecmp(method, "GET") == 0)
    {
        //把所有HTTP header读取并丢弃
        clear_header(sock);
    }
    else//post
    {
        //对POST请求的HTTP要找出content-length
        do
        {
            ret = get_line(sock, buf, sizeof(buf));
            if(strncasecmp(buf, "Content-Length: ", 16) == 0)
            {
                content_length = atoi(&buf[16]);
            }
        }while(ret > 0 && (strcmp(buf, "\n") != 0));
        //没有找到content-length
        if(content_length == -1)
        {
            //出错
            echo_errno(sock, 400);
            return;
        }
    }
    //正确，HTTP状态码200，状态OK
    sprintf(buf,"HTTP/1.0 200 OK\r\n\r\n");
    send(sock, buf, strlen(buf), 0);
    //建立管道
    if(pipe(cgi_input) < 0)
    {
        echo_errno(sock, 404);
        return;
    }
    //建立管道
    if(pipe(cgi_output) < 0)
    {
        echo_errno(sock, 404);
        return;
    }
    //创建进程
    pid_t id = fork();
    if(id == 0)//child
    {
        //关闭cgi_input的写端和cgi_output的读端
        close(cgi_input[1]);
        close(cgi_output[0]);

        //把标准输出（stdout）重定向到cgi_input
        dup2(cgi_input[1], 1);
        //把标准输入（stdin）重定向到cgi_output
        dup2(cgi_output[0], 0);

        //设置request_method环境变量
        sprintf(method_env, "REQUEST_METHOD=%s", method);
        //加入环境变量，对本程序有效。相当于把参数通过环境变量传给cgi脚本
        putenv(method_env);
        //GET
        if(strcasecmp(method, "GET") == 0)
        {
            //设置query_string的环境变量
            sprintf(query_string_env, "QUERY_STRING=%s", query_string);
            putenv(query_string_env);
        }
        else//POST
        {
            //设置content_length的环境变量
            sprintf(content_length_env, "CONTENT_LENGTH=%d", content_length);
            putenv(content_length_env);
        }
        //用execl运行cgi程序，相当于用新的程序替换了原来的进程
        execl(path, path, NULL);
        exit(1);
    }
    else//father
    {
        //关闭cgi_input读端和cgi_output写端
        close(cgi_input[0]);
        close(cgi_output[1]);

        char c = '\0';
        int i = 0;
        if(strcasecmp(method, "POST") == 0)
        {
            //接受POST过来的数据
            for(i = 0; i < content_length; i++)
            {
                recv(sock, &c, 1, 0);
                //把POST过来数据写入cgi_input,重定向到stdin
                write(cgi_input[1], &c, 1);
            }
        }
        //把cgi_output输出到客户端，输入端stdout
        while(read(cgi_output[0], &c, 1) > 0)
        {
            send(sock, &c, 1, 0);//发送给客户端
        }
        printf("ssssssssssssssssss\n");
       // close(cgi_input[1]);
       // close(cgi_output[0]);
        //等待子进程
        waitpid(id, NULL, 0);
    }
}
void echo_www(int sock, const char* path, int size)
{
    int fd = open(path, O_RDONLY);
    if(fd < 0)
    {
        print_debug("open index html error");
        echo_errno(sock, 404);
        return;
    }
    print_debug("open index html success");
    char buf[_SIZE_];
    //把默认的"HTTP/1.0 200 OK"给buf
    sprintf(buf, "HTTP/1.0 200 OK\r\n\r\n");
    //把头部信息给客户端
    send(sock, buf, strlen(buf), 0);
    print_debug("send echo head success");
    if(sendfile(sock, fd, NULL, size) < 0)
    {
        print_debug("sendfile error");
        echo_errno(sock, 404);
        close(fd);
        return;
    }
    close(fd);
}
//链接新的线程
void* accept_request(void *arg)
{
    print_debug("get a new connect...\n");
    //防止创建新的线程，主线程发生阻塞
    pthread_detach(pthread_self());//设置为非阻塞，程序运行结束，自动释放资源
    int sock = (int)arg;
    char buf[_SIZE_];//缓冲区
    char method[_SIZE_/10];//方法：POST或者GET
    char url[_SIZE_];//请求的文件的路径如：GET/color.cgi?color=red中的/color.cgi
    char path[_SIZE_];//文件的相对路径
    int cgi = 0;//标识符
    int ret = -1;
    char* query_string = NULL;//客户端发送GET方法的参数信息。如：GET/color.cgi?color=red中的color=red
    memset(path, '\0', sizeof(path));
    memset(url, '\0', sizeof(url));
    memset(method, '\0', sizeof(method));
    memset(buf, '\0', sizeof(buf));
    //得到请求的第一行
    ret = get_line(sock, buf, sizeof(buf)/sizeof(buf[0]));
    if(ret < 0)
    {
        echo_errno(sock, 400);
        return (void*)1;
    }
    //GET mothod
    int i = 0;//method index
    int j = 0;//buf index
    //把客户端的请求方法存到method数组中
    //获取请求头方法的字符串：POST？GET
    while((i < sizeof(method) - 1) && (j < sizeof(buf)) && (!isspace(buf[j])))
    {
        method[i] = buf[j];
        i++;
        j++;
    }
    method[i] = '\0';
    //遇到空格，直接向下走
    while(isspace(buf[j]) && (j < sizeof(buf)))
    {
        j++;
    }
    i = 0;//url index
    //读取url地址，并存入数组url中
    while((i < sizeof(url) - 1) && (j < sizeof(buf) - 1) && (!isspace(buf[j])))
    {
        url[i] = buf[j];
        i++;
        j++;
    }
    url[i] = '\0';
    print_debug(method);
    print_debug(url);
    //如果既不是GTE方法也不是POST方法，出错
    //strcasecmp是忽略大小写的字符串比较
    if(strcasecmp(method, "GET") != 0 && strcasecmp(method, "POST") != 0)
    {
        echo_errno(sock, 501);
        return (void*)2;
    }
    //如果是POST方法，cgi设置为1，表示能运行CGI程序
    if(strcasecmp(method, "POST") == 0)
    {
        printf("CCCCCCCCCCCCCCCCCC\n");
        cgi = 1;
    }
    //处理GET方法
    if(strcasecmp(method, "GET") == 0)
    {
        query_string = url;
        //GET方法的特点，‘？’后面为参数
        while(*query_string != '\0' && *query_string != '?')
        {
            query_string++;
        }
        if(*query_string = '?')
        {
            cgi = 1;//开启cgi
            *query_string = '\0';
            query_string++;
        }
    }
    //格式化url到path数组中，html文件都在htdoc中
    sprintf(path, "htdoc%s", url);
    //默认情况下为"index.html"
    if(path[strlen(path) - 1] == '/')
    {
        strcat(path, "index.html");
    }
    print_debug(path);
    printf("zzzzzzzzzzzzzzzzzz\n");
    struct stat st;//文件属性的结构体
    //根据路径找到文件
    if(stat(path, &st) < 0)
    {
        print_debug("missing cgi");
        clear_header(sock);
        echo_errno(sock, 404);
        return (void*)3;
    }
    else
    {
        //如果是目录，默认使用"htdoc/index.html"文件
        if(S_ISDIR(st.st_mode))
        {
        printf("vvvvvvvvvvvvvvvvvvvvvv\n");
            strcpy(path, "htdoc/index.html");
        }
        //如果文件为可执行文件，开启cgi
        else if((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH))
        {
            cgi = 1;
        }
        else{}
        if(cgi)
        {
            exec_cgi(sock, path, method, query_string);//调用函数执行cgi脚本
        }
        else
        {
            clear_header(sock);//清除所有header信息
            print_debug("begin enter our echo_html");
            echo_www(sock, path, st.st_size);//输出服务器文件到浏览器上
        }
    }
    close(sock);//断开链接
    return (void*)0;
}
int statup(char* _ip, int _port)
{
    //建立套接字
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        print_log(__FUNCTION__, __LINE__, errno, strerror(errno));
        exit(2);
    }
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in local;//本地地址结构体
    local. sin_family = AF_INET;//设置ip协议
    local.sin_port = htons(_port);//设置端口号
    local.sin_addr.s_addr = inet_addr(_ip);//设置ip地址
    //绑定
    if(bind(sock, (struct sockaddr*)&local, sizeof(local)) < 0)
    {
        print_log(__FUNCTION__, __LINE__, errno, strerror(errno));
        exit(3);
    }
    //监听
    if(listen(sock, 5) < 0)
    {
        print_log(__FUNCTION__, __LINE__, errno, strerror(errno));
        exit(4);
    }
    return sock;
}
int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        usage(argv[0]);
        exit(1);
    }
    //建立链接
    int listen_sock = statup(argv[1], atoi(argv[2]));
    struct sockaddr_in peer;
    socklen_t len = sizeof(peer);
    while(1)
    {
        //建立一个新的套接字链接
        int new_sock = accept(listen_sock, (struct sockaddr*)&peer, &len);
        if(new_sock > 0)
        {
            printf("debug:client socket:%s:%d\n",inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
            pthread_t id;
            //创建线程
            pthread_create(&id, NULL, accept_request, (void*)new_sock);
            pthread_detach(id);//分离线程
        }
    }
    return 0;
}
