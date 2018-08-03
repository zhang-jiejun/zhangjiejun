
#include "stdafx.h"
#include "HttpClient.h"
#include <Log/MyLogEx.h>
 
 
HttpClient HttpClient::m_oInstance;
 
HttpClient* HttpClient::GetInstance()
{
	return &m_oInstance;
}
 
char* HttpClient::m_post_header = "POST %s HTTP/1.1\r\n"
    "Accept: image/gif, image/jpeg, */*\r\nAccept-Language: zh-cn\r\n"
    "Accept-Encoding: gzip, deflate\r\nHost: %s:%d\r\n"
	"Content-Type: application/json\r\nContent-Length: %d\r\n"
    "User-Agent: HLS Slice Service\r\nConnection: Keep-Alive\r\n\r\n%s";
 
char* HttpClient::m_get_header = "GET %s HTTP/1.1\r\n"
    "Accept: image/gif, image/jpeg, */*\r\nAccept-Language: zh-cn\r\n"
    "Accept-Encoding: gzip, deflate\r\nHost: %s:%d\r\n"
    "User-Agent: HLS Slice Service\r\nConnection: Keep-Alive\r\n\r\n";
 
HttpClient::HttpClient()
	: m_iPort(HTTP_DEFAULT_PORT),
	  m_iTimeOut(HTTP_DEFAULT_REQUEST_TIMEOUT),
	  m_sock(INVALID_SOCKET)
{
	memset(m_strIP, 0, sizeof(m_strIP));
	memset(m_strAction, 0, sizeof(m_strAction));
 
	InitWin32NetLib();
}
 
HttpClient::~HttpClient()
{
	if(m_sock != INVALID_SOCKET)
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}
 
	UnInitWin32NetLib();
}
 
bool HttpClient::Send(const char* url, const char* post_data, const int time_out, char* ret_buf, int* ret_len)
{
	if(url == NULL || ret_buf == NULL || *ret_len == 0)
	{
		return false;
	}
 
	if(!ParserUrl(url))
	{
		LOG_PRINTEX(0, MyLogEx::LOG_LEVEL_DEBUG_4, "Parser HTTP URL failed!");
		return false;
	}
 
	//LOG_PRINTEX(0, MyLogEx::LOG_LEVEL_DEBUG_4, "HTTP IP=%s, port=%d, filename=%s",
	//			m_strIP, m_iPort, m_strAction);
 
	m_iTimeOut = (time_out == 0 ? HTTP_DEFAULT_REQUEST_TIMEOUT : time_out);
 
	if(post_data)
	{
		return Post(post_data, ret_buf, ret_len);
	}
	
	return Get(ret_buf, ret_len);
}
 
bool HttpClient::ParserUrl(const char* url)
{
	char szBuf[1024] = {0};
	strncpy_s(szBuf, sizeof(szBuf)-1, url, sizeof(szBuf)-1);
	int length = 0;
	char port_buf[20];
	char *buf_end = (char *)(szBuf + strlen(szBuf));
	char *begin, *host_end, *colon, *question_mark;
 
	/* 查找主机的开始位置 */
	begin = strstr(szBuf, "//");
	begin = (begin ? begin + 2 : szBuf);
 
	colon = strchr(begin, ':');
	host_end = strchr(begin, '/');
 
	if(host_end == NULL)
	{
		host_end = buf_end;
	}
	else
	{   /* 得到文件名 */
		question_mark = strchr(host_end, '?');
		if(question_mark != NULL)
		{
			strncpy_s(m_strAction, MAX_PATH-1, host_end, question_mark-host_end);
		}
		else
		{
			strncpy_s(m_strAction, MAX_PATH-1, host_end, strlen(host_end));
		}
	}
 
	if(colon) /* 得到端口号 */
	{
		colon++;
 
		length = host_end - colon;
		memcpy(port_buf, colon, length);
		port_buf[length] = 0;
		m_iPort = atoi(port_buf);
 
		host_end = colon - 1;
	}
	else
	{
		m_iPort = HTTP_DEFAULT_PORT;
	}
 
	/* 得到主机信息 */
	length = host_end - begin;
	length = (length > MAX_PATH  ? MAX_PATH : length);
	memcpy(m_strIP, begin, length);
	m_strIP[length] = 0;
 
	return true;
}
 
void HttpClient::MakePostBuf(const char* post_data, char* buf, int* buf_len)
{
	sprintf_s(buf, *buf_len, m_post_header, m_strAction, m_strIP, m_iPort, strlen(post_data), post_data);
	*buf_len = strlen(buf);
 
	//LOG_PRINTEX(0, MyLogEx::LOG_LEVEL_DEBUG_4, "POST Data=%s", buf);
}
 
bool HttpClient::TCP_SetTimeout()
{
	int TimeOut=m_iTimeOut;
 
	if(::setsockopt(m_sock, SOL_SOCKET, SO_SNDTIMEO,(char *)&TimeOut, sizeof(TimeOut))==SOCKET_ERROR)
	{
		LOG_PRINTEX(0, MyLogEx::LOG_LEVEL_DEBUG_4, "TCP设置发送超时失败！");
		return false;
	}
 
	if(::setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&TimeOut, sizeof(TimeOut))==SOCKET_ERROR)
	{
		LOG_PRINTEX(0, MyLogEx::LOG_LEVEL_DEBUG_4, "TCP设置接收超时失败！");
		return false;
	}
 
	return true;
}
 
bool HttpClient::TCP_Connect()
{
	int result = 0;
	struct sockaddr_in serv_addr;
 
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons((u_short)m_iPort);
	serv_addr.sin_addr.s_addr = inet_addr(m_strIP);
 
	m_sock = socket(AF_INET, SOCK_STREAM, 0); 
	if(m_sock == INVALID_SOCKET)
	{
		LOG_PRINTEX(0, MyLogEx::LOG_LEVEL_DEBUG_4, "socket() 调用失败！错误码=%d", WSAGetLastError());
		return false;
	}
 
	if(!TCP_SetTimeout())
	{
		return false;
	}
 
	result = connect(m_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if(result == SOCKET_ERROR)
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		//LOG_PRINTEX(0, MyLogEx::LOG_LEVEL_DEBUG_4, "连接失败！错误码=%d", WSAGetLastError());
		return false; 
	}
 
	return true;
}
 
void HttpClient::TCP_Close()
{
	closesocket(m_sock);
	m_sock = INVALID_SOCKET;
}
 
bool HttpClient::Post(const char* post_data, char* ret_buf, int* ret_len)
{
	int iRet;
	char strSendBuf[HTTP_SEND_BUF_LEN] = {0};
	int iSendLen = HTTP_SEND_BUF_LEN;
	MakePostBuf(post_data, strSendBuf, &iSendLen);
 
	if(!TCP_Connect())
	{
		return false;
	}
 
	iRet = send(m_sock, strSendBuf, iSendLen, 0);
	if (iRet == SOCKET_ERROR)
	{
		LOG_PRINTEX(0, MyLogEx::LOG_LEVEL_DEBUG_4, "发送数据失败！错误码=%d", WSAGetLastError());
		TCP_Close();
		return false; 
	}
 
	int recv_len = *ret_len;
	iRet = recv(m_sock, ret_buf, recv_len, 0);
	if(iRet > 0)
	{
		*ret_len = iRet;
	}
	else if(iRet == 0)
	{
		//LOG_PRINTEX(0, MyLogEx::LOG_LEVEL_DEBUG_4, "Server close connection");
	}
	else
	{
		//LOG_PRINTEX(0, MyLogEx::LOG_LEVEL_DEBUG_4, "error == %d", WSAGetLastError());
		*ret_len = 0;
	}
 
	TCP_Close();
 
	return true;
}
 
bool HttpClient::Get(char* ret_buf, int* ret_len)
{
	return false;
}