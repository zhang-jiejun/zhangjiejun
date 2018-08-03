 
#ifndef __http__
#define __http__
 
/*TODO : 连接失败后的特殊处理*/
#include <cstring>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/wait.h>
#include <error.h>
#include <time.h>
#include <iostream>
 
 
using std::string;
using std::endl;
using std::cout;
using std::cin;
 
#define HTTP_KEEP_ALIVE "Keep-Alive"
#define HTTP_GET        "GET"
 
class http_request_header
{
	protected:
		string method; /*GET OPTION HEAD POST PUT DELETE TRACE CONNECT*/  //就是方法，但是我现在只实现了GET方法
		string file;	//目前只实现了GET方法,这里也就是  www.baidu.com/?abc 大概意思就是file = "?abc"
		string host;   /*这里是纯IP*/
		//CRLF
 
	private:
		/*这里在初始化的时候定义了一些常量,以后的程序可能会改*/
		string version;   /*默认就是 HTTP1.1 */
		string connection; /*  默认是Keep-Alive*/
		string space;  //就是空格
		string CRLF;   //就是回车，换行符。
 
	public:
		void set_method(const string &method);
		void set_file(const string &file);
		void set_host(const string &host);
		void set_connection(const string &connection);
 
		http_request_header(); //初始化函数，也就是初始化private里东西
		string make();		//根据当前的数据，生成一个header
 
};
 
class httpclient
{
	private:
		void http_client_create(const string &host, const int port);	//创建一个客户机，参数分别形如 "192.168.1.1"和80这样的两个参数。第一个是IP，第二个是端口号
 
		void http_close();						//断开和服务器链接
 
		int http_recv(string &ret);					//接受服务器返回数据（比如get请求有返回数据）
										//函数返回值为最后一次调用recv函数的返回值。
										//如果3秒没有接受到消息，则会自动退出
 
		int http_client_send(string msg);				//根据当前的header设置，向服务器发送一个msg信息。（会自动根据设置，生成头）
										//返回值和send函数意义相同。
	public:
		int http_get(const string host_addr, const string file, const int port, string &ret);	//向host_addr发送一个http请求，其中的参数是file，端口号port。  返回的字符串为ret
													//这个函数的返回值，-1为失败，并会在标准输出上输出错误信息。否则返回1。
 
	private:
		int socket_fd;							//socket常用
		char rec_buff[1024];						//接受数据的buff
		string msg;							//临时变量
		http_request_header hrh;					//http请求的头部。
 
}http;
 
int main()
{
	string str;
//	http.http_get("14.215.177.38", "", 80, str); //百度首页
	http.http_get("http://14.215.177.37","s?ie=utf-8&f=8&rsv_bp=1&rsv_idx=2&tn=baiduhome_pg&wd=重庆邮电大学移通学院&oq=%25E7%25BD%2591%25E9%25A1%25B5get%25E8%25AF%25B7%25E6%25B1%2582&rsv_pq=a74bcb630000e36c&rsv_t=83fbonPcSyxgw2tYRSXZS6dODMw0Zwk9jrvpFdc9JtHJbUHgnkMjFL4B04rEF%2F10R42C&rqlang=cn&rsv_enter=1&rsv_sug3=3&rsv_sug1=2&rsv_sug7=100&sug=%25E9%2587%258D%25E5%25BA%2586%25E9%2582%25AE%25E7%2594%25B5%25E5%25A4%25A7%25E5%25AD%25A6%25E7%25A7%25BB%25E9%2580%259A%25E5%25AD%25A6%25E9%2599%25A2&rsv_n=1&bs=网页get请求",80,str);		//百度的一个网页，我们通过get请求
	cout << str<<endl;
	FILE *fout = fopen("index.html", "w"); //输出网页，自己可以打开浏览
	fprintf(fout, "%s", str.c_str());
	return 0;
}
 
/*-------------------http_request_header begin-----------------------------*/
 
void http_request_header::set_method(const string &method)
{
	this -> method = method;
}
 
void http_request_header::set_file(const string &file)
{
	this -> file = file;
}
 
void http_request_header::set_host(const string &host)
{
	this -> host = host;
}
 
void http_request_header::set_connection(const string &connection)
{
	this -> connection = connection;
}
 
http_request_header::http_request_header()
{
	version = string("HTTP/1.1");
	CRLF = string("\r\n");
	space = string(" ");
}
string http_request_header::make()
{
	string ret = method;
	if(method.compare(string("GET"))==0)
	{
		ret += string(" /") + file + space + version + CRLF\
		       +  string("HOST:") + space + host + CRLF\
		       +  string("Connection:")  + space + connection + CRLF\
		       +  CRLF;
	}
	else
	{
		printf("error : is not the GET request!?\n");
		exit(0);
	}
	return ret;
}
 
/*-------------------http_request_header end-----------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------httpclient start--------------------------------*/
 
void httpclient::http_client_create(const string &host, const int port)
{
	//struct hostent *he = NULL;
	struct sockaddr_in server_addr;
 
 
	cout << host <<" " << port << endl;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(host.c_str());//*((struct in_addr *)he -> h_addr);
 
 
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
	{
 
		printf("error: creat socket error\n");
		exit(0);
		//return -1;
	}
 
	if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
	{
		printf("error: connect socket error\n");
		exit(0);
		//return -1;
	}
 
}
 
void httpclient::http_close()
{
	close(socket_fd);
}
 
int httpclient::http_recv(string &ret)
{
	int recvnum = 0;
	ret = "";
	memset(rec_buff, 0, sizeof(rec_buff));
	while (	recvnum = recv(socket_fd, rec_buff, sizeof(rec_buff), 0) > 0)
	{
		ret += string(rec_buff);
		memset(rec_buff, 0, sizeof(rec_buff));
	}
	return recvnum;
}
 
int httpclient::http_client_send(string msg)
{
	msg = hrh.make() + msg;
	int flag = send(socket_fd, msg.c_str(), msg.size() ,0);
	if (flag < 0)
	{
		printf("send socket error\n");
	}
	return flag;
}
 
int httpclient::http_get(const string host_addr, const string file, const int port, string &ret)
{
	string host;
 
	if(host_addr.size() == 0)
	{
		printf("error: url's length error \n");
		return -1;
	}
 
	if (host_addr.find("http://", 0) == -1)
	{
		printf("warnning ! the host_addr is not the http ?\n");
		host = host_addr;
	}
	else
	{
		host = host_addr.substr(7, host_addr.size() - 7);
	}
 
	http_client_create(host ,port);
 
	if(socket_fd < 0)
	{
		printf("error : http_lient_create failed\n");
		return -1;
	}
 
	//make the http header
	hrh.set_method(HTTP_GET);
	hrh.set_file(file);
	hrh.set_host(host);
	hrh.set_connection(HTTP_KEEP_ALIVE);
	msg="";
 
	if( http_client_send(msg) < 0)
	{
		printf("error : http_client_send failed..\n");
		return -1;
	}
 
	struct timeval timeout = {3, 0};
	int flag = 0;
	setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
	flag= http_recv(ret);
	http_close();
	return 1;
}
 
/*-----------------------httpclient   end--------------------------------*/
 
#endif
