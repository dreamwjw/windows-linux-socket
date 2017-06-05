#pragma once

#include "protocol.h"

#include <list>
using namespace std;

class CNetWork
{
public:
	CNetWork(void);
	~CNetWork(void);

private:
	typedef struct tagStruSendBuff
	{
		int nSocket;
		char* szData;
		int nLen;
		int nFlag;
	}StruSendBuff;
	list<StruSendBuff> m_vecSendBuff;
	//pthread_mutex_t m_mutexSendBuff;

private:
	char tolower(char c);
	int SaveSendBuff(int client_socket, char* buf, int len, int flag);

public:
	int setSocketNonBlock(int socket, int enable);
	bool SearchPacketHeader(const int fd, char *buf,const int &iBufLen);
	int NonblockingRead(int ifd, unsigned int uiTimeOut);
	int NonblockingWrite(int ifd, unsigned int uiTimeOut);
	int Net_Receive(int client_socket, char* buf, int len, int flag);
	int Net_Send(int client_socket, char* buf, int len, int flag);
	int DoSend();
	int CreateHeader(Header* pHead, WU_uint16_t usCode, WU_uint32_t uiDataLen, WU_uint64_t ullDstId);
	int Keep_Alive_Rsp_Function(int client_socket, unsigned long long ullClientID, unsigned short usAliveSeq);
	int Keep_Alive_Req_Function(int client_socket, Header* pHeader, const char* RecvBuffer);
	int Login_Rsp_Function(int client_socket, unsigned long long ullClientID, unsigned char ucResult);
	int Login_Req_Function(int client_socket, Header* pHeader, const char* RecvBuffer,unsigned long long ullMacID);
	bool UserIsOnline(int nSocketID);
	bool UserIsOnline(unsigned long long ullClientID);
	int GetUserList_Rsp_Function(int client_socket, unsigned long long ullClientID);
	int GetUserList_Req_Function(int client_socket);
	int TalkWithUser_Rsp_Function(int ToUserSocketID, unsigned long long ullToUserID, const char* szContent,
		const char* szFromUser, const char* szToUser);
	int TalkWithUser_Req_Function(int client_socket, const char* RecvBuffer);

};