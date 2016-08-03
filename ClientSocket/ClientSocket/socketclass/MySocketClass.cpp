#include "MySocketClass.h"

void myprintf(const char *format, ...) {}

CMySocketClass::CMySocketClass()
{
	/* Init Windows Socket */ 
	WSADATA  Ws;
	if ( WSAStartup(MAKEWORD(2,2), &Ws) != 0 )  
	{  
		PRINTF("Init Windows Socket Failed::%d\n", GetLastError());    
	} 
}

CMySocketClass::~CMySocketClass()
{
	/* put "q" to exit client */
	int Ret = send(m_Socket, "q", (int)strlen("q"), 0);  
	if ( Ret == SOCKET_ERROR )  
	{  
		PRINTF("Send Info Error::%d\n", GetLastError());  
	}
	PRINTF("Send Info Success!\n");  

	/* close socket */  
	closesocket(m_Socket);  
	WSACleanup();  
}

bool CMySocketClass::Init()
{
	/* Create Socket */  
	m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  
	if ( m_Socket == INVALID_SOCKET )  
	{  
		PRINTF("Create Socket Failed::%d\n", GetLastError());  
		return false;  
	} 
	PRINTF("Create Socket Success!\n");
	return true;
}

bool CMySocketClass::Connect(const char *strIPAddr, unsigned short usPort)
{
	struct sockaddr_in ClientAddr;  
	int Ret = 0;   

	ClientAddr.sin_family = AF_INET;  
	ClientAddr.sin_addr.s_addr = inet_addr(strIPAddr);  
	ClientAddr.sin_port = htons(usPort);  
	memset(ClientAddr.sin_zero, 0x00, 8);  

	/* connect socket */  
	Ret = connect(m_Socket,(struct sockaddr*)&ClientAddr, sizeof(ClientAddr));  
	if ( Ret == SOCKET_ERROR )  
	{  
		PRINTF("Connect Error::%d\n", GetLastError());  
		return false;  
	}  
	else  
	{  
		PRINTF("Connect succedded!\n");
		return true;
	}  
}

bool CMySocketClass::Send(const char *strSend)
{
	int Ret = send(m_Socket, strSend, (int)strlen(strSend), 0);  
	if ( Ret == SOCKET_ERROR )  
	{  
		PRINTF("Send Info Error::%d\n", GetLastError());  
		return false;
	}
	PRINTF("Send Info Success!\n");  
	return true;
}

bool CMySocketClass::Recv(char *RecvBuffer)
{
	char RecvBufferTemp[BUFFER_SIZE] = {0};

	int nLength = recv(m_Socket, RecvBufferTemp, (int)sizeof(RecvBufferTemp), 0);
	if(nLength < 0)
	{
		PRINTF("Recv Info Error::%d\n", GetLastError());  
		return false;
	}
	PRINTF("server say : \n%s\n", RecvBufferTemp);
	strcpy_s(RecvBuffer, BUFFER_SIZE, RecvBufferTemp);

	return true;
}