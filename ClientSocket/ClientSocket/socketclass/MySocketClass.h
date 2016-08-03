#ifndef MYSOCKETCLASS_H_
#define MYSOCKETCLASS_H_

#include <stdio.h>  
#include <Windows.h>  

#pragma comment(lib,"ws2_32.lib") 

//MFC����������printf���������������Ҫ��myprintf���滻printf
void myprintf(const char *format, ...);

#ifdef _CONSOLE
#define PRINTF printf
#else
#define PRINTF myprintf
#endif
class CMySocketClass
{
private:
	SOCKET m_Socket;
public:
	enum {BUFFER_SIZE = 1024};

	CMySocketClass();
	virtual ~CMySocketClass();
	bool Init();
	bool Connect(const char *strIPAddr, unsigned short nPort);
	bool Send(const char *strSend);
	bool Recv(char *RecvBuffer);
};

#endif