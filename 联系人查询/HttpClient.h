#ifndef _HTTP_CLIENT_H_
#define _HTTP_CLIENT_H_
 
#define HTTP_DEFAULT_REQUEST_TIMEOUT		(60*1000)
#define HTTP_DEFAULT_PORT					80
#define HTTP_SEND_BUF_LEN					2048
#define HTTP_RECV_BUF_LEN					2048
 
class HttpClient
{
public:
	HttpClient();
	~HttpClient();
 
public:
	/*********************************************************************
	功能：以阻塞方式发送一个HTTP请求，并返回请求结果
	返回值：返回发送成功或者失败
	参数：url	请求的HTTP url，不能为空。
		  post_data	 POST数据，可以为空，如果为空，则使用GET方式发送。
		  time_out   发送超时时间，如果为0，则使用默认值
		  ret_buf    请求返回结果缓冲区
		  ret_len    请求返回结果缓冲区长度
	***********************************************************************/
	bool Send(const char* url, const char* post_data, const int time_out, char* ret_buf, int* ret_len);
 
private:
	char		m_strIP[MAX_PATH];				/*HTTP服务器地址*/
	int			m_iPort;						/*HTTP服务器端口*/
	char		m_strAction[MAX_PATH];			/*HTTP请求的动作*/
	int			m_iTimeOut;						/*HTTP请求超时时间*/
 
	SOCKET		m_sock;							/*TCP 套接字*/
 
 
	/*********************************************************************
	功能：实现TCP连接HTTP服务器
	返回值：返回发送成功或者失败
	参数：空
	***********************************************************************/
	bool TCP_Connect();
 
	/*********************************************************************
	功能：实现TCP连接关闭
	返回值：空
	参数：空
	***********************************************************************/
	void TCP_Close();
 
	/*********************************************************************
	功能：设置阻塞TCP连接的超时时间
	返回值：返回发送成功或者失败
	参数：空
	***********************************************************************/
	bool TCP_SetTimeout();
 
	/*********************************************************************
	功能：解析URL中的IP地址，端口，文件名等。
	返回值：返回发送成功或者失败
	参数：url	请求的HTTP url，不能为空。
	***********************************************************************/
	bool ParserUrl(const char* url);
 
	/*********************************************************************
	功能：实现HTTP POST方法
	返回值：返回发送成功或者失败
	参数：post_data	 POST数据，不能为空
		  ret_buf	 HTTP返回数据
		  ret_len	 HTTP返回数据长度
	***********************************************************************/
	bool Post(const char* post_data, char* ret_buf, int* ret_len);
 
	/*********************************************************************
	功能：实现HTTP GET方法
	返回值：返回发送成功或者失败
	参数：ret_buf	 HTTP返回数据
		  ret_len	 HTTP返回数据长度
	***********************************************************************/
	bool Get(char* ret_buf, int* ret_len);
 
	/*********************************************************************
	功能：生成POST方法的HTTP包
	返回值：空
	参数：post_data	 POST数据
		  buf		 HTTP数据缓冲区
		  buf_len	 传入缓冲区长度，传出HTTP数据长度
	***********************************************************************/
	void MakePostBuf(const char* post_data, char* buf, int* buf_len);
 
	/*********************************************************************
	功能：初始化Win32网络库
	返回值：空
	参数：空
	***********************************************************************/
	inline void InitWin32NetLib()
	{
		WSADATA wsa_data;
		WSAStartup(MAKEWORD(2,0), &wsa_data);
	}
 
	/*********************************************************************
	功能：注销Win32网络库
	返回值：空
	参数：空
	***********************************************************************/
	inline void UnInitWin32NetLib()
	{
		WSACleanup();
	}
 
private:
	static char* m_post_header;
	static char* m_get_header;
 
public:
	static HttpClient* GetInstance();
private:
	static HttpClient m_oInstance;
};
 
 
#endif